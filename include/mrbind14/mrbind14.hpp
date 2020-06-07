/*
 Copyright (c) 2020 Matthieu Dorier <matthieu.dorier@gmail.com>
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE file.
*/
#ifndef MRBIND14_HPP_
#define MRBIND14_HPP_

#include <mrbind14/detail/class.hpp>
#include <mrbind14/detail/functions.hpp>
#include <mruby.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>
#include <mruby/compile.h>
#include <string>
#include <exception>

namespace mrbind14 {

class interpreter;
class gem;
class exception;
template<typename CppClass, typename ... Options> class klass;
class module;
class object;

class object {

  friend class interpreter;
  friend class gem;
  friend class module;
  template<typename CppClass, typename ... > friend class klass;

  public:

  object(mrb_value val)
  : m_value(val) {}

  object()
  : m_value(mrb_nil_value()) {}

  template<typename T>
  object(mrb_state* mrb, T&& t)
  : m_value(detail::value_from_cpp(t)) {}

  template<typename T>
  bool convertible_to() const {
    return detail::check_type<T>(m_value);
  }

  template<typename T>
  T as() const {
    if(!detail::check_type<T>(m_value)) {
      throw std::runtime_error("Invalid type"); // XXX
    }
    return detail::value_to_cpp<T>(m_value);
  }

  template<typename Function>
  object& def_singleton_method(Function&& fun) {
    // TODO
    return *this;
  }

  private:

  mrb_value m_value;

};

class exception : public object, public std::exception {
  
  public:

  static void translate_and_throw_exception(mrb_state* mrb, mrb_value exc) {
    throw std::runtime_error("MRuby exception occured");
  }
};

template<typename CppClass, typename ... Options>
class klass {

  friend class gem;
  friend class module;

  public:

  template<typename ValueType>
  module& def_const(const char* name, const ValueType& val) {
    mrb_define_const(m_mrb,
                     m_class,
                     name,
                     detail::value_from_cpp(m_mrb, val));
    return *this;
  }

  template<typename Function, typename ... Extra>
  klass& def_method(const char* name, Function&& function, const Extra&... extra) {
    // TODO
    return *this;
  }

  template<typename Function, typename ... Extra>
  klass& def_class_method(const char* name, Function&& function, const Extra&... extra) {
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

  klass& include_module(const module& mod) {
    // TODO
    //mrb_include_module(m_mrb, m_class, mod.m_module);
    return *this;
  }

  private:

  mrb_state* m_mrb;
  std::string m_name;
  struct RClass *m_class;

  klass(mrb_state* mrb, struct RClass *cls, const char* name)
  : m_mrb(mrb), m_name(name), m_class(cls) {}

};

class module {
  
  friend class gem;

  public:

  template<typename Function, typename ... Extra>
  module& def_function(const char* name, Function&& function, const Extra&... extra) {
    detail::def_function(m_mrb, m_module, name, std::forward<Function>(function), extra...);
    return *this;
  }

  template<typename CppClass, typename CppParentClass = void>
  klass<CppClass> def_class(const char* name) {
    RClass* cls = detail::def_class<CppClass, CppParentClass>(m_mrb, m_module, name);
    return klass<CppClass>(m_mrb, name);
  }

  /**
   * @brief Defines a module inside this module.
   *
   * @param name Name of the new module.
   *
   * @return The newly created module.
   */
  module def_module(const char* name) {
    auto mod = mrb_define_module_under(m_mrb, m_module, name);
    return module(m_mrb, mod, name);
  }

  /**
   * @brief Defines a constant inside this module.
   *
   * @tparam ValueType Type of the value.
   * @param name Name of the constant.
   * @param val Value.
   *
   * @return A reference to the current module.
   */
  template<typename ValueType>
  module& def_const(const char* name, const ValueType& val) {
    mrb_define_const(m_mrb,
                     m_module,
                     name,
                     detail::value_from_cpp(m_mrb, val));
    return *this;
  }

  /**
   * @brief Includes a module inside the current module.
   *
   * @param mod Module.
   *
   * @return A reference to the current module.
   */
  module& include_module(const module& mod) {
    mrb_include_module(m_mrb, m_module, mod.m_module);
    return *this;
  }

  private:

  mrb_state* m_mrb;
  std::string m_name;
  struct RClass* m_module;

  module(mrb_state* mrb, struct RClass* mod, const char* name)
  : m_mrb(mrb), m_name(name), m_module(mod) {}

};

class gem {

  public:

  gem(mrb_state* mrb)
  : m_mrb(mrb) {
    detail::init_cpp_class_name(mrb);
  }

  gem(const gem&) = default;

  gem(gem&&) = default;

  gem& operator=(const gem&) = default;

  gem& operator=(gem&&) = default;

  virtual ~gem() = default;

  /**
   * @brief Defines a function at global scope. The function may be a function pointer
   * or an std::function<Ret(Params...)> objects. Lambdas (or any object with parenthesis
   * operator) may be used by first casting them into an std::function.
   *
   * @tparam Function Function type.
   * @tparam Extra Extra options.
   * @param name Name of the function.
   * @param function Function pointer or std::function object.
   * @param extra Extra argument.
   *
   * @return A reference to the current gem.
   */
  template<typename Function, typename ... Extra>
  gem& def_function(const char* name, Function&& function, const Extra&... extra) {
    detail::def_function(m_mrb, m_mrb->kernel_module, name, std::forward<Function>(function), extra...);
    return *this;
  }

  /**
   * @brief Exposes a C++ class as visible to Ruby at global scope.
   * Since Ruby enables only single-class inheritence, a single ParentCppClass
   * type may be provided. If provided, the parent class must have been exposed
   * to MRuby first.
   *
   * @tparam CppClass Type of C++ class.
   * @tparam ParentCppClass Parent C++ class.
   * @param name Name of the class.
   *
   * @return A handle to the newly created class.
   */
  template<typename CppClass, typename ParentCppClass = void>
  klass<CppClass> def_class(const char* name) {
    RClass* cls = detail::def_class<CppClass, ParentCppClass>(m_mrb, m_mrb->kernel_module, name);
    return klass<CppClass>(m_mrb, cls, name);
  }

  /**
   * @brief Defines a module at global scope.
   *
   * @param name Name of the new module.
   *
   * @return The newly created module.
   */
  module def_module(const char* name) {
    auto mod = mrb_define_module(m_mrb, name);
    return module(m_mrb, mod, name);
  }

  /**
   * @brief Defines a constant visible globally.
   *
   * @tparam ValueType Type of the value.
   * @param name Name of the constant.
   * @param val Value.
   *
   * @return A reference to the gem.
   */
  template<typename ValueType>
  gem& def_const(const char* name, const ValueType& val) {
    mrb_define_const(m_mrb,
                     m_mrb->kernel_module,
                     name, detail::value_from_cpp(m_mrb, val));
    return *this;
  }

  protected:

  mrb_state* m_mrb;

};

/**
 * @brief The interpreter object enables creating an MRuby state
 * and executing scripts from it. It extends the gem class, which
 * enables it to be extended with new modules, classes, and functions.
 */
class interpreter : public gem {

  public:

  /**
   * @brief Constructor. Creates a new MRuby state.
   */
  interpreter()
  : gem(mrb_open()) {}

  /**
   * @brief The copy-constructor is deleted.
   */
  interpreter(const interpreter&) = delete;

  /**
   * @brief Move constructor.
   */
  interpreter(interpreter&& other)
  : gem(std::move(other)) {
    other.m_mrb = nullptr;
  }

  /**
   * @brief The copy-assignment operator is deleted.
   */
  interpreter& operator=(const interpreter&) = delete;

  /**
   * @brief Move-assignment operator.
   */
  interpreter& operator=(interpreter&& other) {
    if(m_mrb == other.m_mrb) return *this;
    if(m_mrb) mrb_close(m_mrb);
    m_mrb = other.m_mrb;
    other.m_mrb = nullptr;
    return *this;
  }

  /**
   * @brief The destructor will close the underlying Mruby state
   * and free up its resources.
   */
  ~interpreter() {
    if(m_mrb) mrb_close(m_mrb);
  }

  /**
   * @brief Sets a global variable in the interpreter.
   *
   * @tparam ValueType Type of the value.
   * @param name Name of the global variable (including $).
   * @param val Value.
   */
  template<typename ValueType>
  void set_global_variable(const char* name, const ValueType& val) {
    mrb_sym sym = mrb_intern_static(m_mrb, name, strlen(name));
    mrb_gv_set(m_mrb, sym, detail::value_from_cpp<ValueType>(m_mrb, val));
  }

  /**
   * @brief Gets the value of a global variable.
   *
   * @tparam ValueType Type of the value.
   * @param name Name of the global variable (including $).
   *
   * @return The value associated with a global variable.
   */
  template<typename ValueType>
  ValueType get_global_variable(const char* name) {
    mrb_sym sym = mrb_intern_static(m_mrb, name, strlen(name));
    return detail::value_to_cpp<ValueType>(m_mrb, mrb_gv_get(m_mrb, sym));
  }

  /**
   * @brief Executes the given Ruby script, provided as a null-terminated string.
   *
   * @param script Ruby script.
   *
   * @return The value returned by the Ruby script.
   */
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
