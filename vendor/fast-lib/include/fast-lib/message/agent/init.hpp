/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MESSAGE_INIT_HPP
#define FAST_LIB_MESSAGE_INIT_HPP

#include <fast-lib/serializable.hpp>

namespace fast {
namespace msg {
namespace agent {

/**
 * * topic: fast/agent/<hostname>/status
 * * payload:
 *   task: init
 *   source: <hostname>
 */
struct init : public fast::Serializable
{
	init() = default;
	init(std::string hostname);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string hostname;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::agent::init)

#endif
