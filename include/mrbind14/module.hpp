/*
 Copyright (c) 2020 Matthieu Dorier <matthieu.dorier@gmail.com>
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE file.
*/
#ifndef MRBIND14_MODULE_H_
#define MRBIND14_MODULE_H_

#include <mrbind14/function_binder.hpp>
#include <string>
#include <exception>

namespace mrbind14 {

class object;

class module {
  
  friend class object;

  public:

  template<typename Function, typename ... Extra>
  module& def_function(const char* name, Function&& function, const Extra&... extra) {
    bind_function(m_mrb, m_module, name, std::forward<Function>(function), extra...);
    return *this;
  }

#if 0
  template<typename CppClass, typename CppParentClass = void>
  klass<CppClass> def_class(const char* name) {
    RClass* cls = detail::def_class<CppClass, CppParentClass>(m_mrb, m_module, name);
    return klass<CppClass>(m_mrb, name);
  }
#endif

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
                     cpp_to_mrb(m_mrb, val));
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

  protected:

  mrb_state*     m_mrb    = nullptr;
  std::string    m_name   = "";
  struct RClass* m_module = nullptr;

  module(mrb_state* mrb)
  : m_mrb(mrb), m_name("Kernel"), m_module(m_mrb->kernel_module) {}

  module(mrb_state* mrb, struct RClass* mod, const char* name)
  : m_mrb(mrb), m_name(name), m_module(mod) {}

};

}

#endif
