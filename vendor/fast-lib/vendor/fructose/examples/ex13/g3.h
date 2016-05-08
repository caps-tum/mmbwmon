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

FRUCTOSE_CLASS(exception_test)
{
public:
    FRUCTOSE_TEST(array_bounds)
    {
      std::vector<int> v;
      v.push_back(1234);
      fructose_assert_exception(v.at(2), std::out_of_range);
    }

    FRUCTOSE_TEST(should_catch_std_exceptions)
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

    //FRUCTOSE_TEST(commented_out)
    //{
    //   fructose_assert(false);
    //}
};

// FRUCTOSE_CLASS(MyTest)
// {
// public:
//    void testIt(const std::string&)
//    {
//       fructose_assert(true);
//    }
// };

FRUCTOSE_CLASS(MyTest)
{
public:
   void testIt(const std::string&)
   {
      fructose_assert(true);
   }
};
#endif
