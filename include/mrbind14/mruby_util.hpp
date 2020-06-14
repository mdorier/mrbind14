#ifndef MRBIND14_MRUBY_UTIL_H_
#define MRBIND14_MRUBY_UTIL_H_

#include <mruby.h>
#include <mruby/class.h>

/// Helper function to define a class method
inline void mrb_define_class_method_raw(mrb_state *mrb, struct RClass *c, mrb_sym mid, mrb_method_t method)
{
  mrb_define_class_method(mrb, c, mrb_sym2name(mrb, mid), NULL, MRB_ARGS_ANY());
  mrb_define_method_raw(mrb, ((RObject*)c)->c, mid, method);
}

/// Function to raise an "invalid number of arguments" exception
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

/// Function to raise an "invalid argument type" exception
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

#endif
