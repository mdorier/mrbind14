#ifndef MRBIND14_OBJECT_H_
#define MRBIND14_OBJECT_H_

#include <mruby.h>

namespace mrbind14 {

class object {

  public:

    object(mrb_state* mrb)
    : m_mrb(mrb)
    , m_value(mrb_nil_value()) {}

    object(mrb_state* mrb, mrb_value val)
    : m_mrb(mrb)
    , m_value(val) {}

    template<typename T>
    object(mrb_state* mrb, T&& val);

    template<typename T>
    auto as() const; 

    object(const object&) = default;

    object(object&&) = default;

    object& operator=(const object&) = default;

    object& operator=(object&&) = default;

    virtual ~object() = default;

    mrb_state* mrb() const { return m_mrb; }

    mrb_value native() const { return m_value; }

  private:

    mrb_state* m_mrb;
    mrb_value  m_value;
};

}

#include <mrbind14/type_binder.hpp>

namespace mrbind14 {

template<typename T>
object::object(mrb_state* mrb, T&& val)
: m_mrb(mrb)
, m_value(cpp_to_mrb(mrb, val)) {}

template<typename T>
auto object::as() const {
  return mrb_to_cpp<T>(m_mrb, m_value);
}

}

#endif
