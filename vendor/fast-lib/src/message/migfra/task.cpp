/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/message/migfra/task.hpp>
#include <iostream>

namespace fast {
namespace msg {
namespace migfra {

// merges rhs into lhs
void merge_node(YAML::Node &lhs, const YAML::Node &rhs)
{
	for (const auto &node : rhs) {
		std::string tag = YAML::Dump(node.first);
		if (!lhs[tag]) {
			lhs[tag] = rhs[tag];
		}
	}
}

Task::Task() :
	concurrent_execution("concurrent-execution"),
	time_measurement("time-measurement")
{
}

Task::Task(std::string vm_name, bool concurrent_execution, bool time_measurement) :
	vm_name(std::move(vm_name)),
	concurrent_execution("concurrent-execution", concurrent_execution),
	time_measurement("time-measurement", time_measurement)
{
}

YAML::Node Task::emit() const
{
	YAML::Node node;
	node["vm-name"] = vm_name;
	merge_node(node, concurrent_execution.emit());
	merge_node(node, time_measurement.emit());
	return node;
}

void Task::load(const YAML::Node &node)
{
	fast::load(vm_name, node["vm-name"]);
	concurrent_execution.load(node);
	time_measurement.load(node);
}

Task_container::Task_container() :
	concurrent_execution("concurrent-execution"),
	id("id")
{
}

Task_container::Task_container(std::vector<std::shared_ptr<Task>> tasks, bool concurrent_execution, std::string id) :
	tasks(std::move(tasks)),
	concurrent_execution("concurrent-execution", concurrent_execution),
	id("id", std::move(id))
{
}

std::string Task_container::type(bool enable_result_format) const
{
	std::array<std::string, 4> types;
	if (enable_result_format)
		types = {{"vm started", "vm stopped", "vm migrated", "quit"}};
	else
		types = {{"start vm", "stop vm", "migrate vm", "quit"}};
	if (tasks.empty())
		throw std::runtime_error("No subtasks available to get type.");
	else if (std::dynamic_pointer_cast<Start>(tasks.front()))
		return types[0];
	else if (std::dynamic_pointer_cast<Stop>(tasks.front()))
		return types[1];
	else if (std::dynamic_pointer_cast<Migrate>(tasks.front()))
		return types[2];
	else if (std::dynamic_pointer_cast<Quit>(tasks.front()))
		return types[3];
	else
		throw std::runtime_error("Unknown type of Task.");

}


YAML::Node Task_container::emit() const
{
	YAML::Node node;
	node["task"] = type();
	node["vm-configurations"] = tasks;
	merge_node(node, concurrent_execution.emit());
	merge_node(node, id.emit());
	return node;
}

template<class T> std::vector<std::shared_ptr<Task>> load_tasks(const YAML::Node &node)
{
	std::vector<std::shared_ptr<T>> tasks;
	fast::load(tasks, node["vm-configurations"]);
	return std::vector<std::shared_ptr<Task>>(tasks.begin(), tasks.end());
}

// Specialization for Migrate due to different yaml structure (no "vm-configurations")
template<> std::vector<std::shared_ptr<Task>> load_tasks<Migrate>(const YAML::Node &node)
{
	std::shared_ptr<Migrate> migrate_task;
	fast::load(migrate_task, node);
	return std::vector<std::shared_ptr<Task>>(1, migrate_task);
}

// Specialization for Quit due to different yaml structure (no "vm-configurations")
template<> std::vector<std::shared_ptr<Task>> load_tasks<Quit>(const YAML::Node &node)
{
	std::shared_ptr<Quit> quit_task;
	fast::load(quit_task, node);
	return std::vector<std::shared_ptr<Task>>(1, quit_task);
}

void Task_container::load(const YAML::Node &node)
{
	std::string type;
	try {
		fast::load(type, node["task"]);
	} catch (const std::exception &e) {
		throw Task_container::no_task_exception("Cannot find key \"task\" to load Task from YAML.");
	} 
	if (type == "start vm") {
		tasks = load_tasks<Start>(node);
	} else if (type == "stop vm") {
		tasks = load_tasks<Stop>(node);
	} else if (type == "migrate vm") {
		tasks = load_tasks<Migrate>(node);
	} else if (type == "quit") {
		tasks = load_tasks<Quit>(node);
	} else {
		throw std::runtime_error("Unknown type of Task while loading.");
	}
	concurrent_execution.load(node);
	id.load(node);
}

Start::Start() :
	vcpus("vcpus"),
	memory("memory"),
	xml("xml")
{
}

Start::Start(std::string vm_name, unsigned int vcpus, unsigned long memory, std::vector<PCI_id> pci_ids, bool concurrent_execution) :
	Task::Task(std::move(vm_name), concurrent_execution),
	vcpus("vcpus", vcpus),
	memory("memory", memory),
	pci_ids(std::move(pci_ids)),
	xml("xml")
{
}

/// TODO: Get vm_name from xml.
Start::Start(std::string vm_name, std::string xml, std::vector<PCI_id> pci_ids, bool concurrent_execution) :
	Task::Task(std::move(vm_name), concurrent_execution),
	vcpus("vcpus"),
	memory("memory"),
	pci_ids(std::move(pci_ids)),
	xml("xml", xml)
{
}

YAML::Node Start::emit() const
{
	YAML::Node node = Task::emit();
	merge_node(node, vcpus.emit());
	merge_node(node, memory.emit());
	merge_node(node, xml.emit());
	if (!pci_ids.empty())
		node["pci-ids"] = pci_ids;
	return node;
}

void Start::load(const YAML::Node &node)
{
	Task::load(node);
	vcpus.load(node);
	memory.load(node);
	fast::load(pci_ids, node["pci-ids"], std::vector<PCI_id>());
	xml.load(node);
}

Stop::Stop() :
	force("force")
{
}

Stop::Stop(std::string vm_name, bool force, bool concurrent_execution) :
	Task::Task(std::move(vm_name), concurrent_execution),
	force("force", force)
{
}

YAML::Node Stop::emit() const
{
	YAML::Node node = Task::emit();
	merge_node(node, force.emit());
	return node;
}

void Stop::load(const YAML::Node &node)
{
	Task::load(node);
	force.load(node);
}

Migrate::Migrate() :
	live_migration("live-migration"),
	rdma_migration("rdma-migration"),
	pscom_hook_procs("pscom-hook-procs")
{
}

Migrate::Migrate(std::string vm_name, std::string dest_hostname, bool live_migration, bool rdma_migration, bool concurrent_execution, unsigned int pscom_hook_procs, bool time_measurement) :
	Task::Task(std::move(vm_name), concurrent_execution, time_measurement),
	dest_hostname(std::move(dest_hostname)),
	live_migration("live-migration", live_migration),
	rdma_migration("rdma-migration", rdma_migration),
	pscom_hook_procs("pscom-hook-procs", pscom_hook_procs)
{
}

YAML::Node Migrate::emit() const
{
	YAML::Node node = Task::emit();
	node["destination"] = dest_hostname;
	YAML::Node params = node["parameter"];
	merge_node(params, live_migration.emit());
	merge_node(params, rdma_migration.emit());
	merge_node(params, pscom_hook_procs.emit());
	return node;
}

void Migrate::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(dest_hostname, node["destination"]);
	if (node["parameter"]) {
		live_migration.load(node["parameter"]);
		rdma_migration.load(node["parameter"]);
		pscom_hook_procs.load(node["parameter"]);
	}
}

}
}
}
