// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include <fructose/fructose.h>

// This is a very simple set of tests without there even 
// being a class to test. These tests should all pass.
// To see the assertion mechanism working, use the -r flag
// to reverse the sense of the tests.

const int life_the_available_tests_and_everything = 42;
const int the_neighbour_of_the_beast = 668;
const int is_alive = 6;

struct simpletest : public fructose::test_base<simpletest>
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

int main(int argc, char** argv)
{
    simpletest tests;
    tests.add_test("test42", &simpletest::test42);
    tests.add_test("testbeast", &simpletest::testbeast);
    tests.add_test("fiveisalive", &simpletest::testfivealive);
    return tests.run(argc, argv);
}
