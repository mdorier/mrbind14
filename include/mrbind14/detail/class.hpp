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

template<typename CppClass, typename CppParentClass = void>
RClass* def_class(mrb_state* mrb, RClass* module, const char* name) {
  // TODO
  return nullptr;
}

}

}

#endif
