#include <mrbind14/mrbind14.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <iostream>

using namespace std::string_literals;



class interpreter_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE( interpreter_test );
  CPPUNIT_TEST( test_execute );
  CPPUNIT_TEST( test_def_const );
  CPPUNIT_TEST( test_undefined_const );
  CPPUNIT_TEST( test_def_global );
  CPPUNIT_TEST_SUITE_END();

  public:

  void setUp() {}
  void tearDown() {}

  void test_execute() {
    mrbind14::interpreter mruby;

    std::string code = R"ruby(
      print "Hello World\n"
      42
    )ruby";

    CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
    CPPUNIT_ASSERT_EQUAL(42, mruby.execute(code.c_str()).as<int>());
  }

  void test_def_const() {
    mrbind14::interpreter mruby;

    mruby.def_const("INT_VAL", 42);          // int
    mruby.def_const("FLOAT_VAL", 4.2);       // float
    mruby.def_const("BOOL_VAL", true);       // bool
    mruby.def_const("CSTR_VAL", "Matthieu"); // c-string
    mruby.def_const("STR_VAL", "Lucas"s);    // string

    std::string code = R"ruby(
      print "INT_VAL   = #{INT_VAL}\n"
      print "FLOAT_VAL = #{FLOAT_VAL}\n"
      print "BOOL_VAL  = #{BOOL_VAL}\n"
      print "CSTR_VAL  = #{CSTR_VAL}\n"
      print "STR_VAL   = #{STR_VAL}\n"
    )ruby";

    CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
  }

  void test_def_global() {
    mrbind14::interpreter mruby;

    mruby.set_global("$int_val",   42);        // int
    mruby.set_global("$float_val", 4.2);       // float
    mruby.set_global("$bool_val", true);       // bool
    mruby.set_global("$cstr_val", "Matthieu"); // c-string
    mruby.set_global("$str_val",  "Lucas"s);   // string

    std::string code = R"ruby(
      print "$int_val   = #{$int_val}\n"
      print "$float_val = #{$float_val}\n"
      print "$bool_val  = #{$bool_val}\n"
      print "$cstr_val  = #{$cstr_val}\n"
      print "$str_val   = #{$str_val}\n"
    )ruby";

    CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
  }

  void test_undefined_const() {
    mrbind14::interpreter mruby;

    std::string code = R"ruby(
      print "UNDEFINED = #{UNEDFINED}\n"
    )ruby";
    CPPUNIT_ASSERT_THROW(mruby.execute(code.c_str()), std::runtime_error);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( interpreter_test );
