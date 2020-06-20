#ifndef MRBIND14_TYPE_BINDER_H_
#define MRBIND14_TYPE_BINDER_H_

#include <mruby.h>
#include <mruby/string.h>
#include <mrbind14/mruby_util.hpp>
#include <mrbind14/type_registry.hpp>
#include <mrbind14/type_traits.hpp>

namespace mrbind14 {

namespace detail {

/// The type_binder structure provides three static functions:
/// - cpp_to_mrb converts a C++ value to an mrb_value
/// - mrb_to_cpp converts an mrb_value to a C++ value
/// - check_type checks if an mrb_value is convertible to the given C++ type

template<typename T, typename Enable = void>
struct type_binder;

template<typename Value>
struct type_binder<Value,
  std::enable_if_t<
    std::is_same<std::decay_t<Value>, mrb_value>::value>> {

  static constexpr mrb_value cpp_to_mrb(mrb_state* mrb, mrb_value val) {
    return val;
  }

  static constexpr auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    return val;
  }

  static constexpr bool check_type(mrb_state* mrb, mrb_value val) {
    return true;
  }

};

template<typename Integer>
struct type_binder<Integer, std::enable_if_t<is_integer_not_bool<Integer>::value>> {

  static mrb_value cpp_to_mrb(mrb_state* mrb, Integer i) {
    return mrb_fixnum_value(i);
  }

	static std::decay_t<Integer> mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    return mrb_fixnum_p(val) ? mrb_fixnum(val) : mrb_float(val);
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return mrb_fixnum_p(val) || mrb_float_p(val);
  }

};

template<typename Float>
struct type_binder<Float, std::enable_if_t<is_floating_point<Float>::value>> {
  
  static mrb_value cpp_to_mrb(mrb_state* mrb, Float f) {
    return mrb_float_value(mrb, f);
  }

  static auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    return mrb_fixnum_p(val) ? mrb_fixnum(val) : mrb_float(val);
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return mrb_fixnum_p(val) || mrb_float_p(val);
  }

};

template<typename Bool>
struct type_binder<Bool, std::enable_if_t<is_bool<Bool>::value>> {
  
  static mrb_value cpp_to_mrb(mrb_state* mrb, Bool b) {
    return b ? mrb_true_value() : mrb_false_value();
  }

  static auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    return mrb_test(val);
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return true;
  }
};

template<typename CString>
struct type_binder<CString, std::enable_if_t<is_c_style_string<CString>::value>> {
  
  static mrb_value cpp_to_mrb(mrb_state* mrb, CString str) {
    int ai = mrb_gc_arena_save(mrb);
    auto val = mrb_str_new_cstr(mrb, str);
    mrb_gc_arena_restore(mrb, ai);
    return val;
  }

  static auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    if(val.tt == MRB_TT_SYMBOL) {
		val = mrb_sym2str(mrb, val.value.i);
	}
    return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return mrb_string_p(val) || mrb_symbol_p(val);
  }

};

template<typename String>
struct type_binder<String, std::enable_if_t<is_string<String>::value>> {

  static mrb_value cpp_to_mrb(mrb_state* mrb, String str) {
    int ai = mrb_gc_arena_save(mrb);
    auto val = mrb_str_new(mrb, str.data(), str.size());
    mrb_gc_arena_restore(mrb, ai);
    return val;
  }

  static auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    if(val.tt == MRB_TT_SYMBOL) {
		val = mrb_sym2str(mrb, val.value.i);
	}
    return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return mrb_string_p(val) || mrb_symbol_p(val);
  }

};


template<typename T>
mrb_value cpp_to_mrb(mrb_state* mrb, T val) {
  return type_binder<T>::cpp_to_mrb(mrb, val);
}

template<typename T>
T mrb_to_cpp(mrb_state* mrb, mrb_value val) {
  return type_binder<T>::mrb_to_cpp(mrb, val);
}

template<typename T>
bool check_type(mrb_state* mrb, mrb_value val) {
  return type_binder<T>::check_type(mrb, val);
}

/// Helper structure to check the types of a series of values
template<class ... P>
struct type_checker {};

template<>
struct type_checker<> {
  static bool check(mrb_state* mrb, int i, mrb_value* args, bool should_throw) {
    return true;
  }
};

template<class P>
struct type_checker<P> {
  static bool check(mrb_state* mrb, int i, mrb_value* args, bool should_throw) {
    if(!type_binder<P>::check_type(mrb, args[i])) {
      if(should_throw) {
        auto type_name = get_cpp_class_name<P>(mrb);
        raise_invalid_type(mrb, i, type_name.c_str(), args[i]);
      }
      return false;
    }
    return true;
  }
};

template<class P1, class ... P>
struct type_checker<P1, P...> {
  static bool check(mrb_state* mrb, int i, mrb_value* args, bool should_throw) {
    if(!type_binder<P1>::check_type(mrb, args[i])) {
      if(should_throw) {
        auto type_name = get_cpp_class_name<P1>(mrb);
        raise_invalid_type(mrb, i, type_name.c_str(), args[i]);
      }
      return false;
    } else {
      return type_checker<P...>::check(mrb, i+1, args, should_throw);
    }
  }
};

/// Checks that a C-style array of arguments matches the provided types
template<class ... P>
bool check_arg_types(mrb_state* mrb, mrb_value* args, bool should_throw=true) {
  return type_checker<P...>::check(mrb, 0, args, should_throw);
}

/// Helper structure for parameter pack expension in function_binder.hpp
template<typename T>
struct type_converter {
  static std::decay_t<T> convert(mrb_state* mrb, mrb_value v) {
    return mrb_to_cpp<std::decay_t<T>>(mrb, v);
  }
};

} // namespace detail

} // namespace mrbind14

#include <mrbind14/object.hpp>

namespace mrbind14 {

namespace detail {

template<typename Object>
struct type_binder<Object, std::enable_if_t<std::is_same<std::decay_t<Object>,object>::value>> {

  static mrb_value cpp_to_mrb(mrb_state* mrb, Object val) {
    return val.native();
  }

  static auto mrb_to_cpp(mrb_state* mrb, mrb_value val) {
    return object(mrb, val);
  }

  static bool check_type(mrb_state* mrb, mrb_value val) {
    return true;
  }

};

} // namespace detail

} // namespace mrbind14

#endif
