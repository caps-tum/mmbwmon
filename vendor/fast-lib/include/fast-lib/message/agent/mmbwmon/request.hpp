/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_AGENT_MMBWMON_REQUEST
#define FAST_LIB_MESSAGE_AGENT_MMBWMON_REQUEST

#include <fast-lib/serializable.hpp>

#include <map>
#include <string>
#include <vector>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

/**
 * topic: fast/agent/<hostname>/task/mmbwmon/reply
 * Payload
 * task: mmbwmon request
 * cores: <list of cores>
 */

struct request : public fast::Serializable
{
	request() = default;
	request(const std::vector<std::size_t> &_cores);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::vector<size_t> cores;
};

}
}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::mmbwmon::request)

#endif
