/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_AGENT_MMBWMON_RESTART
#define FAST_LIB_MESSAGE_AGENT_MMBWMON_RESTART

#include <fast-lib/serializable.hpp>

#include <map>
#include <string>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

/**
 * topic: fast/agent/<hostname>/task/mmbwmon/restart
 * Payload
 * task: mmbwmon restart
 * cgroup: cgroup_name
 */

struct restart : public fast::Serializable
{
	restart() = default;
	restart(const std::string _cgroup);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

    std::string cgroup;
};

}
}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::mmbwmon::restart)

#endif
