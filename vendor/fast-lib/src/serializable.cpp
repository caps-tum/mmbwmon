/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/serializable.hpp>

namespace fast
{
	std::string Serializable::to_string() const
	{
		return "---\n" + YAML::Dump(emit()) + "\n---";
	}
	void Serializable::from_string(const std::string &str)
	{
		load(YAML::Load(str));
	}

	namespace yaml {
	
		void merge_node(YAML::Node &lhs, const YAML::Node &rhs)
		{
			for (const auto &node : rhs) {
				std::string tag = YAML::Dump(node.first);
				if (!lhs[tag]) {
					lhs[tag] = rhs[tag];
				}
			}
		}
	
	}

}
