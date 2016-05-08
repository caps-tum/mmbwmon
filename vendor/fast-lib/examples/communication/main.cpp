/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/mqtt_communicator.hpp>
#include <fast-lib/log.hpp>

#include <cstdlib>
#include <exception>
#include <thread>

FASTLIB_LOG_INIT(comm_test_log, "communication tests")

int main(int argc, char *argv[])
{
	(void)argc;(void)argv;
	FASTLIB_LOG_SET_LEVEL(comm_test_log, trace);
	try {
		std::string id = "test-id";
		std::string subscribe_topic = "topic1";
		std::string publish_topic = "topic1";
		std::string host = "localhost";
		int port = 1883;
		int keepalive = 60;

		FASTLIB_LOG(comm_test_log, info) << "Creating a MQTT_communicator for use in test cases.";
		fast::MQTT_communicator comm(id, subscribe_topic, publish_topic, host, port, keepalive, std::chrono::seconds(5));

		FASTLIB_LOG(comm_test_log, info) << "Test of a second MQTT_communicator instance with random id.";
		{
			fast::MQTT_communicator comm("", subscribe_topic, publish_topic, host, port, keepalive, std::chrono::seconds(5));
		}
		FASTLIB_LOG(comm_test_log, info) << "Test of defered MQTT_communicator initialization.";
		{
			// Expected: success
			std::string msg;
			fast::MQTT_communicator comm("", publish_topic);
			FASTLIB_LOG(comm_test_log, debug) << "Add subscriptions.";
			comm.add_subscription("topic3", [&msg](std::string received_msg) {
					FASTLIB_LOG(comm_test_log, debug) << "Received in callback: " << received_msg;
					msg = std::move(received_msg);
			});
			comm.add_subscription("topic4");
			comm.connect_to_broker(host, port, keepalive, std::chrono::seconds(5));
			FASTLIB_LOG(comm_test_log, debug) << "Sending messages.";
			comm.send_message("Hallo Welt", "topic3");
			comm.send_message("Hallo Welt", "topic4");
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}
			std::string msg2 = comm.get_message("topic4");
			if (msg2 != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg2;
				return EXIT_FAILURE;
			}
			FASTLIB_LOG(comm_test_log, debug) << "Messages received.";
		}

		FASTLIB_LOG(comm_test_log, info) << "Send and receive test.";
		{
			// Expected: success
			comm.send_message("Hallo Welt");
			std::string msg = comm.get_message();
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}
		}
		FASTLIB_LOG(comm_test_log, info) << "Receive with timeout test.";
		{
			try {
				// Expected: success
				comm.send_message("Hallo Welt");
				FASTLIB_LOG(comm_test_log, debug) << "Waiting for message. (3s timeout)";
				std::string msg = comm.get_message(std::chrono::seconds(3));
				if (msg != "Hallo Welt") {
					FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
					FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
					return EXIT_FAILURE;
				}

				// Expected: timeout
				FASTLIB_LOG(comm_test_log, debug) << "No message this time.";
				FASTLIB_LOG(comm_test_log, debug) << "Waiting for message. (3s timeout)";
				msg = comm.get_message(std::chrono::seconds(3));
				FASTLIB_LOG(comm_test_log, debug) << "This output should not be reached due to timeout exception.";
				return EXIT_FAILURE;
			} catch (const std::runtime_error &e) {
				if (e.what() != std::string("Timeout while waiting for message.")) {
					FASTLIB_LOG(comm_test_log, debug) << "Expected timeout exception but another exception was thrown.";
					return EXIT_FAILURE;
				}
			}
		}
		FASTLIB_LOG(comm_test_log, info) << "Add and remove subscription test.";
		{
			// Expected: success
			FASTLIB_LOG(comm_test_log, debug) << "Add subscription on \"topic2\".";
			comm.add_subscription("topic2");
			FASTLIB_LOG(comm_test_log, debug) << "Sending message on \"topic2\".";
			comm.send_message("Hallo Welt", "topic2");
			FASTLIB_LOG(comm_test_log, debug) << "Waiting for message on \"topic2\".";
			std::string msg = comm.get_message("topic2");
			FASTLIB_LOG(comm_test_log, debug) << "Message received on \"topic2\". ";
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}

			// Expected: missing subscription
			FASTLIB_LOG(comm_test_log, debug) << "Remove subscription on \"topic2\".";
			comm.remove_subscription("topic2");
			try {
				FASTLIB_LOG(comm_test_log, debug) << "Sending message on \"topic2\".";
				comm.send_message("Hallo Welt", "topic2");
				FASTLIB_LOG(comm_test_log, debug) << "Waiting for message on \"topic2\".";
				std::string msg = comm.get_message("topic2", std::chrono::seconds(1));
				FASTLIB_LOG(comm_test_log, debug) << "This output should not be reached due to topic-not-found exception.";
				return EXIT_FAILURE;
			} catch (const std::out_of_range &e) {
				if (e.what() != std::string("Topic not found in subscriptions.")) {
					FASTLIB_LOG(comm_test_log, debug) << "Expected topic-not-found exception but another exception was thrown.";
					return EXIT_FAILURE;
				}
			}
		}
		FASTLIB_LOG(comm_test_log, info) << "Add subscription with callback test.";
		{
			// Expected: success
			std::string msg;
			FASTLIB_LOG(comm_test_log, debug) << "Add subscription on \"topic3\" with callback.";
			comm.add_subscription("topic3", [&msg](std::string received_msg) {
					FASTLIB_LOG(comm_test_log, debug) << "Received in callback: " << received_msg;
					msg = std::move(received_msg);
			});
			FASTLIB_LOG(comm_test_log, debug) << "Sending message on \"topic3\".";
			comm.send_message("Hallo Welt", "topic3");
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}
			FASTLIB_LOG(comm_test_log, debug) << "Message received on \"topic3\".";

			// Expected: cannot get_message from subscription with callback
			try {
				FASTLIB_LOG(comm_test_log, debug) << "Try to get message from topic with callback.";
				comm.get_message("topic3");
				FASTLIB_LOG(comm_test_log, debug) << "This output should not be reached due to wrong-subscription-type exception.";
				return EXIT_FAILURE;
			} catch (const std::runtime_error &e) {
				if (e.what() != std::string("Error in get_message: This topic is subscribed with callback.")) {
					FASTLIB_LOG(comm_test_log, debug) << "Expected wrong-subscription-type exception but another exception was thrown.";
					return EXIT_FAILURE;
				}
			}
			FASTLIB_LOG(comm_test_log, debug) << "Remove subscription on \"topic2\"";
			comm.remove_subscription("topic3");
		}
		FASTLIB_LOG(comm_test_log, info) << "Wildcard + test.";
		{
			// Expected: success
			FASTLIB_LOG(comm_test_log, debug) << "Add subscription on \"A/+/B\".";
			comm.add_subscription("A/+/B");
			FASTLIB_LOG(comm_test_log, debug) << "Sending message on \"A/C/B\".";
			comm.send_message("Hallo Welt", "A/C/B");
			FASTLIB_LOG(comm_test_log, debug) << "Waiting for message on \"A/+/B\".";
			auto msg = comm.get_message("A/+/B", std::chrono::seconds(3));
			FASTLIB_LOG(comm_test_log, debug) << "Message received on \"A/+/B\".";
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}
			comm.remove_subscription("A/+/B");
		}
		FASTLIB_LOG(comm_test_log, info) << "Wildcard # test.";
		{
			// Expected: success
			FASTLIB_LOG(comm_test_log, debug) << "Add subscription on \"A/#\".";
			comm.add_subscription("A/#");
			FASTLIB_LOG(comm_test_log, debug) << "Sending message on \"A/C/B\".";
			comm.send_message("Hallo Welt", "A/C/B");
			FASTLIB_LOG(comm_test_log, debug) << "Waiting for message on \"A/#\".";
			auto msg = comm.get_message("A/#", std::chrono::seconds(3));
			FASTLIB_LOG(comm_test_log, debug) << "Message received on \"A/#\".";
			if (msg != "Hallo Welt") {
				FASTLIB_LOG(comm_test_log, debug) << "Received unexpected message.";
				FASTLIB_LOG(comm_test_log, debug) << "Message contents:\n" << msg;
				return EXIT_FAILURE;
			}
			comm.remove_subscription("A/#");
		}
	} catch (const std::exception &e) {
		FASTLIB_LOG(comm_test_log, debug) << "Exception: " << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
