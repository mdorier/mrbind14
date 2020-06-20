#ifndef MRBIND14_CPP_FUNCTION_H
#define MRBIND14_CPP_FUNCTION_H

#include <mrbind14/type_traits.hpp>
#include <mrbind14/type_binder.hpp>
#include <mruby.h>
#include <vector>
#include <functional>
#include <iostream>

namespace mrbind14 {

namespace detail {

/// Helper structure that makes a function return the ruby nil
/// value if the C function returns void
template<typename Function>
struct make_function_return_mrb_value {};

template<typename ... P>
struct make_function_return_mrb_value<std::function<void(P...)>> {
  static mrb_value call(mrb_state*, const std::function<void(P...)>& f, P&&... params) {
    f(std::forward<P>(params)...);
    return mrb_nil_value();
  }
};

template<typename R, typename ... P>
struct make_function_return_mrb_value<std::function<R(P...)>> {
  static mrb_value call(mrb_state* mrb, const std::function<R(P...)>& f, P&&... params) {
    return cpp_to_mrb<R>(mrb, f(std::forward<P>(params)...) );
  }
};

class abstract_function {

    public:

    abstract_function() = default;

    abstract_function(const abstract_function&) = default;

    abstract_function(abstract_function&&) = default;

    abstract_function& operator=(abstract_function&&) = default;

    abstract_function& operator=(const abstract_function&) = default;

    virtual ~abstract_function() = default;

    virtual mrb_value call(mrb_state* mrb, unsigned nargs, mrb_value* args) const = 0;

    virtual bool check_args(mrb_state* mrb, unsigned nargs, mrb_value* args) const = 0;

    virtual std::string signature(mrb_state* mrb) const = 0;

};

template<typename F>
class function_impl;

template<typename R, typename ... P>
class function_impl<R(P...)> : public abstract_function {
    
    public:

    template<typename ... Extra>
    function_impl(std::function<R(P...)>&& fun, const Extra&... extra)
    : m_function(std::move(fun)) {}
    
    template<typename ... Extra>
    function_impl(const std::function<R(P...)>& fun, const Extra&... extra)
    : function_impl(std::function<R(P...)>(fun), extra...) {}

    mrb_value call(mrb_state* mrb, unsigned nargs, mrb_value* args) const override {
        if(!check_args(mrb, nargs, args)) throw std::bad_function_call();
        return apply_function(mrb, args, std::index_sequence_for<P...>());
    }

    bool check_args(mrb_state* mrb, unsigned nargs, mrb_value* args) const override {
        if(nargs != sizeof...(P)) return false;
        return check_arg_types<P...>(mrb, args, false);
    }

    std::string signature(mrb_state* mrb) const override {
        std::string result = "(";
        std::vector<std::string> arg_types =
            { get_cpp_class_name<P>(mrb)... };
        auto i = arg_types.size();
        for(const auto& arg_type : arg_types) {
            i -= 1;
            result += arg_type;
            if(i != 0) result += ", ";
        }
        result += ") -> " + get_cpp_class_name<R>(mrb);
        return result;
    }

    private:

    template<size_t ... I>
    mrb_value apply_function(mrb_state* mrb, mrb_value* args, std::index_sequence<I...>) const {
        return make_function_return_mrb_value<decltype(m_function)>::call(
            mrb, m_function, type_converter<P>::convert(mrb, args[I])...);
    }

    std::function<R(P...)> m_function;
};

// Make a function from a std::function rvalue ref
template<typename R, typename ... P, typename ... Extra>
std::unique_ptr<abstract_function>
make_function(std::function<R(P...)>&& f, const Extra&... extra) {
    using function_type = function_impl<R(P...)>;
    return std::make_unique<function_type>(std::move(f), extra...);
}

// Make a function from an const std::function ref
template<typename R, typename ... P, typename ... Extra>
std::unique_ptr<abstract_function>
make_function(const std::function<R(P...)>& f, const Extra&... extra) {
    return make_function(std::function<R(P...)>(f), extra...);
}

// Make a function from a function pointer
template<typename R, typename ... Params, typename ... Extra>
std::unique_ptr<abstract_function>
make_function(R(*f)(Params...), const Extra&... extra) {
    using function_type = function_impl<R(Params...)>;
    return std::make_unique<function_type>(f, extra...);
}

// Make a function from any other object (lambda, object with operator(), etc.)
template<typename Function, typename ... Extra>
std::enable_if_t< !is_std_function_object<std::decay_t<Function>>::value
               && !std::is_function<std::remove_pointer_t<Function>>::value,
    std::unique_ptr<abstract_function>>
make_function(Function f, const Extra&... extra) {
    using function_type = function_impl<function_signature_t<std::decay_t<Function>>>;
    using std_function_type = std::function<function_signature_t<std::decay_t<Function>>>;
    return std::make_unique<function_type>(std_function_type(std::forward<Function>(f)), extra...);
} 

} // namespace detail

class function {

    public:

    function() = default;

    template<typename Function, typename ... Extra>
    function(std::string name, Function fun, const Extra&... extra)
    : m_name(std::move(name))
    , m_impl(detail::make_function(fun, extra...)) {}

    function(const function& other) = delete;

    function(function&&) = default;

    function& operator=(const function&) = delete;

    function& operator=(function&&) = default;

    ~function() = default;

    mrb_value call(mrb_state* mrb, unsigned nargs, mrb_value* args) const {
        if(m_impl) return m_impl->call(mrb, nargs, args);
        else throw std::bad_function_call();
    }

    bool check_args(mrb_state* mrb, unsigned nargs, mrb_value* args) const {
        if(m_impl) return m_impl->check_args(mrb, nargs, args);
        else return false;
    }

    std::string signature(mrb_state* mrb) const {
        if(m_impl) return m_impl->signature(mrb);
        else return std::string();
    }

    const std::string& name() const {
        return m_name;
    }

    private:

    std::string                                m_name;
    std::unique_ptr<detail::abstract_function> m_impl;

};

inline mrb_value function_overload_resolver(mrb_state* mrb, mrb_value self) {
    RClass* mod = mrb_class(mrb, self);
    // get arguments
    mrb_value* args;
    uint32_t narg;
    mrb_get_args(mrb, "*", &args, &narg);
    // get current method name
    mrb_value kernel = mrb_class_find_path(mrb, mrb->kernel_module);
    mrb_value fun_name_val = mrb_funcall(mrb, kernel, "__method__", 0);
    std::string fun_name = detail::mrb_to_cpp<std::string>(mrb, fun_name_val);
    // make instance name
    std::string name_cptr = std::string("__") + fun_name + "__cptr__";
    // retrieve function pointer
    mrb_sym name_cptr_sym = mrb_intern_cstr(mrb, name_cptr.c_str());
    mrb_value cptr_val = mrb_mod_cv_get(mrb, mod, name_cptr_sym);
    auto fptr = static_cast<function*>(mrb_cptr(cptr_val));
    // call the function
    return fptr->call(mrb, narg, args);
}

} // namespace mrbind14

#endif
