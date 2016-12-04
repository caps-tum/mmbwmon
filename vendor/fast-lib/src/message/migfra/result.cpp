/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/message/migfra/result.hpp>

namespace fast {
namespace msg {
namespace migfra {

Result_container::Result_container(const std::string &yaml_str)
{
	from_string(yaml_str);
}

Result_container::Result_container(std::string title, std::vector<Result> results, std::string id) :
	title(std::move(title)),
	results(std::move(results)),
	id(std::move(id))
{
}

YAML::Node Result_container::emit() const
{
	YAML::Node node;
	node["result"] = title;
	if (title == "vm migrated")
		fast::yaml::merge_node(node, results.at(0).emit());
	else
		node["list"] = results;
	if (id != "")
		node["id"] = id;
	return node;
}

void Result_container::load(const YAML::Node &node)
{
	fast::load(title, node["result"]);
	if (title == "vm migrated") {
		results.emplace_back();
		fast::load(results[0], node);
	} else {
		fast::load(results, node["list"]);
	}
	fast::load(id, node["id"], "");
}

Result::Result(std::string vm_name, std::string status, std::string details) :
	vm_name(std::move(vm_name)),
	status(std::move(status)),
	details(std::move(details))
{
}

Result::Result(std::string vm_name, std::string status, Time_measurement time_measurement, std::string details) :
	vm_name(std::move(vm_name)),
	status(std::move(status)),
	details(std::move(details)),
	time_measurement(std::move(time_measurement))
{
}

YAML::Node Result::emit() const
{
	YAML::Node node;
	node["vm-name"] = vm_name;
	node["status"] = status;
	if (details != "")
		node["details"] = details;
	if (!time_measurement.empty())
		node["time-measurement"] = time_measurement;
	return node;
}

void Result::load(const YAML::Node &node)
{
	fast::load(vm_name, node["vm-name"]);
	fast::load(status, node["status"]);
	fast::load(details, node["details"], "");
}

}
}
}
