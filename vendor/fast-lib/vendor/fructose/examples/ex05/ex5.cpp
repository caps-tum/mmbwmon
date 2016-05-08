// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

// This example shows the exception assertions.

#include <stdexcept>
#include <vector>

#include <fructose/fructose.h>

struct my_logic_error : public std::logic_error
{
    my_logic_error(const char* message) : std::logic_error(message) {}
};

struct my_runtime_error : public std::runtime_error
{
    my_runtime_error(const char* message) : std::runtime_error(message) {}
};

struct test : public fructose::test_base<test> 
{
    void array_bounds(const std::string& test_name) 
    {
      std::vector<int> v;
      v.push_back(1234);
      fructose_assert_exception(v.at(2), std::out_of_range);
    }

    void should_catch_std_exceptions(const std::string& test_name)
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
};

int main(int argc, char* argv[]) 
{
  test tests;
  tests.add_test("array_bounds", &test::array_bounds);
  tests.add_test("logic_errors", &test::should_catch_std_exceptions);
  return tests.run(argc, argv);
}
