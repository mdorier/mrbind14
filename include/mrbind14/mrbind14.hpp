/*
 Copyright (c) 2020 Matthieu Dorier <matthieu.dorier@gmail.com>
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE file.
*/
#ifndef MRBIND14_HPP_
#define MRBIND14_HPP_

#include <mruby.h>
#include <mruby/string.h>
#include <mruby/compile.h>
#include <string>
#include <exception>

namespace mrbind14 {

class interpreter;
class gem;
class exception;
template<typename CppClass> class klass;
class module;
class object;

class object {

  public:

  object(mrb_value val)
  : m_value(val) {}

  object()
  : m_value(mrb_nil_value()) {}

  template<typename T>
  object(mrb_state* mrb, T&& t)
  : m_value(from_cpp(t)) {}

  template<typename T>
  bool convertible_to() const {
    return check<T>(m_value);
  }

  template<typename T>
  T as() const {
    if(!check<T>(m_value)) {
      throw std::runtime_error("Invalid type"); // XXX
    }
    return to_cpp<T>(m_value);
  }

  private:

  /// Converts a C++ integer (int, short, long, etc.) into a ruby value
  template<typename Integer>
  static typename std::enable_if<
                    std::is_integral<typename std::decay<Integer>::type>::value
                    && !std::is_same<typename std::decay<Integer>::type, bool>::value,
  mrb_value>::type
  from_cpp(mrb_state*, Integer i) {
    return mrb_fixnum_value(i);
  }

  /// Converts a C++ float (float, double, etc.) into a ruby value
  template<typename Float>
  static typename std::enable_if<std::is_floating_point<Float>::value, mrb_value>::type
  from_cpp(mrb_state*, Float f) {
    return mrb_float_value(f);
  }

  /// Checks that the ruby value is convertible to a C++ numeric type
  template<typename Numeric>
  static typename std::enable_if<
                    std::is_arithmetic<Numeric>::value
                    && !std::is_same<Numeric, bool>::value,
                  bool>::type
  check(mrb_value v) {
    return mrb_fixnum_p(v) || mrb_float_p(v);
  }

  /// Converts a ruby value into an numeric type
  template<typename Numeric>
  static typename std::enable_if<
                    std::is_arithmetic<Numeric>::value
                    && !std::is_same<Numeric, bool>::value,
  Numeric>::type
  to_cpp(mrb_value v) {
    return mrb_fixnum_p(v) ? mrb_fixnum(v) : mrb_float(v);
  }

  /// Converts a C++ null-terminated string into a ruby value
  static mrb_value from_cpp(mrb_state* mrb, const char* str) {
    return mrb_str_new_cstr(mrb, str);
  }

  /// Converts a C++ string into a ruby value
  static mrb_value from_cpp(mrb_state* mrb, const std::string& str) {
    return from_cpp(mrb, str.c_str());
  }

  /// Checks that the ruby value can be converted into a C++ string
  template<typename String>
  static typename std::enable_if<std::is_same<String, std::string>::value, bool>::type
  check(mrb_value v) {
    return mrb_string_p(v);
  }

  /// Converts a ruby value into a C++ string
  template<typename String>
  static typename std::enable_if<std::is_same<String, std::string>::value, std::string>::type
  to_cpp(mrb_value v) {
    return std::string(RSTRING_PTR(v), RSTRING_LEN(v));
  }

  /// Converts a C++ bool into a ruby bool
  static mrb_value from_cpp(mrb_state* mrb, bool b) {
    return b ? mrb_true_value() : mrb_false_value();
  }

  /// Checks that the ruby value can be converted into a boolean
  template<typename Bool>
  static constexpr typename std::enable_if<std::is_same<Bool, bool>::value, bool>::type
  check(mrb_value v) {
    return true;
  }

  /// Converts a ruby value into a bool
  template<typename Bool>
  static constexpr typename std::enable_if<std::is_same<Bool, bool>::value, bool>::type
  to_cpp(mrb_value v) {
    return mrb_test(v);
  }

  mrb_value m_value;

};

class exception : public object, public std::exception {
  
  public:

  static void translate_and_throw_exception(mrb_state* mrb, mrb_value exc) {
    throw std::runtime_error("MRuby exception occured");
  }
};

template<typename CppClass>
class klass {

  friend class gem;
  friend class module;

  public:

  template<typename Function, typename ... Extra>
  klass& def_method(const char* name, Function&& function, const Extra&... extra) {
    // TODO
    return *this;
  }

  template<typename Type>
  klass& def_attr_reader(const char* name, const Type CppClass::*member) {
    // TODO
    return *this;
  }

  template<typename Type>
  klass& def_attr_writer(const char* name, const Type CppClass::*member) {
    // TODO
    return *this;
  }

  template<typename Type>
  klass& def_attr_accessor(const char* name, const Type CppClass::*member) {
    return def_attr_reader<Type>(name, member).template def_attr_writer<Type>(name, member);
  }

  private:

  mrb_state* m_mrb;
  std::string m_name;

  klass(mrb_state* mrb, const char* name)
  : m_mrb(mrb), m_name(name) {}

};

class module {
  
  friend class gem;

  public:

  template<typename Function, typename ... Extra>
  module& def_function(const char* name, Function&& function, const Extra&... extra) {
    // TODO
    return *this;
  }

  template<typename CppClass = void>
  klass<CppClass> def_class(const char* name) {
    // TODO
    return klass<CppClass>(m_mrb, name);
  }

  module def_module(const char* name) {
    // TODO
    return module(m_mrb, name);
  }

  private:

  mrb_state* m_mrb;
  std::string m_name;

  module(mrb_state* mrb, const char* name)
  : m_mrb(mrb), m_name(name) {}

};

class gem {

  public:

  gem(mrb_state* mrb)
  : m_mrb(mrb) {}

  gem(const gem&) = default;

  gem(gem&&) = default;

  gem& operator=(const gem&) = default;

  gem& operator=(gem&&) = default;

  virtual ~gem() = default;

  template<typename Function, typename ... Extra>
  gem& def_function(const char* name, Function&& function, const Extra&... extra) {
    // TODO
    return *this;
  }

  template<typename CppClass = void>
  klass<CppClass> def_class(const char* name) {
    // TODO
    return klass<CppClass>(m_mrb, name);
  }

  module def_module(const char* name) {
    // TODO
    return module(m_mrb, name);
  }

  protected:

  mrb_state* m_mrb;

};

class interpreter : public gem {

  public:

  interpreter()
  : gem(mrb_open()) {}

  interpreter(const interpreter&) = delete;

  interpreter(interpreter&& other)
  : gem(std::move(other)) {
    other.m_mrb = nullptr;
  }

  interpreter& operator=(const interpreter&) = delete;

  interpreter& operator=(interpreter&& other) {
    if(m_mrb == other.m_mrb) return *this;
    if(m_mrb) mrb_close(m_mrb);
    m_mrb = other.m_mrb;
    other.m_mrb = nullptr;
    return *this;
  }

  ~interpreter() {
    if(m_mrb) mrb_close(m_mrb);
  }

  object execute(const char* script) {
    auto val = mrb_load_string(m_mrb, script);
    if(m_mrb->exc) {
      exception::translate_and_throw_exception(m_mrb, mrb_obj_value(m_mrb->exc));
    }
    return object(val);
  }

};

}

#endif
