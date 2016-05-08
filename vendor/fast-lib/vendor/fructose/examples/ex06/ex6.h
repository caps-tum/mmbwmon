// Copyright (c) 2012 Andrew Peter Marlow.
// All rights reserved.

#include "fructose/test_base.h"
#include <cmath>

// This test example is like ex3 except that it uses macros that
// work in combination with the test harness generator.
// With this way of working, the test class is in a header file
// (this one) and function main which includes this header is
// automatically generated.
// This provides a much less error-prone way to ensure that
// all the tests defined in the header added to the test suite
// by FRUCTOSE.

FRUCTOSE_CLASS(simpletest)
{
public:
    FRUCTOSE_TEST(floating)
    {
      double mypi = 4.0 * std::atan(1.0);
      fructose_assert_double_eq(M_PI, mypi);
    }
};

