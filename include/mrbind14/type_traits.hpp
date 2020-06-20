#ifndef MRBIND14_TYPE_TRAITS_H_
#define MRBIND14_TYPE_TRAITS_H_

#include <type_traits>
#include <string>

namespace mrbind14 {

/// Checks if a type is an integer but not a bool
template<typename T>
struct is_integer_not_bool {
  static constexpr bool value = 
    std::is_integral<std::decay_t<T>>::value &&
    !std::is_same<std::decay_t<T>, bool>::value;
};

/// Checks if a type is a floating point type
template<typename T>
struct is_floating_point {
  static constexpr bool value =
    std::is_floating_point<std::decay_t<T>>::value;
};

/// Checks if a type is a bool
template<typename T>
struct is_bool {
  static constexpr bool value =
    std::is_same<bool, std::decay_t<T>>::value;
};

/// Checks if a type is char* or const char*
template<typename T>
struct is_c_style_string {
  static constexpr bool value =
    std::is_same<std::decay_t<T>, char*>::value ||
    std::is_same<std::decay_t<T>, const char*>::value;
};

/// Checks if a type is an std::string
template<typename T>
struct is_string {
  static constexpr bool value =
    std::is_same<std::string, std::decay_t<T>>::value;
};

/// Removes the class component in member function types,
/// e.g. remove_class<R (C::*)(A...)>::type = R(A...)
template<typename T>
struct remove_class {};

template<typename C, typename R, typename... A>
struct remove_class<R (C::*)(A...)> {
    typedef R type(A...);
};

template<typename C, typename R, typename... A>
struct remove_class<R (C::*)(A...) const> {
    typedef R type(A...);
};

template<typename T>
using remove_class_t = typename remove_class<T>::type;

/// Extract the type of the operator() from the type of the object,
/// e.g. strip_function_object<std::function<R(A...)>>::type =
///      R (std::function<R(A...)>::*operator())(A...)
template<typename F>
struct strip_function_object {
    using type = typename remove_class<decltype(&F::operator())>::type;
};

/// Extracts the function signature from a function, function pointer or lambda.
template<typename Function, typename F = std::remove_reference_t<Function>>
using function_signature = std::conditional<
    std::is_function<F>::value,
    F,
    typename std::conditional<
        std::is_pointer<F>::value || std::is_member_pointer<F>::value,
        typename std::remove_pointer<F>::type,
        typename strip_function_object<F>::type
    >::type
>;

template<typename Function>
using function_signature_t = typename function_signature<Function>::type;

/// Checks if the provided type is std::function<...>
template<typename T>
struct is_std_function_object {
    static constexpr bool value = false;
};

template<typename R, typename ... A>
struct is_std_function_object<std::function<R(A...)>> {
    static constexpr bool value = true;
};

/// Helper type trait to detect function pointers
template<typename T>
struct is_function_pointer
{
  static const bool value =
    std::is_pointer<T>::value ?
    std::is_function<typename std::remove_pointer<T>::type>::value :
    false;
};

} // mrbind14

#endif
