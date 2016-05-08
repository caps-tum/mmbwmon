// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include <fructose/fructose.h>

// This example exercises a few of the fructose assert macros
// so you can see the output format when the assertion fails.
// It also tests some new features that were added at 1.1.0.

struct tests : public fructose::test_base<tests>
{
    void features1(const std::string& test_name) {
      fructose_assert(1 == 2);
      fructose_assert_eq(1, 2);
      fructose_assert_ne(1, 1);
      fructose_assert_eq("apples", "apples");
      fructose_assert_ne("apples", "oranges");
    }

    void same_data(const std::string& test_name) {
       unsigned char data1[] = { 0xFE, 0x12, 0x54, 0x19, 0xB7,
                                 0xBE, 0xEF, 0xDE, 0xAD, 0xC8 };

       unsigned char data2[] = { 0xFE, 0x12, 0x54, 0x19, 0xB7,
                                 0xBE, 0xEE, 0xDE, 0xAD, 0xC8 };

       size_t len = sizeof(data1)/ sizeof(data1[0]);

       fructose_assert_same_data(data1, data1, len);
       fructose_assert_same_data(data2, data2, len);

       // We expect these tests below to fail.

       fructose_assert_same_data(data1, data2, len);

       // Now try with char data.
       char chars1[] = { 'a', 'b', 'c', 'd', 'e' };
       char chars2[] = { 'a', 'b', 'c', 'g', 'e' };
       fructose_assert_same_data(chars1, chars2, 5);

       // And now with an integer.
       const int int1 = 0x12345678;
       const int int2 = 0x12340478;
       fructose_assert_same_data(&int1, &int2, sizeof(int));
    }
};      
    
int main(int argc, char* argv[])
{     
    tests the_tests;
    the_tests.add_test("features1", &tests::features1);
    the_tests.add_test("same_data", &tests::same_data);
    return the_tests.run(argc, argv);
}   
