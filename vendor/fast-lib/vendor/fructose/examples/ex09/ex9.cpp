// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

/*
This example is the start of an effort to provide assertThat
with Hamcrest style matchers.
 */

#include <fructose/fructose.h>

#include <stdexcept>

struct assert_that_test : public fructose::test_base<assert_that_test>
{
    void simple_test(const std::string& test_name)
    {
      // This is not ready yet, it is just a placeholder.
      //        fructose_assert_that("hello", true, fructose_is(false));
      //        fructose_assert_that("hello", true, fructose_isEqual(false));
      //        fructose_assert_that("hello", true, fructose_isEqualTo(false));
    }

};

int main(int argc, char** argv)
{
    assert_that_test tests;
    tests.add_test("assertthat", &assert_that_test::simple_test);
    return tests.run(argc, argv);
}
