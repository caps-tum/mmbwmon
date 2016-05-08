/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_AGENT_STOP_MONITOR_HPP
#define FAST_LIB_MESSAGE_AGENT_STOP_MONITOR_HPP

#include <fast-lib/serializable.hpp>

#include <map>
#include <string>

namespace fast {
namespace msg {
namespace agent {

/**
 * topic: fast/agent/<hostname>/task
 * Payload
 *  task: stop monitoring
 *  job-description:
 *    job-id: <job id>
 *    process-id: <process id of the vm>
 */

struct job_description : public fast::Serializable
{
	job_description() = default;
	job_description(std::string job_id, unsigned int process_id);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string job_id;
	unsigned int process_id;
};

struct stop_monitoring : public fast::Serializable
{
	stop_monitoring() = default;
	stop_monitoring(std::string job_id, unsigned int process_id);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	job_description job_desc;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::job_description)
YAML_CONVERT_IMPL(fast::msg::agent::stop_monitoring)

#endif
