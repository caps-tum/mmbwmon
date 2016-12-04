#include <fructose/fructose.h>

#include <fast-lib/message/migfra/task.hpp>

using namespace fast::msg::migfra;

struct Task_tester :
	public fructose::test_base<Task_tester>
{
	void task1(const std::string &test_name)
	{
		(void) test_name;
		Task task1;
		task1.concurrent_execution = true;
		task1.time_measurement = false;

		Task task2;
		auto buf = task1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		task2.from_string(buf);
		fructose_assert(task2.concurrent_execution.is_valid());
		fructose_assert(task2.time_measurement.is_valid());
		fructose_assert(task2.concurrent_execution == task1.concurrent_execution);
		fructose_assert(task2.time_measurement == task1.time_measurement);
	}
	void task2(const std::string &test_name)
	{
		(void) test_name;
		Task task1;

		Task task2;
		auto buf = task1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		task2.from_string(buf);
		fructose_assert(!task2.concurrent_execution.is_valid());
		fructose_assert(!task2.time_measurement.is_valid());
	}
	void start1(const std::string &test_name)
	{
		(void) test_name;
		Start start1;
		start1.vm_name = "vm1";
		start1.vcpus = 8;
		start1.memory = 8 * 1024 * 1024;
		start1.pci_ids.emplace_back(0x15b3, 0x1004); // lookup emplace_back params

		Start start2;
		auto buf = start1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		start2.from_string(buf);
		fructose_assert(start2.vm_name.is_valid());
		fructose_assert(start2.vcpus.is_valid());
		fructose_assert(start2.memory.is_valid());
		fructose_assert_eq(start2.pci_ids.size(), 1);
		fructose_assert(!start2.xml.is_valid());
		fructose_assert(start2.vm_name == start1.vm_name);
		fructose_assert(start2.vcpus == start1.vcpus);
		fructose_assert(start2.memory == start1.memory);
		fructose_assert_eq(start2.pci_ids[0], start1.pci_ids[0]);
	}

	void start2(const std::string &test_name)
	{
		(void) test_name;
		Start start1;
		start1.xml = "insert xml here";

		Start start2;
		auto buf = start1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		start2.from_string(buf);
		fructose_assert(!start2.vm_name.is_valid());
		fructose_assert(!start2.vcpus.is_valid());
		fructose_assert(!start2.memory.is_valid());
		fructose_assert_eq(start2.pci_ids.size(), 0);
		fructose_assert(start2.xml.is_valid());
		fructose_assert(start2.xml == start1.xml);
	}

	void stop1(const std::string &test_name)
	{
		(void) test_name;
		Stop stop1;
		stop1.vm_name = "vm1";
		stop1.force = false;
		stop1.undefine = true;

		Stop stop2;
		auto buf = stop1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		stop2.from_string(buf);
		fructose_assert(stop2.force.is_valid());
		fructose_assert(stop2.undefine.is_valid());
		fructose_assert_eq(stop2.vm_name, stop1.vm_name);
		fructose_assert(stop2.force == stop1.force);
		fructose_assert(stop2.undefine == stop1.undefine);
	}

	void stop2(const std::string &test_name)
	{
		(void) test_name;
		Stop stop1;
		stop1.vm_name = "vm1";

		Stop stop2;
		auto buf = stop1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		stop2.from_string(buf);
		fructose_assert(!stop2.force.is_valid());
		fructose_assert_eq(stop2.vm_name, stop1.vm_name);
	}

	void migrate(const std::string &test_name)
	{
		(void) test_name;
		Migrate mig1;
		mig1.vm_name = "vm1";
		mig1.dest_hostname = "server-B";
		mig1.migration_type = "warm";
		mig1.rdma_migration = true;
		mig1.pscom_hook_procs = "2";
		mig1.vcpu_map = {{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3}};

		Migrate mig2;
		auto buf = mig1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		mig2.from_string(buf);
		fructose_assert(mig2.migration_type.is_valid());
		fructose_assert(mig2.rdma_migration.is_valid());
		fructose_assert(mig2.pscom_hook_procs.is_valid());
		fructose_assert(mig2.vcpu_map.is_valid());
		fructose_assert_eq(mig2.vm_name, mig1.vm_name);
		fructose_assert_eq(mig2.dest_hostname, mig1.dest_hostname);
		fructose_assert(mig2.migration_type == mig1.migration_type);
		fructose_assert(mig2.rdma_migration == mig1.rdma_migration);
		fructose_assert(mig2.pscom_hook_procs == mig1.pscom_hook_procs);
		fructose_assert(mig2.vcpu_map == mig1.vcpu_map);
	}

	void repin(const std::string &test_name)
	{
		(void) test_name;
		Repin r1;
		r1.vm_name = "vm1";
		r1.vcpu_map = {{0,1,2,3},{0,1,2,3},{0,1,2,3},{0,1,2,3}};
		Repin r2;
		auto buf = r1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		r2.from_string(buf);
		fructose_assert_eq(r2.vm_name, r1.vm_name);
		fructose_assert(r2.vcpu_map == r1.vcpu_map);
	}

	void suspend(const std::string &test_name)
	{
		(void) test_name;
		Suspend suspend1;
		suspend1.vm_name = "vm1";

		Suspend suspend2;
		auto buf = suspend1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		suspend2.from_string(buf);
		fructose_assert_eq(suspend2.vm_name, suspend1.vm_name);
	}

	void resume(const std::string &test_name)
	{
		(void) test_name;
		Resume resume1;
		resume1.vm_name = "vm1";

		Resume resume2;
		auto buf = resume1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		resume2.from_string(buf);
		fructose_assert_eq(resume2.vm_name, resume1.vm_name);
	}

	void quit(const std::string &test_name)
	{
		(void) test_name;
		std::cout << "Test not implemented." << std::endl;
	}

	void task_cont_start(const std::string &test_name)
	{
		(void) test_name;
		Task_container tc1;
		tc1.id = "42";
		auto first_start = std::make_shared<Start>();
		first_start->xml = "<xml>\n</xml>";
		tc1.tasks.push_back(first_start);
		auto second_start = std::make_shared<Start>();
		second_start->vm_name = "vm1";
		second_start->memory = 2*1024*1024;
		second_start->vcpus = 2;
		second_start->pci_ids.push_back(PCI_id(0x15b3, 0x1004));
		tc1.tasks.push_back(second_start);

		Task_container tc2;
		auto buf = tc1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		tc2.from_string(buf);
		fructose_assert(tc2.type() == "start vm");
	}

	void task_cont_migrate(const std::string &test_name)
	{
		(void) test_name;
		Task_container tc1;
		tc1.id = "42";
		auto mig = std::make_shared<Migrate>();
		mig->vm_name = "vm1";
		mig->dest_hostname = "desthost";
		mig->time_measurement = true;
		mig->migration_type = "live";
		tc1.tasks.push_back(mig);

		Task_container tc2;
		auto buf = tc1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		tc2.from_string(buf);
		fructose_assert(tc2.type() == "migrate vm");
	}

	void task_cont_repin(const std::string &test_name)
	{
		(void) test_name;
		Task_container tc1;
		auto repin = std::make_shared<Repin>();
		repin->vm_name = "vm1";
		repin->vcpu_map = {{0},{1},{2},{3}};
		tc1.tasks.push_back(repin);

		Task_container tc2;
		auto buf = tc1.to_string();
		std::cout << "Serialized string: " << buf << std::endl;
		tc2.from_string(buf);
		fructose_assert(tc2.type() == "repin vm");
	}
};

int main(int argc, char **argv)
{
	Task_tester tests;
	tests.add_test("task1", &Task_tester::task1);
	tests.add_test("task2", &Task_tester::task2);
	tests.add_test("start1", &Task_tester::start1);
	tests.add_test("start2", &Task_tester::start2);
	tests.add_test("stop1", &Task_tester::stop1);
	tests.add_test("stop2", &Task_tester::stop2);
	tests.add_test("migrate", &Task_tester::migrate);
	tests.add_test("repin", &Task_tester::repin);
	tests.add_test("suspend", &Task_tester::suspend);
	tests.add_test("resume", &Task_tester::resume);
	tests.add_test("quit", &Task_tester::quit);
	tests.add_test("task_cont_start", &Task_tester::task_cont_start);
	tests.add_test("task_cont_migrate", &Task_tester::task_cont_migrate);
	tests.add_test("task_cont_repin", &Task_tester::task_cont_repin);
	return tests.run(argc, argv);
}
