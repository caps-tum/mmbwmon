/*
 * This file is part of fast-lib.
 * Copyright (C) 2017 Jens Breitbart
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_AGENT_MMBWMON_SYSTEM_INFO
#define FAST_LIB_MESSAGE_AGENT_MMBWMON_SYSTEM_INFO

#include <fast-lib/serializable.hpp>

#include <map>
#include <string>
#include <vector>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

/**
 * topic: fast/agent/<hostname>/task/mmbwmon/system_info
 * Payload
 * task: mmbwmon system info
 * threads: <number of threads>
 * smt: <smt factor> (eg 2 on normal Xeon)
 * numa: <number of NUMA domains>
 * bandwidth: <measured memory bandwidth in compact,1 mode> (in GBytes/s)
 */

struct system_info : public fast::Serializable
{
	system_info() = default;
	system_info(const size_t _threads, const size_t _smt, const size_t _numa, const std::vector<double> &_membw);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	size_t threads;
	size_t smt;
	size_t numa;
	std::vector<double> membw;
};

}
}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::mmbwmon::system_info)

#endif
