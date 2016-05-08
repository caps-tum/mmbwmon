#include <fast-lib/message/agent/stop_monitor.hpp>

namespace fast {
namespace msg {
namespace agent {

//
// job_description implementation
//

job_description::job_description(std::string job_id, unsigned int process_id) :
	job_id(std::move(job_id)),
	process_id(process_id)
{
}

YAML::Node job_description::emit() const
{
	YAML::Node node;
	node["job-id"] = job_id;
	node["process-id"] = process_id;
	return node;
}

void job_description::load(const YAML::Node &node)
{
	fast::load(job_id, node["job-id"]);
	fast::load(process_id, node["process-id"]);
}

//
// stop_monitoring implementation
//

stop_monitoring::stop_monitoring(std::string job_id, unsigned int process_id) :
	job_desc(std::move(job_id), process_id)
{
}

YAML::Node stop_monitoring::emit() const
{
	YAML::Node node;
	node["task"] = "stop monitoring";
	node["job-description"] = job_desc.emit();
	return node;
}

void stop_monitoring::load(const YAML::Node &node)
{
	fast::load(job_desc.job_id, node["job-description"]["job-id"]);
	fast::load(job_desc.process_id, node["job-description"]["process-id"]);
}

}
}
}
