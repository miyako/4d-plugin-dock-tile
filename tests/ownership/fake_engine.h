// Synthetic 4D engine behind the real SDK's gCall4D boundary.
#pragma once
#include "4DPluginAPI.h"
#include <string>
#include <vector>

namespace fake4d {
struct Obj;
void install();
void reset();
Obj *new_object(bool collection = false);
void release(Obj *o);
int refcount(Obj *o);
void put_real(Obj *o, const char *key, double v);
void put_bool(Obj *o, const char *key, bool v);
void put_string(Obj *o, const char *key, const char *v);
void put_ref(Obj *o, const char *key, Obj *child); // retains child
bool get_string(Obj *o, const char *key, std::string *out);
size_t live_strings();
size_t live_handles();
size_t live_objects();
// next N EX_GET_OBJ_VALUE calls fail with an error and write a poison value
void fail_next_gets(int n);
const std::vector<std::string> &violations();
}
