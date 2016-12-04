/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/message/migfra/task.hpp>

#include <iostream>

// Alias merge_node function
const auto &merge_node = fast::yaml::merge_node;

namespace fast {
namespace msg {
namespace migfra {

Task::Task() :
	concurrent_execution("concurrent-execution"),
	time_measurement("time-measurement"),
	driver("driver")
{
}

Task::Task(bool concurrent_execution, bool time_measurement) :
	concurrent_execution("concurrent-execution", concurrent_execution),
	time_measurement("time-measurement", time_measurement),
	driver("driver")
{
}

YAML::Node Task::emit() const
{
	YAML::Node node;
	merge_node(node, concurrent_execution.emit());
	merge_node(node, time_measurement.emit());
	merge_node(node, driver.emit());
	return node;
}

void Task::load(const YAML::Node &node)
{
	concurrent_execution.load(node);
	time_measurement.load(node);
	driver.load(node);
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
	std::array<std::string, 7> types;
	if (enable_result_format)
		types = {{"vm started", "vm stopped", "vm migrated", "vm repinned", "vm suspended", "vm resumed", "quit"}};
	else
		types = {{"start vm", "stop vm", "migrate vm", "repin vm", "suspend vm", "resume vm", "quit"}};
	if (tasks.empty())
		throw std::runtime_error("No subtasks available to get type.");
	else if (std::dynamic_pointer_cast<Start>(tasks.front()))
		return types[0];
	else if (std::dynamic_pointer_cast<Stop>(tasks.front()))
		return types[1];
	else if (std::dynamic_pointer_cast<Migrate>(tasks.front()))
		return types[2];
	else if (std::dynamic_pointer_cast<Repin>(tasks.front()))
		return types[3];
	else if (std::dynamic_pointer_cast<Suspend>(tasks.front()))
		return types[4];
	else if (std::dynamic_pointer_cast<Resume>(tasks.front()))
		return types[5];
	else if (std::dynamic_pointer_cast<Quit>(tasks.front()))
		return types[6];
	else
		throw std::runtime_error("Unknown type of Task.");

}


YAML::Node Task_container::emit() const
{
	YAML::Node node;
	auto type_str = type();
	node["task"] = type_str;
	if (type_str == "migrate vm") {
		merge_node(node, tasks.front()->emit());
	} else if (type_str == "repin vm") {
		merge_node(node, tasks.front()->emit());
	} else {
		node["vm-configurations"] = tasks;
	}
	merge_node(node, concurrent_execution.emit());
	merge_node(node, id.emit());
	return node;
}

std::vector<std::shared_ptr<Task>> load_start_task(const YAML::Node &node)
{
	std::vector<std::shared_ptr<Start>> tasks;
	fast::load(tasks, node["vm-configurations"]);
	return std::vector<std::shared_ptr<Task>>(tasks.begin(), tasks.end());
}

std::vector<std::shared_ptr<Task>> load_stop_task(const YAML::Node &node)
{
	std::vector<std::shared_ptr<Stop>> tasks;
	fast::load(tasks, node["list"]);
	return std::vector<std::shared_ptr<Task>>(tasks.begin(), tasks.end());
}

std::vector<std::shared_ptr<Task>> load_migrate_task(const YAML::Node &node)
{
	std::shared_ptr<Migrate> migrate_task;
	fast::load(migrate_task, node);
	return std::vector<std::shared_ptr<Task>>(1, migrate_task);
}

std::vector<std::shared_ptr<Task>> load_repin_task(const YAML::Node &node)
{
	std::shared_ptr<Repin> repin_task;
	fast::load(repin_task, node);
	return std::vector<std::shared_ptr<Task>>(1, repin_task);
}

std::vector<std::shared_ptr<Task>> load_suspend_task(const YAML::Node &node)
{
	std::vector<std::shared_ptr<Suspend>> tasks;
	fast::load(tasks, node["list"]);
	return std::vector<std::shared_ptr<Task>>(tasks.begin(), tasks.end());
}

std::vector<std::shared_ptr<Task>> load_resume_task(const YAML::Node &node)
{
	std::vector<std::shared_ptr<Resume>> tasks;
	fast::load(tasks, node["list"]);
	return std::vector<std::shared_ptr<Task>>(tasks.begin(), tasks.end());
}

std::vector<std::shared_ptr<Task>> load_quit_task(const YAML::Node &node)
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
		tasks = load_start_task(node);
	} else if (type == "stop vm") {
		tasks = load_stop_task(node);
	} else if (type == "migrate vm") {
		tasks = load_migrate_task(node);
	} else if (type == "repin vm") {
		tasks = load_repin_task(node);
	} else if (type == "suspend vm") {
		tasks = load_suspend_task(node);
	} else if (type == "resume vm") {
		tasks = load_resume_task(node);
	} else if (type == "quit") {
		tasks = load_quit_task(node);
	} else {
		throw std::runtime_error("Unknown type of Task while loading.");
	}
	concurrent_execution.load(node);
	id.load(node);
}

Start::Start() :
	vm_name("vm-name"),
	vcpus("vcpus"),
	memory("memory"),
	xml("xml")
{
}

Start::Start(std::string vm_name, unsigned int vcpus, unsigned long memory, std::vector<PCI_id> pci_ids, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name("vm-name", std::move(vm_name)),
	vcpus("vcpus", vcpus),
	memory("memory", memory),
	pci_ids(std::move(pci_ids)),
	xml("xml")
{
}

Start::Start(std::string xml, std::vector<PCI_id> pci_ids, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name("vm-name"),
	vcpus("vcpus"),
	memory("memory"),
	pci_ids(std::move(pci_ids)),
	xml("xml", xml)
{
}

YAML::Node Start::emit() const
{
	YAML::Node node = Task::emit();
	merge_node(node, vm_name.emit());
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
	vm_name.load(node);
	vcpus.load(node);
	memory.load(node);
	fast::load(pci_ids, node["pci-ids"], std::vector<PCI_id>());
	xml.load(node);
}

Stop::Stop() :
	force("force"),
	undefine("undefine")
{
}

Stop::Stop(std::string vm_name, bool force, bool undefine, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name(std::move(vm_name)),
	force("force", force),
	undefine("undefine", undefine)
{
}

YAML::Node Stop::emit() const
{
	YAML::Node node = Task::emit();
	node["vm-name"] = vm_name;
	merge_node(node, force.emit());
	merge_node(node, undefine.emit());
	return node;
}

void Stop::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(vm_name, node["vm-name"]);
	force.load(node);
	undefine.load(node);
}

Migrate::Migrate() :
	migration_type("migration-type"),
	rdma_migration("rdma-migration"),
	pscom_hook_procs("pscom-hook-procs"),
	transport("transport"),
	swap_with("swap-with"),
	vcpu_map("vcpu-map")
{
}

Migrate::Migrate(std::string vm_name, std::string dest_hostname, std::string migration_type, bool rdma_migration, bool concurrent_execution, unsigned int pscom_hook_procs, bool time_measurement) :
	Task::Task(concurrent_execution, time_measurement),
	vm_name(std::move(vm_name)),
	dest_hostname(std::move(dest_hostname)),
	migration_type("migration-type", std::move(migration_type)),
	rdma_migration("rdma-migration", rdma_migration),
	pscom_hook_procs("pscom-hook-procs", std::to_string(pscom_hook_procs)),
	transport("transport"),
	swap_with("swap-with"),
	vcpu_map("vcpu-map")
{
}

Migrate::Migrate(std::string vm_name, std::string dest_hostname, std::string migration_type, bool rdma_migration, bool concurrent_execution, std::string pscom_hook_procs, bool time_measurement) :
	Task::Task(concurrent_execution, time_measurement),
	vm_name(std::move(vm_name)),
	dest_hostname(std::move(dest_hostname)),
	migration_type("migration-type", std::move(migration_type)),
	rdma_migration("rdma-migration", rdma_migration),
	pscom_hook_procs("pscom-hook-procs", std::move(pscom_hook_procs)),
	transport("transport"),
	swap_with("swap-with"),
	vcpu_map("vcpu-map")
{
}

YAML::Node Migrate::emit() const
{
	YAML::Node node = Task::emit();
	node["vm-name"] = vm_name;
	node["destination"] = dest_hostname;
	YAML::Node params = node["parameter"];
	merge_node(params, migration_type.emit());
	merge_node(params, rdma_migration.emit());
	merge_node(params, pscom_hook_procs.emit());
	merge_node(params, transport.emit());
	merge_node(params, swap_with.emit());
	merge_node(params, vcpu_map.emit());
	if (vcpu_map.is_valid())
		params[vcpu_map.get_tag()].SetStyle(YAML::EmitterStyle::Flow);
	return node;
}

void Migrate::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(vm_name, node["vm-name"]);
	fast::load(dest_hostname, node["destination"]);
	if (node["parameter"]) {
		migration_type.load(node["parameter"]);
		rdma_migration.load(node["parameter"]);
		pscom_hook_procs.load(node["parameter"]);
		transport.load(node["parameter"]);
		swap_with.load(node["parameter"]);
		vcpu_map.load(node["parameter"]);
	}
}

Repin::Repin()
{
}

Repin::Repin(std::string vm_name, std::vector<std::vector<unsigned int>> vcpu_map, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name(std::move(vm_name)),
	vcpu_map(std::move(vcpu_map))
{
}

YAML::Node Repin::emit() const
{
	YAML::Node node = Task::emit();
	node["vm-name"] = vm_name;
	node["vcpu-map"] = vcpu_map;
	node["vcpu-map"].SetStyle(YAML::EmitterStyle::Flow);
	return node;
}

void Repin::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(vm_name, node["vm-name"]);
	fast::load(vcpu_map, node["vcpu-map"]);
}

Suspend::Suspend()
{
}

Suspend::Suspend(std::string vm_name, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name(std::move(vm_name))
{
}

YAML::Node Suspend::emit() const
{
	YAML::Node node = Task::emit();
	node["vm-name"] = vm_name;
	return node;
}

void Suspend::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(vm_name, node["vm-name"]);
}

Resume::Resume()
{
}

Resume::Resume(std::string vm_name, bool concurrent_execution) :
	Task::Task(concurrent_execution),
	vm_name(std::move(vm_name))
{
}

YAML::Node Resume::emit() const
{
	YAML::Node node = Task::emit();
	node["vm-name"] = vm_name;
	return node;
}

void Resume::load(const YAML::Node &node)
{
	Task::load(node);
	fast::load(vm_name, node["vm-name"]);
}

}
}
}
