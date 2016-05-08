// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include <fructose/fructose.h>

// This tests that when the only kind of assertions are 
// loop assertions, fructose correctly reports that assertions 
// have been made. You have to use the --verbose option to see this working.

struct simpletest : public fructose::test_base<simpletest>
{
    void test_basic_loop(const std::string& test_name)
    {
        fructose_loop_assert(1, true);
    }

    void test_loop1(const std::string& test_name)
    {
        fructose_loop1_assert(1, 1, true);
    }

    void test_loop2(const std::string& test_name)
    {
        fructose_loop2_assert(1, 2, 1, true);
    }

    void test_loop3(const std::string& test_name)
    {
        fructose_loop3_assert(1, 2, 3, 1, true);
    }

    void test_loop4(const std::string& test_name)
    {
        fructose_loop4_assert(1, 2, 3, 4, 1, true);
    }

    void test_loop5(const std::string& test_name)
    {
        fructose_loop5_assert(1, 2, 3, 4, 5, 1, true);
    }

};

int main(int argc, char** argv)
{
    simpletest tests;
    tests.add_test("test_basic", &simpletest::test_basic_loop);
    tests.add_test("test_loop1", &simpletest::test_loop1);
    tests.add_test("test_loop2", &simpletest::test_loop2);
    tests.add_test("test_loop3", &simpletest::test_loop3);
    tests.add_test("test_loop4", &simpletest::test_loop4);
    tests.add_test("test_loop5", &simpletest::test_loop5);
    return tests.run(argc, argv);
}
