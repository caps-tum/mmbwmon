// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

/*
This example is the same as ex2 except that the macros are in uppercase.
 */

#include <stdexcept>
#include <cmath>

#include <fructose/fructose.h>

const int life_the_available_tests_and_everything = 41;
const int the_neighbour_of_the_beast = 668;
const int is_alive = 6;

// These tests are rigged so that some of them fail.
// The tests include exercising floating point comparisons
// and exception handling.

namespace {
    void throw_a_runtime_error()
    {
        throw std::runtime_error("this is a runtime error");
    }
}

struct simpletest : public fructose::test_base<simpletest>
{
    void test42(const std::string& test_name)
    {
        FRUCTOSE_ASSERT(life_the_available_tests_and_everything == 6*7);
    }

    void testbeast(const std::string& test_name)
    {
        FRUCTOSE_ASSERT(the_neighbour_of_the_beast == 666);
    }

    void testfivealive(const std::string& test_name)
    {
        const int five = 5;
        FRUCTOSE_ASSERT_EQ(five, is_alive);
    }

    void testexceptions(const std::string& test_name)
    {
        FRUCTOSE_ASSERT_EXCEPTION(throw_a_runtime_error(), std::logic_error);
        FRUCTOSE_ASSERT_NO_EXCEPTION(throw_a_runtime_error());
    }

    void testloopdata(const std::string& test_name)
    {
        static const struct {
            int line_number;
            int a;
            int b;
            int expected_result;
        } data[] = {
            {__LINE__, 3, 4, 12}
          , {__LINE__, 1, 50, 50}
          , {__LINE__, 5, 12, 60}
          , {__LINE__, 6, 6, 37}
          , {__LINE__, 7, 10, 70}
        };

        for (std::size_t i = 0; i < sizeof(data)/sizeof(data[0]); ++i)
        {
            if (verbose())
            {
                std::cout << "Testing to see if "
                          << data[i].a << " * " << data[i].b 
                          << " = " << data[i].expected_result
                          << std::endl;
            }
            int result = data[i].a * data[i].b;
            FRUCTOSE_LOOP1_ASSERT(data[i].line_number, i,
                                 result == data[i].expected_result);
        }
    }

    void testfloatingpoint(const std::string& test_name)
    {
        double my_pi = 3.141592654;
        double calc_pi = 4.0 * atan(1.0);
        FRUCTOSE_ASSERT_DOUBLE_EQ_REL_ABS(my_pi, calc_pi, 0.01, 0.01);
        FRUCTOSE_ASSERT_DOUBLE_EQ(my_pi, calc_pi);
        FRUCTOSE_ASSERT_DOUBLE_NE(my_pi, calc_pi);
        FRUCTOSE_ASSERT_DOUBLE_NE_REL_ABS(my_pi, calc_pi, 0.01, 0.01);
    }
};

int main(int argc, char** argv)
{
    simpletest tests;
    tests.add_test("test42", &simpletest::test42);
    tests.add_test("testbeast", &simpletest::testbeast);
    tests.add_test("fiveisalive", &simpletest::testfivealive);
    tests.add_test("exceptions", &simpletest::testexceptions);
    tests.add_test("testloopdata", &simpletest::testloopdata);
    tests.add_test("testfloatingpoint", &simpletest::testfloatingpoint);
    return tests.run(argc, argv);
}
