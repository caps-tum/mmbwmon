/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_MESSAGE_MIGFRA_PCI_ID_HPP
#define FAST_MESSAGE_MIGFRA_PCI_ID_HPP

#include <fast-lib/serializable.hpp>

#include <ostream>
#include <string>

namespace fast {
namespace msg {
namespace migfra {

// Contains the vendor id and device id to identify a PCI device type.
struct PCI_id :
	public fast::Serializable
{
	using vendor_t = unsigned short;
	using device_t = unsigned short;

	PCI_id(vendor_t vendor, device_t device);
	// Default (copy-)constructor and assignment for use in std::vector.
	PCI_id() = default;
	PCI_id(const PCI_id &) = default;
	PCI_id & operator=(const PCI_id &) = default;

	bool operator==(const PCI_id &rhs) const;
	std::string vendor_hex() const;
	std::string device_hex() const;
	std::string str() const;

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;

	vendor_t vendor;
	device_t  device;
};

std::ostream & operator<<(std::ostream &os, const PCI_id &rhs);

}
}
}

YAML_CONVERT_IMPL(fast::msg::migfra::PCI_id)

// std::hash specialization to make PCI_id a valid key for unordered_map.
namespace std
{
	template<> struct hash<fast::msg::migfra::PCI_id>
	{
		typedef fast::msg::migfra::PCI_id argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type &s) const
		{
			const result_type h1(std::hash<argument_type::vendor_t>()(s.vendor));
			const result_type h2(std::hash<argument_type::device_t>()(s.device));
			return h1 ^ (h2 << 1);
		}
	};
}

#endif
