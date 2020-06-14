#ifndef MRBIND14_TYPE_REGISTRY_H_
#define MRBIND14_TYPE_REGISTRY_H_

#include <mruby.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <string>
#include <typeindex>

namespace mrbind14 {

/// This function adds the name of type T into the $__cpp_class_name__ array
template<typename T>
void register_cpp_class_name(mrb_state* mrb, const char* name) {
  mrb_sym sym    = mrb_intern_lit(mrb, "$__cpp_class_names__");
  mrb_value hash = mrb_gv_get(mrb, sym);
  size_t id      = typeid(typename std::decay<T>::type).hash_code();
  mrb_value key  = mrb_fixnum_value(id);
  int ai         = mrb_gc_arena_save(mrb);
  mrb_value val  = mrb_str_new_cstr(mrb, name);
  mrb_hash_set(mrb, hash, key, val);
  mrb_gc_arena_restore(mrb, ai);
}

/// This function retrieves the name of the type T from the mrb_state
template<typename T>
std::string get_cpp_class_name(mrb_state* mrb) {
  mrb_sym sym    = mrb_intern_lit(mrb, "$__cpp_class_names__");
  mrb_value hash = mrb_gv_get(mrb, sym);
  auto id        = typeid(typename std::decay<T>::type).hash_code();
  mrb_value key  = mrb_fixnum_value(id);
  mrb_value def  = mrb_str_new_cstr(mrb, "???");
  mrb_value val  = mrb_hash_fetch(mrb, hash, key, def);
  return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
}

/// This function sets up the $__cpp_class_names__ global variable in the mrb_state
inline void init_cpp_class_names(mrb_state* mrb) {
  mrb_sym sym = mrb_intern_lit(mrb, "$__cpp_class_names__");
  mrb_value hash = mrb_gv_get(mrb, sym);
  if(!mrb_nil_p(hash)) return;
  hash = mrb_hash_new(mrb);
  mrb_gv_set(mrb, sym, hash);
  register_cpp_class_name<bool>(mrb, "bool");
  register_cpp_class_name<int>(mrb, "int");
  register_cpp_class_name<char>(mrb, "char");
  register_cpp_class_name<wchar_t>(mrb, "wchar");
  register_cpp_class_name<short>(mrb, "short");
  register_cpp_class_name<long>(mrb, "long");
  register_cpp_class_name<long long>(mrb, "long long");
  register_cpp_class_name<unsigned>(mrb, "unsigned");
  register_cpp_class_name<unsigned char>(mrb, "char");
  register_cpp_class_name<unsigned short>(mrb, "unsigned");
  register_cpp_class_name<unsigned long>(mrb, "unsigned");
  register_cpp_class_name<unsigned long long>(mrb, "unsigned long long");
  register_cpp_class_name<float>(mrb, "float");
  register_cpp_class_name<double>(mrb, "double");
  register_cpp_class_name<long double>(mrb, "long double");
}

}

#endif
