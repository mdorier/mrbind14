#ifndef MRBIND14_FUNCTION_BINDER_H_
#define MRBIND14_FUNCTION_BINDER_H_

#include <mrbind14/mruby_util.hpp>
#include <mrbind14/type_traits.hpp>
#include <mrbind14/type_binder.hpp>
#include <mruby/proc.h>

namespace mrbind14 {

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
    return cpp_to_mrb<R>(mrb, function(std::forward<P>(params)...) );
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
    return cpp_to_mrb<R>(mrb, function(std::forward<P>(params)...) );
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
        mrb, fp, type_converter<P>::convert(mrb, args[I])...);
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
      // TODO translate C++ exception into Ruby exception
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
        mrb, fp, type_converter<P>::convert(mrb, args[I])...);
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
      // TODO translate C++ exception into Ruby exception
      throw;
    }
    return mrb_nil_value();
  }
};

/// Defines a function in a provided module, using a C-style function pointer
template<typename Function, typename ... Extra>
typename std::enable_if<is_function_pointer<Function>::value, void>::type
bind_function(mrb_state* mrb, RClass* mod, const char* name, Function&& function, const Extra&... extra) {
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

/// Defines a function using an std::function object passed by rvalue ref
template<typename R, typename ... P, typename ... Extra>
void bind_function(mrb_state* mrb, RClass* mod, const char* name, 
             std::function<R(P...)>&& function, const Extra&... extra) {
  // create function name as a symbol
  mrb_sym func_name_s = mrb_intern_cstr(mrb, name);
  // allocate memory to put the function object
  void* p = mrb_malloc_simple(mrb, sizeof(function));
  // create the function object in that memory
  auto fun_ptr = new(p) std::function<R(P...)>(std::move(function));
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

/// Defines a function using an std::function object passed by const lvalue ref
template<typename R, typename ... P, typename ... Extra>
void bind_function(mrb_state* mrb, RClass* mod, const char* name, 
             const std::function<R(P...)>& function, const Extra&... extra) {
  bind_function(mrb, mod, name, std::function<R(P...)>(function), extra...);
}

/// Defines a function using any object with a parenthesis operator (e.g. lambdas)
template<typename Function, typename ... Extra>
std::enable_if_t<!is_std_function_object<Function>::value
               && !is_function_pointer<Function>::value,
  void>
bind_function(mrb_state* mrb, RClass* mod, const char* name,
             Function&& function, const Extra&... extra) {
  using function_type = std::function<function_signature_t<std::decay_t<Function>>>;
  bind_function(mrb, mod, name, function_type(std::forward<Function>(function)), extra...);
}

}

#endif
