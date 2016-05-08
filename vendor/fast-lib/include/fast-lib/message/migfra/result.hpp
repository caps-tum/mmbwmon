/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_MIGFRA_RESULT_HPP
#define FAST_LIB_MESSAGE_MIGFRA_RESULT_HPP

#include <fast-lib/message/migfra/time_measurement.hpp>
#include <fast-lib/serializable.hpp>

#include <vector>

namespace fast {
namespace msg {
namespace migfra {

/**
 * \brief Represents the result of a Task.
 *
 * Results are sent back packed in a vector representing all results of a Task.
 */
struct Result : public fast::Serializable
{
	Result() = default;
	Result(std::string vm_name, std::string status, std::string details = "");
	Result(std::string vm_name, std::string status, Time_measurement time_measurement, std::string details = "");

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string vm_name;
	std::string status;
	std::string details;
	Time_measurement time_measurement;
};

/**
 * \brief Contains a vector of results and enables proper YAML conversion.
 *
 * A Result_container is neccesary to convert results to YAML in the right format.
 * Represents result of a Task_container.
 */
struct Result_container : public fast::Serializable
{
	Result_container() = default;
	Result_container(const std::string &yaml_str);
	Result_container(std::string title, std::vector<Result> results, std::string id = "");

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string title;
	std::vector<Result> results;
	std::string id;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::migfra::Result)
YAML_CONVERT_IMPL(fast::msg::migfra::Result_container)

#endif
