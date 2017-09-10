#include <fast-lib/message/agent/mmbwmon/system_info.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

system_info::system_info(const size_t _threads, const size_t _smt, const size_t _numa, const std::vector<double> &_membw): threads(_threads), smt(_smt), numa(_numa), membw(_membw)
{
}

YAML::Node system_info::emit() const
{
	YAML::Node node;
	node["threads"] = threads;
	node["smt"] = smt;
	node["numa"] = numa;
	node["bandwidth"] = membw;
	return node;
}

void system_info::load(const YAML::Node &node)
{
	fast::load(threads, node["threads"]);
	fast::load(smt, node["smt"]);
	fast::load(numa, node["numa"]);
	fast::load(membw, node["bandwidth"]);
}

}
}
}
}
