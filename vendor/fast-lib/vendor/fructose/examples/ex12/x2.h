// Copyright (c) 2012 Andrew Peter Marlow.
// All rights reserved.

#ifndef FRUCTOSE_MAIN_TEST_X2_H
#define FRUCTOSE_MAIN_TEST_X2_H

#include <stdexcept>
#include <cmath>

#include "fructose/fructose.h"

// These tests are rigged so that some of them fail.
// The tests include exercising floating point comparisons
// and exception handling.

namespace {
    void throw_a_runtime_error()
    {
        throw std::runtime_error("this is a runtime error");
    }
}

class misc_tests : public fructose::test_base<misc_tests>
{
public:
    void testexceptions(const std::string& test_name)
    {
        fructose_assert_exception(throw_a_runtime_error(), std::logic_error);
        fructose_assert_no_exception(throw_a_runtime_error());
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
            fructose_loop1_assert(data[i].line_number, i,
                                 result == data[i].expected_result);
        }
    }

    void testfloatingpoint(const std::string& test_name)
    {
        double my_pi = 3.141592654;
        double calc_pi = 4.0 * atan(1.0);
        fructose_assert_double_eq_rel_abs(my_pi, calc_pi, 0.01, 0.01);
        fructose_assert_double_eq(my_pi, calc_pi);
        fructose_assert_double_ne(my_pi, calc_pi);
        fructose_assert_double_ne_rel_abs(my_pi, calc_pi, 0.01, 0.01);
    }
};
#endif
