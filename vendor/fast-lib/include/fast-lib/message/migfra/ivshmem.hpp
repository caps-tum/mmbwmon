/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_MESSAGE_MIGFRA_IVSHMEM_HPP
#define FAST_MESSAGE_MIGFRA_IVSHMEM_HPP

#include <fast-lib/serializable.hpp>
#include <fast-lib/optional.hpp>

#include <string>

namespace fast {
namespace msg {
namespace migfra {

struct Device_ivshmem :
	public fast::Serializable
{
	Device_ivshmem();

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	std::string id;
	std::string size;
	Optional<std::string> path;
};

}
}
}

YAML_CONVERT_IMPL(fast::msg::migfra::Device_ivshmem)

#endif
