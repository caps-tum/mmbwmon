#include <fructose/fructose.h>

#include <fast-lib/optional.hpp>

using namespace fast;

struct Person :
	Serializable
{
	int id;
	Person(int id = -1) :
		id(id)
	{
	}
	YAML::Node emit() const override
	{
		YAML::Node node;
		node["id"] = id;
		return node;
	}
	void load(const YAML::Node &node) override
	{
		fast::load(id, node[id]);
	}
};
YAML_CONVERT_IMPL(Person)

struct Task_tester :
	public fructose::test_base<Task_tester>
{
	void optional_bool(const std::string &test_name)
	{
		(void) test_name;
		const std::string tag = "b";
		Optional<bool> b(tag);
		fructose_assert(!b);
		fructose_assert(!b.is_valid());
		fructose_assert_eq(tag, b.get_tag());
		fructose_assert_exception(b.get(), std::runtime_error);
		fructose_assert_exception(*b, std::runtime_error);
		b = false;
		fructose_assert(b);
		fructose_assert(b.is_valid());
		fructose_assert(*b == false);
		fructose_assert(b.get() == false);
		fructose_assert_no_exception(b.get());
		fructose_assert_no_exception(*b);
	}

	void optional_struct(const std::string &test_name)
	{
		(void) test_name;
			
		const std::string tag = "person";
		Optional<Person> p(tag);
		fructose_assert(!p);
		fructose_assert(!p.is_valid());
		fructose_assert_eq(tag, p.get_tag());
		fructose_assert_exception(p.get(), std::runtime_error);
		fructose_assert_exception(*p, std::runtime_error);
		fructose_assert_exception((void) p->id, std::runtime_error);
		p = Person(1);
		fructose_assert(p);
		fructose_assert(p.is_valid());
		fructose_assert_no_exception(p.get());
		fructose_assert_no_exception(*p);
		fructose_assert_no_exception((void) (*p).id);
		fructose_assert_no_exception((void) p.get().id);
		fructose_assert_no_exception((void) p->id);
		fructose_assert((*p).id == 1);
		fructose_assert(p.get().id == 1);
		fructose_assert(p->id == 1);
	}
};

int main(int argc, char **argv)
{
	Task_tester tests;
	tests.add_test("optional-bool", &Task_tester::optional_bool);
	tests.add_test("optional-struct", &Task_tester::optional_struct);
	return tests.run(argc, argv);
}
