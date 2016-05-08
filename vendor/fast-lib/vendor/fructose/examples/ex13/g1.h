#ifndef FRUCTOSE_MAIN_TEST_X1_H
#define FRUCTOSE_MAIN_TEST_X1_H

#include "fructose/fructose.h"

const int life_the_available_tests_and_everything = 42;
const int the_neighbour_of_the_beast = 668;
const int is_alive = 6;

FRUCTOSE_CLASS(sample_test)
{
public:
    FRUCTOSE_TEST(test42)
    {
        fructose_assert(life_the_available_tests_and_everything == 6*7);
    }

    FRUCTOSE_TEST(beast)
    {
        fructose_assert(the_neighbour_of_the_beast == 668);
    }

    FRUCTOSE_TEST(fivealive)
    {
        const int five = 5;
        fructose_assert_eq(five, is_alive);
    }
};
#endif
