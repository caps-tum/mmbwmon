// Copyright (c) 2014 Andrew Peter Marlow.
// All rights reserved.

#include <fructose/fructose.h>

class multiplication {
  double x_, y_;
public:
  multiplication(double x, double y) : x_(x), y_(y) {};
  double times() const {return x_ * y_; };
};

struct timestest : 
  public fructose::test_base<timestest> {
    void loops(const std::string& test_name) {
      static const struct {
	int line_number;
	double x, y, expected;
      } data[] = {
	{ __LINE__, 3, 4, 12}
      , { __LINE__, 5.2, 6.8, 35.36}
      , { __LINE__, -8.1, -9.2, 74.52}
      , { __LINE__, 0.1, 90, 9}
      };
      for (unsigned int i = 0; 
           i < sizeof(data)/sizeof(data[0]); ++i) 
      {
	     multiplication m(data[i].x, data[i].y);
	     double result = m.times();
	     if (verbose()) {
	       std::cout << data[i].x << " * " << data[i].y
			 << " got " << result << " expected " 
			 << data[i].expected  << std::endl;
	     }
	     fructose_loop1_assert(data[i].line_number, i,
				  result == data[i].expected);
      }

    }
};

int main(int argc, char* argv[]) {
  timestest tests;
  tests.add_test("loops", &timestest::loops);
  return tests.run(argc, argv);
}
