#include <fast-lib/message/agent/mmbwmon/request.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {

request::request(const std::vector<size_t> &_cores) : cores(_cores)
{
}

YAML::Node request::emit() const
{
	YAML::Node node;
	node["task"] = "mmbwmon request";
	node["cores"] = cores;
	return node;
}

void request::load(const YAML::Node &node)
{
	fast::load(cores, node["cores"]);
}

}
}
}
}
