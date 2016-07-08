#include <fast-lib/message/agent/mmbwmon/ack.hpp>

namespace fast {
namespace msg {
namespace agent {
namespace mmbwmon {


YAML::Node ack::emit() const
{
	YAML::Node node;
	node["task"] = "mmbwmon ack";
	return node;
}

void ack::load(const YAML::Node &node)
{
}

}
}
}
}
