// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

// This example include some features new to version 0.8.0

#include <fructose/fructose.h>

struct features_test : public fructose::test_base<features_test> 
{
    void features(const std::string& test_name) {
        fructose_assert_ne(4, 4);
    }

};

int main(int argc, char* argv[])
{
    features_test tests;
    tests.add_test("features", &features_test::features);
    return tests.run(argc, argv);
}
