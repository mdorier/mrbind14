/*
   Copyright (c) 2020 Matthieu Dorier <matthieu.dorier@gmail.com>
   All rights reserved. Use of this source code is governed by a
   BSD-style license that can be found in the LICENSE file.
 */
#ifndef MRBIND14_MODULE_H_
#define MRBIND14_MODULE_H_

//#include <mrbind14/function_binder.hpp>
#include <mrbind14/cpp_function.hpp>
#include <mrbind14/type_binder.hpp>
#include <mruby/value.h>
#include <string>
#include <exception>

namespace mrbind14 {

class object;

class module {

    friend class object;

    public:

    template<typename Function, typename ... Extra>
    module& def_function(const char* name, Function&& f, const Extra&... extra) {
        void* p = mrb_malloc_simple(m_mrb, sizeof(function));
        auto fptr = new(p) function(name, std::forward<Function>(f), extra...);
        std::string name_cptr = std::string("__") + name + "__cptr__";
        mrb_sym name_cptr_sym = mrb_intern_cstr(m_mrb, name_cptr.c_str());
        mrb_mod_cv_set(m_mrb, m_module, name_cptr_sym, mrb_cptr_value(m_mrb, (void*)fptr));
        mrb_define_module_function(m_mrb, m_module, name, function_overload_resolver,  MRB_ARGS_ANY());
        return *this;
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
                detail::cpp_to_mrb(m_mrb, val));
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

    /**
     * @brief Checks if a particular function name is defined
     * in this module.
     *
     * @param function_name Name of the function
     *
     * @return true if function is defined, false otherwise.
     */
    bool respond_to(const std::string& function_name) const {
        mrb_sym function_name_sym = mrb_intern_cstr(m_mrb, function_name.c_str());
        return mrb_obj_respond_to(m_mrb, m_module, function_name_sym);
    }

    /**
     * @brief Cheks if a partifular class variable is defined
     * in this module.
     *
     * @param variable_name Name of the variable.
     *
     * @return true if the variable is defined, false otherwise.
     */
    bool cv_defined(const std::string& variable_name) const {
        mrb_sym variable_name_sym = mrb_intern_cstr(m_mrb, variable_name.c_str());
        return mrb_mod_cv_defined(m_mrb, m_module, variable_name_sym);
    }

    /**
     * @brief Get the value of a particular class variable from this module.
     * If the variable does not exist, nil is returned.
     *
     * @param variable_name Name of the variable.
     *
     * @return An object handle containing the value.
     */
    object cv_get(const std::string& variable_name) const {
        if(!cv_defined(variable_name)) return object(m_mrb);
        mrb_sym variable_name_sym = mrb_intern_cstr(m_mrb, variable_name.c_str());
        return object(m_mrb, mrb_mod_cv_get(m_mrb, m_module, variable_name_sym));
    }

    /**
     * @brief Set the value of a particular class variable in this module.
     *
     * @tparam ValueType Type of value.
     * @param variable_name Name of the variable.
     * @param val Value to set.
     */
    template<typename ValueType>
    void cv_set(const std::string& variable_name, const ValueType& val) {
        mrb_sym variable_name_sym = mrb_intern_cstr(m_mrb, variable_name.c_str());
        mrb_mod_cv_set(m_mrb, m_module, variable_name_sym, detail::cpp_to_mrb(m_mrb, val));
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
