#include <fructose/fructose.h>

struct Template_tester :
	public fructose::test_base<Template_tester>
{
	Template_tester()
	{
		// Setup ...
	}

	~Template_tester()
	{
		// Teardown ...
	}


	void test1(const std::string &test_name)
	{
		// Test ...
		fructose_assert(!false);
		fructose_assert_eq(42, true);
		fructose_assert_ne(true, false);
		fructose_assert_no_exception(6*7);
	}
};

int main(int argc, char **argv)
{
	Message_tester tests;
	tests.add_test("test1", &Template_tester::test1);
	return tests.run(argc, argv);
}
