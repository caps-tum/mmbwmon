/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/mqtt_communicator.hpp>
#include <fast-lib/serializable.hpp>
#include <fast-lib/log.hpp>

#include <cstdlib>
#include <exception>

FASTLIB_LOG_INIT(comm_serial_test_log, "communication tests")

// Inherit from fast::Serializable
struct Data : 
	public fast::Serializable
{
	std::string task;
	unsigned int id;
	std::vector<std::string> vms;

	// Override these two methods to state which members should be serialized
	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;
};

int main(int argc, char *argv[])
{
	(void)argc;(void)argv;
	FASTLIB_LOG_SET_LEVEL(comm_serial_test_log, trace);
	try {
		std::string id = "test-id";
		std::string subscribe_topic = "topic1";
		std::string publish_topic = "topic1";
		std::string host = "localhost";
		int port = 1883;
		int keepalive = 60;

		fast::MQTT_communicator comm(id, subscribe_topic, publish_topic, host, port, keepalive);

		Data d1;
		d1.task = "greet";
		d1.id = 42;
		d1.vms = {"vm-name-1", "vm-name-2"};
		comm.send_message(d1.to_string());

		Data d2;
		d2.from_string(comm.get_message());
		if (d2.task != d2.task || d1.id != d2.id || d1.vms.size() != d2.vms.size()) {
			FASTLIB_LOG(comm_serial_test_log, info) << "Received data is corrupt.";
			return EXIT_FAILURE;
		}
	} catch (const std::exception &e) {
		FASTLIB_LOG(comm_serial_test_log, info) << "Exception: " << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

YAML::Node Data::emit() const
{
	FASTLIB_LOG(comm_serial_test_log, info) << "Emitting data to node.";
	YAML::Node node;
	node["task"] = task;
	node["data-id"] = id;
	node["vms"] = vms;
	return node;
}

void Data::load(const YAML::Node &node)
{
	FASTLIB_LOG(comm_serial_test_log, info) << "Loading data from node.";
	fast::load(task, node["task"], "idle"); // "idle" is the default value if yaml does not contain the node "task"
	fast::load(id, node["data-id"]); // fast::load is like calling "id = node.as<decltype(id)>()"
	fast::load(vms, node["vms"]);
}
