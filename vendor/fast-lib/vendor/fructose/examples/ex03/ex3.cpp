// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include <cmath>

#include <fructose/fructose.h>

struct simpletest : 
  public fructose::test_base<simpletest> {
    void floating(const std::string& test_name) {
      double mypi = 4.0 * std::atan(1.0);
      fructose_assert_double_eq(M_PI, mypi);
    }

    void test_that_is_not_ready_yet(const std::string& test_name) {
      fructose_fail("this test is not ready yet");
    }
};

int main(int argc, char* argv[]) {
  simpletest tests;
  tests.add_test("float", &simpletest::floating);
  tests.add_test("not_ready", &simpletest::test_that_is_not_ready_yet);
  return tests.run(argc, argv);
}

