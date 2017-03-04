/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/message/migfra/ivshmem.hpp>

namespace fast {
namespace msg {
namespace migfra {

Device_ivshmem::Device_ivshmem() :
	size("0"),
	path("path")
{
}

YAML::Node Device_ivshmem::emit() const
{
	YAML::Node node;
	node["id"] = id;
	node["size"] = size;
	fast::yaml::merge_node(node, path.emit());
	return node;
}

void Device_ivshmem::load(const YAML::Node &node)
{
	fast::load(id, node["id"]);
	fast::load(size, node["size"]);
	path.load(node);
}

}
}
}
