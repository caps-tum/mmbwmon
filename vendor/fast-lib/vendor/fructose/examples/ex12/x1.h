// Copyright (c) 2012 Andrew Peter Marlow.
// All rights reserved.

#ifndef FRUCTOSE_MAIN_TEST_X1_H
#define FRUCTOSE_MAIN_TEST_X1_H

#include "fructose/fructose.h"

const int life_the_available_tests_and_everything = 42;
const int the_neighbour_of_the_beast = 668;
const int is_alive = 6;

struct sample_test : public fructose::test_base<sample_test>
{
    void test42(const std::string& test_name)
    {
        fructose_assert(life_the_available_tests_and_everything == 6*7);
    }

    void testbeast(const std::string& test_name)
    {
        fructose_assert(the_neighbour_of_the_beast == 668);
    }

    void testfivealive(const std::string& test_name)
    {
        const int five = 5;
        fructose_assert_eq(five, is_alive);
    }
};
#endif
