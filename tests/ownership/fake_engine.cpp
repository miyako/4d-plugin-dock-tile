// Models the engine side of the 4D Plugin API ownership contract:
// - EX_GET_OBJ_VALUE returns an owned copy (new Unistring / retained ref)
// - EX_SET_OBJ_VALUE deep-copies; the caller keeps ownership of its variable
// - EX_CLEAR_VARIABLE releases what a variable owns
// Every engine allocation is a real heap block, so ASan sees double frees and
// use-after-free; leaks are counted explicitly (LSan is not available on macOS).
#include "fake_engine.h"
#include "EntryPoints.h"
#include "PrivateTypes.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>

namespace fake4d {

struct Value {
    PA_VariableKind kind = eVK_Undefined;
    double real = 0;
    bool boolean = false;
    std::u16string text;
    Obj *ref = nullptr;
};

struct Obj {
    int rc = 1;
    bool collection = false;
    std::map<std::u16string, Value> props;
};

static std::set<PA_Unichar *> gStrings;
static std::set<Obj *> gObjects;
static std::set<void *> gHandles;
static std::vector<std::string> gViolations;
static int gFailGets = 0;
static PA_Picture const kPoison = (PA_Picture)(uintptr_t)0xBADBAD0;

static void violation(const std::string &s) { gViolations.push_back(s); }

static std::u16string u16(const char *s) { return std::u16string(s, s + strlen(s)); }

// EngineBlock and PA_Variable are packed: always take Unistrings by value.
static std::u16string from_unistring(PA_Unistring u) {
    if (!u.fString) return std::u16string();
    return std::u16string((const char16_t *)u.fString, (size_t)u.fLength);
}

static PA_Unistring make_unistring(const std::u16string &s) {
    PA_Unichar *p = new PA_Unichar[s.size() + 1];
    memcpy(p, s.data(), s.size() * sizeof(PA_Unichar));
    p[s.size()] = 0;
    gStrings.insert(p);
    PA_Unistring u = {(PA_long32)s.size(), p, 0, 0};
    return u;
}

static void dispose_unistring(PA_Unistring u) {
    PA_Unichar *p = u.fString;
    if (!p) return;
    auto it = gStrings.find(p);
    if (it == gStrings.end()) { violation("dispose_unknown_string"); return; }
    gStrings.erase(it);
    delete[] p;
}

static void retain(Obj *o) {
    if (!gObjects.count(o)) { violation("retain_dead_object"); return; }
    o->rc++;
}

void release(Obj *o) {
    if (!o) return;
    if (!gObjects.count(o)) { violation("release_dead_object"); return; }
    if (--o->rc == 0) {
        for (auto &kv : o->props) if (kv.second.ref) release(kv.second.ref);
        gObjects.erase(o);
        delete o;
    }
}

static void clear_variable(PA_Variable *v) {
    switch (v->fType) {
        case eVK_Unistring: dispose_unistring(v->uValue.fString); break;
        case eVK_Object:
        case eVK_Collection:
            if (v->uValue.fObject) release((Obj *)v->uValue.fObject);
            break;
        case eVK_Picture:
            if (v->uValue.fPicture == kPoison) violation("cleared_value_after_get_error");
            else if (v->uValue.fPicture) violation("unexpected_picture_clear");
            break;
        default: break;
    }
    v->fType = eVK_Undefined;
    memset(&v->uValue, 0, sizeof(v->uValue));
}

static void get_value(EngineBlock *eb) {
    PA_Variable *out = (PA_Variable *)eb->fPtr2;
    if (gFailGets > 0) {
        gFailGets--;
        // The SDK's local `PA_Variable value;` is uninitialised on failure;
        // model it as a value the plugin must neither own nor clear.
        out->fType = eVK_Picture;
        out->fFiller = 0;
        out->uValue.fPicture = kPoison;
        eb->fError = -1;
        return;
    }
    Obj *o = (Obj *)eb->fPtr1;
    if (!gObjects.count(o)) { violation("get_from_dead_object"); eb->fError = -1; return; }
    out->fFiller = 0;
    auto it = o->props.find(from_unistring(eb->fUniString1));
    if (it == o->props.end()) { out->fType = eVK_Undefined; return; }
    const Value &val = it->second;
    out->fType = (char)val.kind;
    switch (val.kind) {
        case eVK_Real: out->uValue.fReal = val.real; break;
        case eVK_Boolean: out->uValue.fBoolean = val.boolean; break;
        case eVK_Unistring: out->uValue.fString = make_unistring(val.text); break;
        case eVK_Object:
        case eVK_Collection: retain(val.ref); out->uValue.fObject = val.ref; break;
        default: out->fType = eVK_Undefined; break;
    }
}

static void set_value(EngineBlock *eb) {
    Obj *o = (Obj *)eb->fPtr1;
    if (!gObjects.count(o)) { violation("set_on_dead_object"); return; }
    const PA_Variable *in = (const PA_Variable *)eb->fPtr2;
    Value val;
    int kind = (unsigned char)in->fType; // eVK_Null (255) does not fit a signed char
    if (kind == eVK_Null) kind = eVK_Undefined;
    val.kind = (PA_VariableKind)kind;
    switch (kind) {
        case eVK_Real: val.real = in->uValue.fReal; break;
        case eVK_Longint: val.kind = eVK_Real; val.real = in->uValue.fLongint; break;
        case eVK_Boolean: val.boolean = in->uValue.fBoolean; break;
        case eVK_Unistring:
            {
                PA_Unichar *p = in->uValue.fString.fString;
                if (p && !gStrings.count(p)) violation("set_unknown_string");
            }
            val.text = from_unistring(in->uValue.fString);
            break;
        case eVK_Object:
        case eVK_Collection:
            val.ref = (Obj *)in->uValue.fObject;
            if (val.ref) retain(val.ref);
            break;
        case eVK_Undefined: break;
        default: violation("set_unsupported_kind"); return;
    }
    Value &slot = o->props[from_unistring(eb->fUniString1)];
    if (slot.ref) release(slot.ref);
    slot = val;
}

static void execute_command(EngineBlock *eb) {
    PA_Variable **params = (PA_Variable **)eb->fPtr1;
    PA_Variable *result = (PA_Variable *)eb->fPtr2;
    if (eb->fParam1 == 1231 && eb->fParam2 == 2) { // OB Is defined
        Obj *o = (Obj *)params[0]->uValue.fObject;
        bool defined = gObjects.count(o) &&
            o->props.count(from_unistring(params[1]->uValue.fString));
        result->fType = eVK_Boolean;
        result->uValue.fBoolean = defined;
        return;
    }
    violation("unexpected_command_" + std::to_string((long)eb->fParam1));
}

struct Handle { size_t size; char *data; };

static void handle_manager(EngineBlock *eb) {
    Handle *h = (Handle *)eb->fHandle;
    switch (eb->fParam1) {
        case 1: {
            h = new Handle{(size_t)eb->fParam2, (char *)calloc(1, (size_t)eb->fParam2 + 1)};
            gHandles.insert(h);
            eb->fHandle = (PA_Handle)h;
            break;
        }
        case 2:
            if (!gHandles.erase(h)) { violation("dispose_unknown_handle"); break; }
            free(h->data); delete h;
            break;
        case 3:
            h->data = (char *)realloc(h->data, (size_t)eb->fParam2 + 1);
            h->size = (size_t)eb->fParam2;
            eb->fParam1 = 1;
            break;
        case 4: eb->fParam2 = (sLONG_PTR)h->size; break;
        case 5: eb->fParam3 = (sLONG_PTR)h->data; break;
        case 6: case 7: case 8: break;
        default: violation("unexpected_handle_op");
    }
}

static void call4d(short selector, EngineBlock *eb) {
    switch (selector) {
        case EX_CREATE_UNISTRING: {
            const PA_Unichar *src = (const PA_Unichar *)eb->fHandle;
            size_t n = 0;
            while (src && src[n]) n++;
            eb->fUniString1 = make_unistring(std::u16string((const char16_t *)src, n));
            break;
        }
        case EX_VARIABLE_TO_STRING: { // borrowed view of the variable's own text
            const PA_Variable *v = (const PA_Variable *)eb->fHandle;
            if (v->fType == eVK_Unistring) eb->fUniString1 = v->uValue.fString;
            else eb->fError = -1;
            break;
        }
        case EX_DISPOSE_UNISTRING: dispose_unistring(eb->fUniString1); break;
        case EX_GET_OBJ_VALUE: get_value(eb); break;
        case EX_SET_OBJ_VALUE: set_value(eb); break;
        case EX_CLEAR_VARIABLE: clear_variable((PA_Variable *)eb->fPtr1); break;
        case EX_EXECUTE_COMMAND_BY_ID: execute_command(eb); break;
        case EX_HANDLE_MANAGER: handle_manager(eb); break;
        default: violation("unexpected_selector_" + std::to_string(selector));
    }
}

void install() { gCall4D = (Call4DProcPtr)call4d; }

void reset() {
    gViolations.clear();
    gFailGets = 0;
}

Obj *new_object(bool collection) {
    Obj *o = new Obj;
    o->collection = collection;
    gObjects.insert(o);
    return o;
}

int refcount(Obj *o) { return gObjects.count(o) ? o->rc : 0; }

void put_real(Obj *o, const char *k, double v) { Value x; x.kind = eVK_Real; x.real = v; o->props[u16(k)] = x; }
void put_bool(Obj *o, const char *k, bool v) { Value x; x.kind = eVK_Boolean; x.boolean = v; o->props[u16(k)] = x; }
void put_string(Obj *o, const char *k, const char *v) { Value x; x.kind = eVK_Unistring; x.text = u16(v); o->props[u16(k)] = x; }
void put_ref(Obj *o, const char *k, Obj *c) {
    Value x; x.kind = c->collection ? eVK_Collection : eVK_Object; x.ref = c; retain(c); o->props[u16(k)] = x;
}

bool get_string(Obj *o, const char *k, std::string *out) {
    auto it = o->props.find(u16(k));
    if (it == o->props.end() || it->second.kind != eVK_Unistring) return false;
    out->assign(it->second.text.begin(), it->second.text.end());
    return true;
}

size_t live_strings() { return gStrings.size(); }
size_t live_handles() { return gHandles.size(); }
size_t live_objects() { return gObjects.size(); }
void fail_next_gets(int n) { gFailGets = n; }
const std::vector<std::string> &violations() { return gViolations; }
}
