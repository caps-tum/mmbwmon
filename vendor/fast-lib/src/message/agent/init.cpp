#include <fast-lib/message/agent/init.hpp>

namespace fast {
namespace msg {
namespace agent {

init::init(std::string hostname) :
	hostname(std::move(hostname))
{
}

YAML::Node init::emit() const
{
	YAML::Node node;
	node["task"] = "init";
	node["hostname"] = hostname;
	return node;
}

void init::load(const YAML::Node &node)
{
	fast::load(hostname, node["hostname"]);
}

}
}
}
