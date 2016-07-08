#include <fast-lib/message/agent/mmbwmon/restart.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

restart::restart(const std::string _cgroup) : cgroup(_cgroup)
{
}

YAML::Node restart::emit() const
{
	YAML::Node node;
	node["task"] = "mmbwmon restart";
    node["cgroup"] = cgroup;
	return node;
}

void restart::load(const YAML::Node &node)
{
	fast::load(cgroup, node["cgroup"]);
}

}
}
}
}
