/*
 * This file is part of migration-framework.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/message/migfra/pci_id.hpp>

#include <sstream>
#include <iomanip>

namespace fast {
namespace msg {
namespace migfra {

// Converts integer type numbers to string in hex format.
static std::string to_hex_string(unsigned int integer, int digits, bool show_base = true)
{
        std::stringstream ss;
        ss << (show_base ? "0x" : "") << std::hex << std::setfill('0') << std::setw(digits) << +integer;
        return ss.str();
}


//
// PCI_id implementation
//

PCI_id::PCI_id(vendor_t vendor, device_t device) :
	vendor(vendor), device(device)
{
}

bool PCI_id::operator==(const PCI_id &rhs) const
{
	return vendor == rhs.vendor && device == rhs.device;
}

std::string PCI_id::vendor_hex() const
{
	return to_hex_string(vendor, 4);
}

std::string PCI_id::device_hex() const
{
	return to_hex_string(device, 4);
}

std::string PCI_id::str() const
{
	return to_hex_string(vendor, 4, false) + ":" + to_hex_string(device, 4, false);
}

YAML::Node PCI_id::emit() const
{
	YAML::Node node;
	node["vendor"] = vendor_hex();
	node["device"] = device_hex();
	return node;
}

void PCI_id::load(const YAML::Node &node)
{
	vendor = std::stoul(node["vendor"].as<std::string>(), nullptr, 0);
	device = std::stoul(node["device"].as<std::string>(), nullptr, 0);
}

std::ostream & operator<<(std::ostream &os, const PCI_id &rhs)
{
	return os << rhs.str();
}

}
}
}
