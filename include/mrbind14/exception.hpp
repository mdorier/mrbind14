#ifndef MRBIND14_EXCEPTION_H_
#define MRBIND14_EXCEPTION_H_

#include <mrbind14/object.hpp>
#include <mruby.h>
#include <exception>

namespace mrbind14 {

class exception : public object, public std::exception {

  public:

  static void translate_and_throw_exception(mrb_state* mrb, mrb_value exc) {
    throw std::runtime_error("MRuby exception occured");
  }
};

}

#endif
