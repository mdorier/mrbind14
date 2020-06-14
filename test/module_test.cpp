#include <mrbind14/mrbind14.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <iostream>

using namespace std::string_literals;



class module_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE( module_test );
  CPPUNIT_TEST( test_def_module );
  CPPUNIT_TEST( test_def_const );
  CPPUNIT_TEST( test_undefined_const );
  CPPUNIT_TEST_SUITE_END();

  public:

  void setUp() {}
  void tearDown() {}

  void test_def_module() {
    mrbind14::interpreter mruby;

    mruby.def_module("MyModule");

    std::string code = R"ruby(
      MyModule
    )ruby";

    CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
  }

  void test_def_const() {
    mrbind14::interpreter mruby;

    auto mod = mruby.def_module("MyModule");

    mod.def_const("INT_VAL", 42);          // int
    mod.def_const("FLOAT_VAL", 4.2);       // float
    mod.def_const("BOOL_VAL", true);       // bool
    mod.def_const("CSTR_VAL", "Matthieu"); // c-string
    mod.def_const("STR_VAL", "Lucas"s);    // string

    std::string code = R"ruby(
      print "MyModule::INT_VAL   = #{MyModule::INT_VAL}\n"
      print "MyModule::FLOAT_VAL = #{MyModule::FLOAT_VAL}\n"
      print "MyModule::BOOL_VAL  = #{MyModule::BOOL_VAL}\n"
      print "MyModule::CSTR_VAL  = #{MyModule::CSTR_VAL}\n"
      print "MyModule::STR_VAL   = #{MyModule::STR_VAL}\n"
    )ruby";

    CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
  }

  void test_undefined_const() {
    mrbind14::interpreter mruby;

    auto mod = mruby.def_module("MyModule");

    std::string code = R"ruby(
      print "MyModule::UNDEFINED = #{MyModule::UNEDFINED}\n"
    )ruby";
    CPPUNIT_ASSERT_THROW(mruby.execute(code.c_str()), std::runtime_error);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION( module_test );
