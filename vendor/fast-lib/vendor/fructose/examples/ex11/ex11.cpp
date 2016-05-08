// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

// This example shows how fructose integrates with hamcrest matchers.
// By default all the test pass. Use the reverse mode of fructose
// to see the nice messages you get via hamcrest.

#define FRUCTOSE_USING_HAMCREST
#include <fructose/fructose.h>

using namespace hamcrest;

struct features_test : public fructose::test_base<features_test> 
{
    void features(const std::string& test_name) {
      fructose_assert_that(4, equal_to(4));
      fructose_assert_that(10, is(equal_to(10)));
      fructose_assert_that(1, is_not(equal_to(2)));
      fructose_assert_that("good", any_of(equal_to("bad"), 
                                          equal_to("mediocre"), 
                                          equal_to("good"), 
                                          equal_to("excellent")));
      fructose_assert_that("politicians", is_not(any_of(equal_to("decent"), 
                                                        equal_to("honest"), 
                                                        equal_to("truthful"))));
    }

};

int main(int argc, char* argv[])
{
    features_test tests;
    tests.add_test("features", &features_test::features);
    return tests.run(argc, argv);
}
