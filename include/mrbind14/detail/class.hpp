#ifndef MRBIND14_CLASS_H_
#define MRBIND14_CLASS_H_

#include <mrbind14/detail/types.hpp>
#include <mruby.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <string>
#include <typeindex>

namespace mrbind14 {

namespace detail {

/// Class with no parent Cpp class
template<typename CppClass, typename CppParentClass>
std::enable_if_t<
        std::is_same<CppParentClass, void>::value,
  RClass*>
def_class(mrb_state* mrb, RClass* module, const char* name) {
  RClass *cls = mrb_define_class_under(mrb, module, name, mrb->object_class);
  register_cpp_class_name<CppClass>(mrb, name);
  return cls;
}

/// Class with parent Cpp class
template<typename CppClass, typename CppParentClass>
std::enable_if_t<
        !std::is_same<CppParentClass, void>::value,
  RClass*>
def_class(mrb_state* mrb, RClass* module, const char* name) {
  // TODO
  RClass *cls = mrb_define_class_under(mrb, module, name, mrb->object_class);
  register_cpp_class_name<CppClass>(mrb, name);
  return cls;
}

}

}

#endif
