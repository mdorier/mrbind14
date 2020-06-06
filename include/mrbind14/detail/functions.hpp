#ifndef MRBIND14_FUNCTION_H_
#define MRBIND14_FUNCTION_H_

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <string>
#include <exception>
#include <iostream>
#include <mrbind14/detail/types.hpp>

namespace mrbind14 {

namespace detail {

/// Helper functionto define a class method
inline void mrb_define_class_method_raw(mrb_state *mrb, struct RClass *c, mrb_sym mid, mrb_method_t method)
{
  mrb_define_class_method(mrb, c, mrb_sym2name(mrb, mid), NULL, MRB_ARGS_ANY());
  mrb_define_method_raw(mrb, ((RObject*)c)->c, mid, method);
}

inline void raise_invalid_nargs(
    mrb_state *mrb,
    mrb_value func_name,
    int narg,
    int nparam) {
  // TODO
}

inline void raise_invalid_type(
    mrb_state *mrb,
    int parameter_index,
    const char* required_type_name,
    mrb_value value) {
  // TODO
}

/// Helper type trait to detect function pointers
template<typename T>
struct is_function_pointer
{
  static const bool value =
    std::is_pointer<T>::value ?
    std::is_function<typename std::remove_pointer<T>::type>::value :
    false;
};

/// Helper structure that makes a function return the ruby nil
/// value if the C function returns void
template<typename Function>
struct make_function_return_mrb_value {};

template<typename ... P>
struct make_function_return_mrb_value<void(*)(P...)> {
  static mrb_value call(mrb_state*, void(*function)(P...), P&&... params) {
    function(std::forward<P>(params)...);
    return mrb_nil_value();
  }
};

template<typename R, typename ... P>
struct make_function_return_mrb_value<R(*)(P...)> {
  static mrb_value call(mrb_state* mrb, R(*function)(P...), P&&... params) {
    return value_from_cpp<R>(mrb, function(std::forward<P>(params)...) );
  }
};

/// Helper structure to wrap C-style function pointers and convert arguments/return values.
template<typename Function>
struct function_binder {};

template<typename R, typename ... P>
struct function_binder<R (*)(P...)> {
  
  static constexpr int NPARAM = sizeof...(P);

  template<size_t ... I>
  static mrb_value apply_function(
      mrb_state* mrb,
      R(*fp)(P...),
      mrb_value* args,
      std::index_sequence<I...>) {
    return make_function_return_mrb_value<R (*)(P...)>::call(
        mrb, fp, type_converter<P>::convert(args[I])...);
  }

  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* args;
    int narg;
    mrb_get_args(mrb, "*", &args, &narg);
    if(narg != NPARAM)
      raise_invalid_nargs(mrb, mrb_cfunc_env_get(mrb, 1), narg, NPARAM);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    auto fp = (R(*)(P...))(mrb_cptr(cfunc));
    try {
      return apply_function(mrb, fp, args, std::index_sequence_for<P...>());
    } catch(...) {
      // TODO translate exception
      throw;
    }
    return mrb_nil_value();
  }
};

/// Defines a function in a provided module
template<typename Function, typename ... Extra>
typename std::enable_if<is_function_pointer<Function>::value, void>::type
def_function(mrb_state* mrb, RClass* mod, const char* name, Function&& function, const Extra&... extra) {
  mrb_sym func_name_s = mrb_intern_cstr(mrb, name);
  mrb_value env[] = {
    mrb_cptr_value(mrb, (void*)function),
    mrb_symbol_value(func_name_s),
  };
  RProc* proc = mrb_proc_new_cfunc_with_env(mrb, function_binder<Function>::call, 2, env);
  mrb_method_t method;
  MRB_METHOD_FROM_PROC(method, proc);
  if(mod == mrb->kernel_module) {
    mrb_define_method_raw(mrb, mod, func_name_s, method);
  } else {
    mrb_define_class_method_raw(mrb, mod, func_name_s, method);
  }
}

}

}

#endif
