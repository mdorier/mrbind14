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
  mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S': wrong number of arguments (%S for %S)",
             func_name,
             mrb_fixnum_value(narg),
             mrb_fixnum_value(nparam));
}

inline void raise_invalid_type(
    mrb_state *mrb,
    int parameter_index,
    const char* required_type_name,
    mrb_value value) {
  const char * argument_class_name = mrb_obj_classname(mrb, value);
  mrb_raisef(mrb, E_TYPE_ERROR, "Cannot convert %S into %S (argument %S)",
             mrb_str_new_cstr(mrb, argument_class_name),
             mrb_str_new_cstr(mrb, required_type_name),
             mrb_fixnum_value(parameter_index + 1));
}

/// Helper structure to check the types of a series of values
template<class ... P>
struct type_checker {};

template<>
struct type_checker<> {
  static void check(mrb_state* mrb, int i, mrb_value* args) {}
};

template<class P>
struct type_checker<P> {
  static void check(mrb_state* mrb, int i, mrb_value* args) {
    if(!check_type<P>(args[i])) {
      auto type_name = get_cpp_class_name<P>(mrb);
      raise_invalid_type(mrb, i, type_name.c_str(), args[i]);
    }
  }
};

template<class P1, class ... P>
struct type_checker<P1, P...> {
  static void check(mrb_state* mrb, int i, mrb_value* args) {
    if(!check_type<P1>(args[i])) {
      auto type_name = get_cpp_class_name<P1>(mrb);
      raise_invalid_type(mrb, i, type_name.c_str(), args[i]);
    } else {
      type_checker<P...>::check(mrb, i+1, args);
    }
  }
};

template<class ... P>
void check_arg_types(mrb_state* mrb, mrb_value* args) {
  type_checker<P...>::check(mrb, 0, args);
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

template<typename ... P>
struct make_function_return_mrb_value<std::function<void(P...)>> {
  static mrb_value call(mrb_state*, const std::function<void(P...)>& function, P&&... params) {
    function(std::forward<P>(params)...);
    return mrb_nil_value();
  }
};

template<typename R, typename ... P>
struct make_function_return_mrb_value<std::function<R(P...)>> {
  static mrb_value call(mrb_state* mrb, const std::function<R(P...)>& function, P&&... params) {
    return value_from_cpp<R>(mrb, function(std::forward<P>(params)...) );
  }
};

/// Helper structure to wrap functions and convert arguments/return values.
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
    check_arg_types<P...>(mrb, args);
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

template<typename R, typename ... P>
struct function_binder<std::function<R(P...)>> {
  
  static constexpr int NPARAM = sizeof...(P);

  template<size_t ... I>
  static mrb_value apply_function(
      mrb_state* mrb,
      const std::function<R(P...)>& fp,
      mrb_value* args,
      std::index_sequence<I...>) {
    return make_function_return_mrb_value<std::function<R(P...)>>::call(
        mrb, fp, type_converter<P>::convert(args[I])...);
  }

  static mrb_value call(mrb_state* mrb, mrb_value self) {
    mrb_value* args;
    int narg;
    mrb_get_args(mrb, "*", &args, &narg);
    if(narg != NPARAM)
      raise_invalid_nargs(mrb, mrb_cfunc_env_get(mrb, 1), narg, NPARAM);
    check_arg_types<P...>(mrb, args);
    mrb_value cfunc = mrb_cfunc_env_get(mrb, 0);
    auto fp = static_cast<std::function<R(P...)>*>(mrb_cptr(cfunc));
    try {
      return apply_function(mrb, *fp, args, std::index_sequence_for<P...>());
    } catch(...) {
      // TODO translate exception
      throw;
    }
    return mrb_nil_value();
  }
};

/// Defines a function in a provided module, using a C-style function pointer
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

template<typename R, typename ... P, typename ... Extra>
void def_function(mrb_state* mrb, RClass* mod, const char* name, 
             const std::function<R(P...)>& function, const Extra&... extra) {
  mrb_sym func_name_s = mrb_intern_cstr(mrb, name);
  void* p = mrb_malloc_simple(mrb, sizeof(function));
  auto fun_ptr = new(p) std::function<R(P...)>(function);
  mrb_value env[] = {
    mrb_cptr_value(mrb, static_cast<void*>(fun_ptr)),
    mrb_symbol_value(func_name_s),
  };
  RProc* proc = mrb_proc_new_cfunc_with_env(mrb,
      function_binder<std::function<R(P...)>>::call, 2, env);
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
