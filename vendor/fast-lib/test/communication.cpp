#include <fructose/fructose.h>

#include <fast-lib/mqtt_communicator.hpp>

#include <memory>
#include <chrono>
#include <thread>

struct Communication_tester :
	public fructose::test_base<Communication_tester>
{
	std::string id;
	std::string topic1;
	std::string topic2;
	std::string wildcard_topic1;
	std::string wildcard_topic2;
	std::string host;
	int port;
	int keepalive;
	fast::MQTT_communicator comm;

	Communication_tester(std::string host = "localhost") :
		id(""),
		topic1("test/topic1"),
		topic2("test/topic2"),
		wildcard_topic1("test/wildcard/#"),
		wildcard_topic2("test/wildcard/+"),
		host(host),
		port(1883),
		keepalive(60),
		comm(id, topic1)
	{
	}

	void connect(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(!comm.is_connected());
		fructose_assert_no_exception(
			comm.connect_to_broker(host, port, keepalive, std::chrono::seconds(5))
		);
		fructose_assert(comm.is_connected());
	}

	void second_communicator(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		std::unique_ptr<fast::MQTT_communicator> comm_ptr;
		fructose_assert_no_exception(
			comm_ptr.reset(new fast::MQTT_communicator("", topic1, host, port, keepalive, std::chrono::seconds(5)))
		);
		fructose_assert(comm_ptr->is_connected());
		// Sleep a second before comm_ptr gets destroyed
		// Otherwise there is a race between destruction and getting on_connect called (FIXME?)
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	void subscribe(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		fructose_assert_no_exception(
			comm.add_subscription(topic1)
		);
		fructose_assert_no_exception(
			comm.add_subscription(wildcard_topic1)
		);
		fructose_assert_no_exception(
			comm.add_subscription(wildcard_topic2)
		);
	}

	void send_receive(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		const std::string original_msg("Hallo Welt");
		std::string msg;
		fructose_assert_no_exception(
			comm.send_message(original_msg, topic1)
		);
		fructose_assert_no_exception(
			msg = comm.get_message(topic1, std::chrono::seconds(5))
		);
		fructose_assert_eq(msg, original_msg);
	}

	void wildcard1(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		const std::string original_msg("Hallo Welt");
		const std::string topic = "test/wildcard/topic-1";
		std::string msg;
		fructose_assert_no_exception(
			comm.send_message(original_msg, topic)
		);
		std::string actual_topic;
		fructose_assert_no_exception(
			msg = comm.get_message(wildcard_topic1, std::chrono::seconds(5), &actual_topic)
		);
		fructose_assert_eq(msg, original_msg);
		fructose_assert_eq(actual_topic, topic);
	}

	void wildcard2(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		const std::string original_msg("Hallo Welt");
		const std::string topic = "test/wildcard/topic-1";
		std::string msg;
		fructose_assert_no_exception(
			comm.send_message(original_msg, topic)
		);
		std::string actual_topic;
		fructose_assert_no_exception(
			msg = comm.get_message(wildcard_topic2, std::chrono::seconds(5), &actual_topic)
		);
		fructose_assert_eq(msg, original_msg);
		fructose_assert_eq(actual_topic, topic);
	}

	void unsubscribe(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		fructose_assert_no_exception(
			comm.remove_subscription(wildcard_topic2)
		);
		fructose_assert_no_exception(
			comm.remove_subscription(wildcard_topic1)
		);
		fructose_assert_no_exception(
			comm.remove_subscription(topic1)
		);
	}

	void disconnect(const std::string &test_name)
	{
		(void) test_name;
		fructose_assert(comm.is_connected());
		fructose_assert_no_exception(
			comm.disconnect_from_broker();
		);
		std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO: Implement wait for disconnect in disconnect_from_broker
		fructose_assert(!comm.is_connected());
	}
};

int main(int argc, char **argv)
{
	Communication_tester tests;
	tests.add_test("connect", &Communication_tester::connect);
	tests.add_test("second communicator", &Communication_tester::second_communicator);
	tests.add_test("subscribe", &Communication_tester::subscribe);
	tests.add_test("send and receive", &Communication_tester::send_receive);
	tests.add_test("wildcard #", &Communication_tester::wildcard1);
	tests.add_test("wildcard +", &Communication_tester::wildcard2);
	tests.add_test("unsubscribe", &Communication_tester::unsubscribe);
	tests.add_test("disconnect", &Communication_tester::disconnect);
	return tests.run(argc, argv);
}
