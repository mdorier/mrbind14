#include <mrbind14/mrbind14.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <iostream>

using namespace std::string_literals;

static void f1() {
    std::cout << "in f1" << std::endl;
}

static void f2(int, float, const std::string&, bool) {
    std::cout << "in f2" << std::endl;
}

static float f3() {
    std::cout << "in f3" << std::endl;
    return 4.9;
}

bool f4(int, float, const std::string&, bool) {
    std::cout << "in f4" << std::endl;
    return true;
}

bool f5_int(int x) {
    std::cout << "in f5_int" << std::endl;
    return true;
}

bool f5_int_float(int x, float y) {
    std::cout << "in f5_int_float" << std::endl;
    return true;
}

class function_test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( function_test );
    CPPUNIT_TEST( test_def_function );
    CPPUNIT_TEST( test_wrong_argument_type );
    CPPUNIT_TEST( test_wrong_num_arguments );
    CPPUNIT_TEST( test_undefined_function );
    CPPUNIT_TEST( test_def_std_function );
    CPPUNIT_TEST( test_def_lambda );
    CPPUNIT_TEST( test_def_function_object );
    //CPPUNIT_TEST( test_overload );
    CPPUNIT_TEST_SUITE_END();

    public:

    void setUp() {}
    void tearDown() {}

    void test_def_function() {
        mrbind14::interpreter mruby;

        mruby.def_function("f1", f1);
        mruby.def_function("f2", f2);
        mruby.def_function("f3", f3);
        mruby.def_function("f4", f4);

        std::string code = R"ruby(
            f1()
            #f2(1, 2.0, "Matthieu", true)
            #f3()
            #f4(1, 2.0, "Matthieu", true)
        )ruby";

        CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
    }

    void test_wrong_argument_type() {
        mrbind14::interpreter mruby;

        mruby.def_function("f2", f2);

        std::string code = R"ruby(
            f2("wrong", 2.0, "Matthieu", true)
        )ruby";

        CPPUNIT_ASSERT_THROW(mruby.execute(code.c_str()), std::bad_function_call);
    }

    void test_wrong_num_arguments() {
        mrbind14::interpreter mruby;

        mruby.def_function("f2", f2);

        std::string code = R"ruby(
            f2(1, 2.0, "Matthieu")
        )ruby";

        CPPUNIT_ASSERT_THROW(mruby.execute(code.c_str()), std::bad_function_call);
    }

    void test_undefined_function() {
        mrbind14::interpreter mruby;

        std::string code = R"ruby(
            f1()
        )ruby";

        CPPUNIT_ASSERT_THROW(mruby.execute(code.c_str()), std::runtime_error);
    }

    void test_def_std_function() {
        mrbind14::interpreter mruby;

        std::function<void(void)> f1 = 
            [](){ std::cout << "in f1" << std::endl; };
        std::function<void(int, float, const std::string&, bool)> f2 = 
            [](int, float, const std::string&, bool){ std::cout << "in f2" << std::endl; };
        std::function<float(void)> f3 = 
            [](){ std::cout << "in f3" << std::endl; return 4.9; };
        std::function<bool(int, float, const std::string&, bool)> f4 = 
            [](int, float, const std::string&, bool){ std::cout << "in f4" << std::endl; return true; };

        mruby.def_function("f1", f1);
        mruby.def_function("f2", f2);
        mruby.def_function("f3", f3);
        mruby.def_function("f4", f4);

        std::string code = R"ruby(
            f1()
            f2(1, 2.0, "Matthieu", true)
            f3()
            f4(1, 2.0, "Matthieu", true)
        )ruby";

        CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
    }

    void test_def_lambda() {
        mrbind14::interpreter mruby;

        mruby.def_function("f1", [](){ std::cout << "in f1" << std::endl; });
        mruby.def_function("f2", [](int, float, const std::string&, bool){ std::cout << "in f2" << std::endl; });
        mruby.def_function("f3", [](){ std::cout << "in f3" << std::endl; return 4.9; });
        mruby.def_function("f4", [](int, float, const std::string&, bool){ std::cout << "in f4" << std::endl; return true; });

        std::string code = R"ruby(
            f1()
            f2(1, 2.0, "Matthieu", true)
            f3()
            f4(1, 2.0, "Matthieu", true)
        )ruby";

        CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
    }

    void test_def_function_object() {
        mrbind14::interpreter mruby;

        mruby.def_function("f1", f1);
        mruby.def_function("f2", f2);
        mruby.def_function("f3", f3);
        mruby.def_function("f4", f4);

        std::string code = R"ruby(
            f1()
            f2(1, 2.0, "Matthieu", true)
            f3()
            f4(1, 2.0, "Matthieu", true)
        )ruby";

        CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));
    }

    void test_overload() {
        mrbind14::interpreter mruby;

        mruby.def_function("f5", f5_int);
        mruby.def_function("f5", f5_int_float);

        std::string code = R"ruby(
            f5(4)
            f5(4, 5.6)
        )ruby";

        CPPUNIT_ASSERT_NO_THROW(mruby.execute(code.c_str()));

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( function_test );
