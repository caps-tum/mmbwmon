#include <fast-lib/message/agent/mmbwmon/stop.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

stop::stop(const std::string _cgroup) : cgroup(_cgroup)
{
}

YAML::Node stop::emit() const
{
	YAML::Node node;
	node["task"] = "mmbwmon stop";
    node["cgroup"] = cgroup;
	return node;
}

void stop::load(const YAML::Node &node)
{
	fast::load(cgroup, node["cgroup"]);
}

}
}
}
}
