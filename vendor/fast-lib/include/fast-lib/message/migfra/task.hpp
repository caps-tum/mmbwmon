/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_MIGFRA_TASK_HPP
#define FAST_LIB_MESSAGE_MIGFRA_TASK_HPP

#include <fast-lib/message/migfra/ivshmem.hpp>
#include <fast-lib/message/migfra/pci_id.hpp>
#include <fast-lib/message/migfra/time_measurement.hpp>
#include <fast-lib/optional.hpp>
#include <fast-lib/serializable.hpp>

#include <memory>
#include <string>
#include <vector>

namespace fast {
namespace msg {
namespace migfra {

/**
 * \brief An abstract struct to provide an interface for a Task.
 */
struct Task :
	public fast::Serializable
{
	Task();
	/**
	 * \brief Constructor for Task.
	 *
	 * \param concurrent_execution Execute Task in dedicated thread.
	 * \param time_measurement Measure execution time and send in Result.
	 */
	Task(bool concurrent_execution, bool time_measurement = false);
	virtual ~Task(){}

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	Optional<bool> concurrent_execution;
	Optional<bool> time_measurement;
	Optional<std::string> driver;
};

/**
 * \brief A struct containing tasks.
 *
 * Contains several Tasks and executes those.
 * Task_handler will call execute method to execute the task.
 */
struct Task_container :
	public fast::Serializable
{
	struct no_task_exception :
		public std::runtime_error
	{
		no_task_exception(const std::string &str);
	};

	/**
	 * \brief Generate trivial default constructor.
	 *
	 * Constructs a Task_container without tasks.
	 * The execute method will return immediatly on a such constructed Task_container.
	 */
	Task_container();
	/**
	 * \brief Constructor for Task_container.
	 *
	 * \param tasks The tasks to execute.
	 * \param concurrent_execution Create and wait on tasks to be finished in dedicated thread.
	 */
	Task_container(std::vector<std::shared_ptr<Task>> tasks, bool concurrent_execution, std::string id = "");

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::vector<std::shared_ptr<Task>> tasks;
	Optional<bool> concurrent_execution;
	Optional<std::string> id;

	/**
	 * \brief Get readable type of tasks.
	 *
	 * Returned type is the same format as in YAML (task:/result:).
	 * \param enable_result_format Set to true if type should be stored in Result, else Task format is used.
	 */
	std::string type(bool enable_result_format = false) const;
};

/**
 * \brief Task to start a single virtual machine.
 * TODO: vm_name should be optional
 */
struct Start :
	public Task
{
	Start();
	/**
	 * \brief Constructor for Start task.
	 *
	 * \param vm_name The name of the virtual machine to start.
	 * \param vcpus The number of virtual cpus to assign to the virtual machine.
	 * \param memory The ram to assign to the virtual machine in MiB.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 */
	Start(std::string vm_name, unsigned int vcpus, unsigned long memory, std::vector<PCI_id> pci_ids, bool concurrent_execution);
	Start(std::string xml, std::vector<PCI_id> pci_ids, bool concurrent_execution);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	Optional<std::string> vm_name;
	Optional<unsigned int> vcpus;
	Optional<unsigned long> memory;
	Optional<std::vector<std::vector<unsigned int>>> memnode_map;
	std::vector<PCI_id> pci_ids;
	Optional<std::string> xml;
	Optional<Device_ivshmem> ivshmem;
	Optional<bool> transient;
	Optional<std::vector<std::vector<unsigned int>>> vcpu_map;
};

/**
 * \brief Task to stop a single virtual machine.
 */
struct Stop :
	public Task
{
	Stop();
	/**
	 * \brief Constructor for Stop task.
	 *
	 * \param vm_name The name of the virtual machine to stop.
	 * \param force Force stopping the domain by using destroy instead of shutdown.
	 * \param undefine Undefine the domain after stopping.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 */
	Stop(std::string vm_name, bool force, bool undefine, bool concurrent_execution);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	Optional<std::string> vm_name;
	Optional<std::string> regex;
	Optional<bool> force;
	Optional<bool> undefine;
};

/**
 * \brief Used for the swap-with option of the migrate task.
 */
struct Swap_with :
	public fast::Serializable
{
	Swap_with();

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
	Optional<std::string> pscom_hook_procs;
	Optional<std::vector<std::vector<unsigned int>>> vcpu_map;
};


/**
 * \brief Task to migrate a virtual machine.
 */
struct Migrate :
	public Task
{
	Migrate();
	/**
	 * \brief Constructor for Migrate task.
	 *
	 * \param vm_name The name of the virtual machine to migrate.
	 * \param dest_hostname The name of the host to migrate to.
	 * \param migration_type Option to enable live migration.
	 * \param rdma_migration Option to enable rdma migration.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 * \param pscom_hook_procs Number of processes to suspend during migration.
	 * \param time_measurement Measure execution time and send in Result.
	 */
	Migrate(std::string vm_name, std::string dest_hostname, std::string migration_type, bool rdma_migration, bool concurrent_execution, unsigned int pscom_hook_procs, bool time_measurement);
	/**
	 * \brief Constructor for Migrate task.
	 *
	 * \param vm_name The name of the virtual machine to migrate.
	 * \param dest_hostname The name of the host to migrate to.
	 * \param migration_type Option to enable live migration.
	 * \param rdma_migration Option to enable rdma migration.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 * \param pscom_hook_procs Number of processes to suspend during migration as string or "auto" for auto detection.
	 * \param time_measurement Measure execution time and send in Result.
	 */
	Migrate(std::string vm_name, std::string dest_hostname, std::string migration_type, bool rdma_migration, bool concurrent_execution, std::string pscom_hook_procs, bool time_measurement);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
	std::string dest_hostname;
	Optional<std::string> migration_type;
	Optional<bool> rdma_migration;
	Optional<std::string> pscom_hook_procs;
	Optional<std::string> transport;
	Optional<Swap_with> swap_with;
	Optional<std::vector<std::vector<unsigned int>>> vcpu_map;
};

/**
 * \brief Task to repin vcpus of a virtual machine.
 */
struct Repin :
	public Task
{
	Repin();
	Repin(std::string vm_name, std::vector<std::vector<unsigned int>> vcpu_map, bool concurrent_execution = true);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
	std::vector<std::vector<unsigned int>> vcpu_map;
};

/**
 * \brief Task to suspend the execution of a single virtual machine.
 */
struct Suspend :
	public Task
{
	Suspend();
	/**
	 * \brief Constructor for Suspend task.
	 *
	 * \param vm_name The name of the virtual machine to stop.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 */
	Suspend(std::string vm_name, bool concurrent_execution);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
};

/**
 * \brief Task to resume the execution of a single virtual machine.
 */
struct Resume :
	public Task
{
	Resume();
	/**
	 * \brief Constructor for Suspend task.
	 *
	 * \param vm_name The name of the virtual machine to stop.
	 * \param concurrent_execution Execute this Task in dedicated thread.
	 */
	Resume(std::string vm_name, bool concurrent_execution);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
};

/**
 * \brief Task to quit migfra.
 */
struct Quit :
	public Task
{
};

}
}
}

// Adds support for direct yaml conversion, e.g., a vector of these types:
// 	std::vector<T> objs;
// 	fast::load(objs, node["list"]);
YAML_CONVERT_IMPL(fast::msg::migfra::Task)
YAML_CONVERT_IMPL(fast::msg::migfra::Task_container)
YAML_CONVERT_IMPL(fast::msg::migfra::Start)
YAML_CONVERT_IMPL(fast::msg::migfra::Stop)
YAML_CONVERT_IMPL(fast::msg::migfra::Swap_with)
YAML_CONVERT_IMPL(fast::msg::migfra::Migrate)
YAML_CONVERT_IMPL(fast::msg::migfra::Repin)
YAML_CONVERT_IMPL(fast::msg::migfra::Suspend)
YAML_CONVERT_IMPL(fast::msg::migfra::Resume)
YAML_CONVERT_IMPL(fast::msg::migfra::Quit)

#endif
