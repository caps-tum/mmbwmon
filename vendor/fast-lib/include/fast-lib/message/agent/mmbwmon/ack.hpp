/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_AGENT_MMBWMON_ACK
#define FAST_LIB_MESSAGE_AGENT_MMBWMON_ACK

#include <fast-lib/serializable.hpp>

#include <map>
#include <string>
#include <vector>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

/**
 * topic: various
 * Payload
 * task: ack
 */

struct ack : public fast::Serializable
{
	ack() = default;

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;
};

}
}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::mmbwmon::ack)

#endif
