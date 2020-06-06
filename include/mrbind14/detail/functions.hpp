#ifndef MRBIND14_FUNCTION_H_
#define MRBIND14_FUNCTION_H_

#include <mruby.h>
#include <string>
#include <exception>

namespace mrbind14 {

namespace detail {

template<typename Function, typename ... Extra>
void def_function(mrb_state* mrb, const char* name, Function&& function, const Extra&... extra) {
  
}

}

}

#endif
