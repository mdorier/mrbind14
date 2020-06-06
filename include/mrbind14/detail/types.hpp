#ifndef MRBIND14_TYPES_H_
#define MRBIND14_TYPES_H_

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

  /// Converts a C++ integer (int, short, long, etc.) into a ruby value
  template<typename Integer>
  static typename std::enable_if<
                    std::is_integral<typename std::decay<Integer>::type>::value
                    && !std::is_same<typename std::decay<Integer>::type, bool>::value,
  mrb_value>::type
  value_from_cpp(mrb_state*, Integer i) {
    return mrb_fixnum_value(i);
  }

  /// Converts a C++ float (float, double, etc.) into a ruby value
  template<typename Float>
  static typename std::enable_if<std::is_floating_point<Float>::value, mrb_value>::type
  value_from_cpp(mrb_state* mrb, Float f) {
    return mrb_float_value(mrb, f);
  }

  /// Checks that the ruby value is convertible to a C++ numeric type
  template<typename Numeric>
  static typename std::enable_if<
                    std::is_arithmetic<Numeric>::value
                    && !std::is_same<Numeric, bool>::value,
                  bool>::type
  check_type(mrb_value v) {
    return mrb_fixnum_p(v) || mrb_float_p(v);
  }

  /// Converts a ruby value into an numeric type
  template<typename Numeric>
  static typename std::enable_if<
                    std::is_arithmetic<Numeric>::value
                    && !std::is_same<Numeric, bool>::value,
  Numeric>::type
  value_to_cpp(mrb_value v) {
    return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v);
  }

  /// Converts a C++ null-terminated string into a ruby value
  static mrb_value value_from_cpp(mrb_state* mrb, const char* str) {
    return mrb_str_new_cstr(mrb, str);
  }

  /// Converts a C++ string into a ruby value
  static mrb_value value_from_cpp(mrb_state* mrb, const std::string& str) {
    return value_from_cpp(mrb, str.c_str());
  }

  /// Checks that the ruby value can be converted into a C++ string
  template<typename String>
  static typename std::enable_if<std::is_same<String, std::string>::value, bool>::type
  check_type(mrb_value v) {
    return mrb_string_p(v);
  }

  /// Converts a ruby value into a C++ string
  template<typename String>
  static typename std::enable_if<std::is_same<String, std::string>::value, std::string>::type
  value_to_cpp(mrb_value v) {
    return std::string(RSTRING_PTR(v), RSTRING_LEN(v));
  }

  /// Converts a C++ bool into a ruby bool
  static mrb_value value_from_cpp(mrb_state* mrb, bool b) {
    return b ? mrb_true_value() : mrb_false_value();
  }

  /// Checks that the ruby value can be converted into a boolean
  template<typename Bool>
  static constexpr typename std::enable_if<std::is_same<Bool, bool>::value, bool>::type
  check_type(mrb_value v) {
    return true;
  }

  /// Converts a ruby value into a bool
  template<typename Bool>
  static constexpr typename std::enable_if<std::is_same<Bool, bool>::value, bool>::type
  value_to_cpp(mrb_value v) {
    return mrb_test(v);
  }

  /// Helper structure for parameter pack expension in functions.hpp
  template<typename T>
  struct type_converter {

    static T convert(mrb_value v) {
      return value_to_cpp<T>(v);
    }

  };

  /// This function adds the name of type T into the $__cpp_class_name__ array
  template<typename T>
  void register_cpp_class_name(mrb_state* mrb, const char* name) {
    mrb_sym sym = mrb_intern_lit(mrb, "$__cpp_class_names__");
    mrb_value hash = mrb_gv_get(mrb, sym);
    auto id = typeid(typename std::decay<T>::type).hash_code();
    auto key = value_from_cpp(mrb, id);
    auto val = value_from_cpp(mrb, name);
    mrb_hash_set(mrb, hash, key, val);
  }

  /// This function retrieves the name of the type T from the mrb_state
  template<typename T>
  std::string get_cpp_class_name(mrb_state* mrb) {
    mrb_sym sym = mrb_intern_lit(mrb, "$__cpp_class_names__");
    mrb_value hash = mrb_gv_get(mrb, sym);
    auto id = typeid(typename std::decay<T>::type).hash_code();
    mrb_value key = value_from_cpp(mrb, id);
    mrb_value def = value_from_cpp(mrb, "???");
    mrb_value val = mrb_hash_fetch(mrb, hash, key, def);
    return value_to_cpp<std::string>(val);
  }

  /// This function sets up the $__cpp_class_name__ global variable in the mrb_state
  inline void init_cpp_class_name(mrb_state* mrb) {
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
  }

}

}

#endif
