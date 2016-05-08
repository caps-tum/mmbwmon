// Copyright (c) 2012 Andrew Peter Marlow.
// All rights reserved.

#ifndef FRUCTOSE_MAIN_TEST_X3_H
#define FRUCTOSE_MAIN_TEST_X3_H

#include <stdexcept>
#include <vector>

#include "fructose/fructose.h"

struct my_logic_error : public std::logic_error
{
    my_logic_error(const char* message) : std::logic_error(message) {}
};

struct my_runtime_error : public std::runtime_error
{
    my_runtime_error(const char* message) : std::runtime_error(message) {}
};

struct exception_test : public fructose::test_base<exception_test> 
{
    void test_array_bounds(const std::string&) 
    {
      std::vector<int> v;
      v.push_back(1234);
      fructose_assert_exception(v.at(2), std::out_of_range);
    }

    void test_should_catch_std_exceptions(const std::string& )
    {
        fructose_assert_exception(throw my_logic_error("a coding error has been detected"), 
                                  std::logic_error);
        fructose_assert_exception(throw my_runtime_error("my runtime error"),
                                  std::runtime_error);
        fructose_assert_exception(throw my_logic_error("another coding error has been detected"), 
                                  std::exception);
        fructose_assert_exception(throw my_runtime_error("my runtime error"),
                                  std::exception);
    }

    //void test_commented_out(const std::string&)
    //{
    //   fructose_assert(false);
    //}
};

// class MyTest : public fructose::test_base<MyTest>
// {
// public:
//    void testIt(const std::string&)
//    {
//       fructose_assert(true);
//    }
//  };

class MyTest : public fructose::test_base<MyTest>
{
public:
   void testIt(const std::string&)
   {
      fructose_assert(true);
   }
};
#endif
