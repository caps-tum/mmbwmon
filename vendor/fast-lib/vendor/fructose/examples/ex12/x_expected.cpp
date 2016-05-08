// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include "x1.h"
#include "x2.h"
#include "x3.h"

#include <stdlib.h>

int main(int argc, char* argv[])
{
    int retval = EXIT_SUCCESS;

    {
        sample_test sample_test_instance;
        sample_test_instance.add_test("test42", &sample_test::test42);
        sample_test_instance.add_test("testbeast", &sample_test::testbeast);
        sample_test_instance.add_test("testfivealive", &sample_test::testfivealive);
        const int r = sample_test_instance.run(argc, argv);
        retval = retval == EXIT_SUCCESS ? r : EXIT_FAILURE;
    }
    {
        misc_tests misc_tests_instance;
        misc_tests_instance.add_test("testexceptions", &misc_tests::testexceptions);
        misc_tests_instance.add_test("testloopdata", &misc_tests::testloopdata);
        misc_tests_instance.add_test("testfloatingpoint", &misc_tests::testfloatingpoint);
        const int r = misc_tests_instance.run(argc, argv);
        retval = retval == EXIT_SUCCESS ? r : EXIT_FAILURE;
    }
    {
        exception_test exception_test_instance;
        exception_test_instance.add_test("test_array_bounds", &exception_test::test_array_bounds);
        exception_test_instance.add_test("test_should_catch_std_exceptions", &exception_test::test_should_catch_std_exceptions);
        const int r = exception_test_instance.run(argc, argv);
        retval = retval == EXIT_SUCCESS ? r : EXIT_FAILURE;
    }
    {
        MyTest MyTest_instance;
        MyTest_instance.add_test("testIt", &MyTest::testIt);
        const int r = MyTest_instance.run(argc, argv);
        retval = retval == EXIT_SUCCESS ? r : EXIT_FAILURE;
    }

    return retval;
}
