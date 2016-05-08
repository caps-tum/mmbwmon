// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

/*
This example is to show that when a test method throws
an exception, it is caught and handled properly. The harness
is meant to fail.
 */

#include <stdexcept>
#include <cmath>

#include <fructose/fructose.h>

struct exceptiontest : public fructose::test_base<exceptiontest>
{
    void test_that_exception_causes_failure(const std::string& test_name)
    {
        throw std::runtime_error("this is a runtime error");

    }

    void test_that_another_exception_causes_failure(const std::string& test_name)
    {
        throw std::runtime_error("this is a another runtime error");
    }

};

int main(int argc, char** argv)
{
    exceptiontest tests;
    tests.add_test("exceptions", &exceptiontest::test_that_exception_causes_failure);
    tests.add_test("exceptions2", &exceptiontest::test_that_another_exception_causes_failure);
    return tests.run(argc, argv);
}
