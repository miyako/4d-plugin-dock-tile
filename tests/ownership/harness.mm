// Ownership regressions for the JSON object getters used by DOCK SET PROGRESS.
// Runs the real plugin sources and the real SDK against tests/ownership/fake_engine.
// Usage: harness [expected-failure ...]  (names of tests that must fail)
#include "4DPlugin-Dock-Tile.h"
#include "4DPlugin-JSON.h"
#include "fake_engine.h"
#include <cstdio>
#include <functional>
#include <set>
#include <string>

using namespace fake4d;

static int gDispatched = 0;
extern "C" void ownership_harness_dispatch_async(dispatch_queue_t, dispatch_block_t) {
    gDispatched++; // never run: the block would touch NSApp and the Dock
}

static std::vector<std::string> gFailures;
#define CHECK(cond) do { if (!(cond)) gFailures.push_back(#cond); } while (0)

static void progress(double value, PA_ObjectRef options) {
    double v = value;
    PA_ObjectRef o = options;
    void *params[2] = {&v, &o};
    PluginBlock block = {params, nullptr, nullptr};
    int before = gDispatched;
    @autoreleasepool { DOCK_SET_PROGRESS(&block); }
    CHECK(gDispatched == before + 1);
}

static Obj *color(double r, double g, double b) {
    Obj *c = new_object();
    put_real(c, "red", r); put_real(c, "green", g); put_real(c, "blue", b); put_real(c, "alpha", 0.5);
    return c;
}

struct Test { const char *name; std::function<void()> body; };

static const Test kTests[] = {
    // --- reachable: DOCK SET PROGRESS (selector 9) ---
    {"progress_color_ref_released", [] {
        Obj *opts = new_object(), *c = color(1, 0, 0);
        put_real(opts, "style", 1); put_real(opts, "lineWidth", 3);
        put_real(opts, "radius", 20); put_real(opts, "inset", 2);
        put_ref(opts, "color", c);
        progress(0.5, opts);
        CHECK(refcount(c) == 2); // test + opts
        release(c); release(opts);
        CHECK(refcount(c) == 0);
    }},
    {"progress_nonreal_values_released", [] {
        Obj *opts = new_object(), *radius = new_object();
        put_string(opts, "style", "bar"); put_string(opts, "lineWidth", "wide");
        put_ref(opts, "radius", radius); put_bool(opts, "inset", true);
        progress(0.5, opts);
        CHECK(refcount(radius) == 2);
        release(radius); release(opts);
    }},
    {"progress_color_not_object_released", [] {
        Obj *opts = new_object();
        put_string(opts, "color", "red");
        progress(0.5, opts);
        release(opts);
    }},
    {"progress_color_nonreal_components_released", [] {
        Obj *opts = new_object(), *c = new_object();
        put_string(c, "red", "1"); put_string(c, "green", "0"); put_string(c, "blue", "0");
        put_ref(opts, "color", c);
        progress(0.5, opts);
        CHECK(refcount(c) == 2);
        release(c); release(opts);
    }},
    {"control_progress_get_error_not_cleared", [] {
        Obj *opts = new_object(), *c = color(0, 1, 0);
        put_real(opts, "style", 2); put_real(opts, "lineWidth", 4); put_ref(opts, "color", c);
        fail_next_gets(16);
        progress(0.5, opts);
        CHECK(refcount(c) == 2);
        release(c); release(opts);
    }},
    {"control_progress_no_options", [] { progress(0.5, nullptr); }},
    {"control_progress_empty_options", [] {
        Obj *opts = new_object();
        progress(0.25, opts);
        release(opts);
    }},
    // --- dormant: helpers with no consumer in this plugin ---
    {"dormant_ob_get_a_released", [] {
        Obj *o = new_object();
        put_string(o, "k", "hello");
        CUTF8String s;
        CHECK(ob_get_a(o, L"k", &s));
        CHECK(s == CUTF8String((const uint8_t *)"hello"));
        release(o);
    }},
    {"dormant_ob_get_b_nonbool_released", [] {
        Obj *o = new_object();
        put_string(o, "k", "true"); put_bool(o, "b", true);
        CHECK(!ob_get_b(o, L"k"));
        CHECK(ob_get_b(o, L"b"));
        release(o);
    }},
    {"dormant_ob_get_c_nonmatching_released", [] {
        Obj *o = new_object();
        put_string(o, "k", "x");
        CHECK(ob_get_c(o, L"k") == nullptr);
        release(o);
    }},
    {"control_dormant_ob_get_c_transfers_ref", [] {
        Obj *o = new_object(), *c = new_object(true);
        put_ref(o, "c", c);
        PA_CollectionRef got = ob_get_c(o, L"c");
        CHECK(got == (PA_CollectionRef)c);
        CHECK(refcount(c) == 3); // test + o + caller
        PA_DisposeCollection(got);
        release(c); release(o);
        CHECK(refcount(c) == 0);
    }},
    {"control_ob_get_o_transfers_ref", [] {
        Obj *o = new_object(), *c = new_object();
        put_ref(o, "c", c);
        PA_ObjectRef got = ob_get_o(o, L"c");
        CHECK(got == (PA_ObjectRef)c);
        CHECK(refcount(c) == 3);
        PA_DisposeObject(got);
        release(c); release(o);
    }},
    {"control_getters_error_not_cleared", [] {
        Obj *o = new_object();
        put_string(o, "s", "x"); put_real(o, "n", 1); put_bool(o, "b", true);
        CUTF8String s;
        fail_next_gets(1); ob_get_a(o, L"s", &s);
        fail_next_gets(1); CHECK(ob_get_n(o, L"n") == 0.0);
        fail_next_gets(1); CHECK(!ob_get_b(o, L"b"));
        fail_next_gets(1); CHECK(ob_get_o(o, L"s") == nullptr);
        fail_next_gets(1); CHECK(ob_get_c(o, L"s") == nullptr);
        release(o);
    }},
    {"control_ob_get_n_and_is_defined", [] {
        Obj *o = new_object();
        put_real(o, "n", 2.5);
        CHECK(ob_get_n(o, L"n") == 2.5);
        CHECK(ob_get_n(o, L"missing") == 0.0);
        CHECK(ob_is_defined(o, L"n"));
        CHECK(!ob_is_defined(o, L"missing"));
        release(o);
    }},
    {"control_setters", [] {
        Obj *o = new_object(), *child = new_object();
        ob_set_s(o, "a", "x"); ob_set_s(o, L"b", "y"); ob_set_a(o, L"c", L"z");
        ob_set_n(o, L"n", 1); ob_set_b(o, L"t", true); ob_set_0(o, L"z");
        std::string got;
        CHECK(get_string(o, "a", &got) && got == "x");
        CHECK(get_string(o, "c", &got) && got == "z");
        PA_ObjectRef ref = (PA_ObjectRef)child;
        ob_set_o(o, L"o", ref); // consumes the caller's reference
        CHECK(refcount(child) == 1);
        release(o);
        CHECK(refcount(child) == 0);
    }},
};

int main(int argc, char **argv) {
    install();
    std::set<std::string> expected(argv + 1, argv + argc);
    int unexpected = 0;
    for (const Test &t : kTests) {
        reset();
        gFailures.clear();
        size_t strings = live_strings(), handles = live_handles(), objects = live_objects();
        t.body();
        for (auto &v : violations()) gFailures.push_back("engine: " + v);
        if (live_strings() != strings)
            gFailures.push_back("leaked strings: " + std::to_string(live_strings() - strings));
        if (live_handles() != handles) gFailures.push_back("leaked handles");
        if (live_objects() != objects)
            gFailures.push_back("leaked objects: " + std::to_string(live_objects() - objects));
        bool failed = !gFailures.empty();
        bool want = expected.erase(t.name) > 0;
        const char *verdict = failed ? (want ? "XFAIL" : "FAIL") : (want ? "XPASS" : "PASS");
        printf("%-5s %s\n", verdict, t.name);
        for (auto &f : gFailures) printf("      %s\n", f.c_str());
        if (failed != want) unexpected++;
    }
    for (auto &name : expected) { printf("UNKNOWN expected failure %s\n", name.c_str()); unexpected++; }
    printf("%s\n", unexpected ? "RESULT: UNEXPECTED" : "RESULT: OK");
    return unexpected ? 1 : 0;
}
