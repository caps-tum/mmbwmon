// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

// This example shows how fructose tests can pick up command
// line arguments.
// There are two tests so that we can show that different tests
// can receive different command line arguments.
//
#include <fructose/fructose.h>

struct tester : public fructose::test_base<tester> 
{
  void dump_args() const
  {
      std::vector<std::string> args = get_args();
      std::cout << "There are " << args.size() << " arguments." << std::endl;
      std::vector<std::string>::const_iterator it = args.begin();
      for (; it != args.end(); ++it)
      {
        std::cout << "   " << *it << std::endl;
      }
  }

  void basic1(const std::string& test_name) {
    dump_args();
  }

  void basic2(const std::string& test_name) {
    dump_args();
  }

};

int main(int argc, char* argv[])
{
    tester tests;
    tests.add_test("basic1", &tester::basic1);
    tests.add_test("basic2", &tester::basic2);
    return tests.run(argc, argv);
}
