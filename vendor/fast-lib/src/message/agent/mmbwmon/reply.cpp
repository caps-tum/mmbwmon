#include <fast-lib/message/agent/mmbwmon/reply.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

reply::reply(const std::vector<size_t> &_cores, double _result) : cores(_cores), result(_result)
{
}

YAML::Node reply::emit() const
{
	YAML::Node node;
	node["task"] = "mmbwmon response";
	node["cores"] = cores;
    node["result"] = result;
	return node;
}

void reply::load(const YAML::Node &node)
{
	fast::load(cores, node["cores"]);
    fast::load(result, node["result"]);
}

}
}
}
}
