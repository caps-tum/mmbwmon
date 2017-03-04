/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#include <fast-lib/log.hpp>
#include <fast-lib/mqtt_communicator.hpp>

#include <cstdlib>
#include <queue>
#include <regex>
#include <stdexcept>
#include <thread>

FASTLIB_LOG_INIT(comm_log, "MQTT_communicator")

FASTLIB_LOG_SET_LEVEL_GLOBAL(comm_log, trace);

namespace fast {

/// Helper function to make error codes human readable.
static std::string mosq_err_string(const std::string &str, int code)
{
	return str + mosqpp::strerror(code);
}

/// Helper function to convert a given topic into a regular expression
static std::regex topic_to_regex(const std::string &topic)
{
	// Replace "+" by "[^/]*"
	auto regex_topic = std::regex_replace(topic, std::regex(R"((\+))"), R"([^/]*)");
	// Replace "#" by "[^/]*(?:/[^/]*)*$"
	regex_topic = std::regex_replace(regex_topic, std::regex(R"((#))"), R"([^/]*(?:/[^/]*)*$)");
	return std::regex(regex_topic);
}



class MQTT_subscription
{
public:
	MQTT_subscription(int qos);
	virtual ~MQTT_subscription() = default;
	virtual void add_message(const mosquitto_message *msg) = 0;
	virtual std::string get_message(const std::chrono::duration<double> &duration, std::string *actual_topic = nullptr) = 0;
	const int qos;
};

MQTT_subscription::MQTT_subscription(int qos) :
	qos(qos)
{
}

class MQTT_subscription_get : public MQTT_subscription
{
public:
	MQTT_subscription_get(int qos);
	void add_message(const mosquitto_message *msg) override;
	std::string get_message(const std::chrono::duration<double> &duration, std::string *actual_topic = nullptr) override;
private:
	std::mutex msg_queue_mutex;
	std::condition_variable msg_queue_empty_cv;
	std::queue<mosquitto_message*> messages; /// \todo Consider using unique_ptr.
};

class MQTT_subscription_callback : public MQTT_subscription
{
public:
	MQTT_subscription_callback(int qos, std::function<void(std::string)> callback);
	void add_message(const mosquitto_message *msg) override;
	std::string get_message(const std::chrono::duration<double> &duration, std::string *actual_topic = nullptr) override;
private:
	std::string message;
	std::function<void(std::string)> callback;
};

MQTT_subscription_get::MQTT_subscription_get(int qos) :
	MQTT_subscription(qos)
{
}

void MQTT_subscription_get::add_message(const mosquitto_message *msg)
{
	mosquitto_message* buf = nullptr;
	buf = static_cast<mosquitto_message*>(std::malloc(sizeof(mosquitto_message)));
	if (!buf)
		throw std::runtime_error("malloc failed allocating mosquitto_message.");
	std::lock_guard<std::mutex> lock(msg_queue_mutex);
	messages.push(buf);
	mosquitto_message_copy(messages.back(), msg);
	if (messages.size() == 1)
		msg_queue_empty_cv.notify_one();
}

std::string MQTT_subscription_get::get_message(const std::chrono::duration<double> &duration, std::string *actual_topic)
{
	std::unique_lock<std::mutex> lock(msg_queue_mutex);
	if (duration == std::chrono::duration<double>::max()) {
		// Wait without timeout
		msg_queue_empty_cv.wait(lock, [this]{return !messages.empty();});
	} else {
		// Wait with timeout
		if (!msg_queue_empty_cv.wait_for(lock, duration, [this]{return !messages.empty();}))
			throw std::runtime_error("Timeout while waiting for message.");
	}
	auto msg = messages.front();
	messages.pop();
	lock.unlock();
	std::string buf(static_cast<char*>(msg->payload), msg->payloadlen);
	if (actual_topic)
		actual_topic->assign(msg->topic);
	mosquitto_message_free(&msg);
	return buf;
}

MQTT_subscription_callback::MQTT_subscription_callback(int qos, std::function<void(std::string)> callback) :
	MQTT_subscription(qos),
	callback(std::move(callback))
{
}


void MQTT_subscription_callback::add_message(const mosquitto_message *msg)
{
	callback(std::string(static_cast<char*>(msg->payload), msg->payloadlen));
}

std::string MQTT_subscription_callback::get_message(const std::chrono::duration<double> &duration, std::string *actual_topic)
{
	(void) duration, (void) actual_topic;
	throw std::runtime_error("Error in get_message: This topic is subscribed with callback.");
}

MQTT_communicator::MQTT_communicator(const std::string &id, const std::string &publish_topic) :
	mosqpp::mosquittopp(id == "" ? nullptr : id.c_str()),
	default_publish_topic(publish_topic),
	connected(false)
{
	init_mosq_lib();
	start_mosq_loop();
}

MQTT_communicator::MQTT_communicator(const std::string &id,
				     const std::string &publish_topic,
				     const std::string &host,
				     int port,
				     int keepalive,
				     const timeout_duration_t &timeout) :
	MQTT_communicator(id, publish_topic)
{
	connect_to_broker(host, port, keepalive, timeout);
}
MQTT_communicator::MQTT_communicator(const std::string &id,
				     const std::string &subscribe_topic,
				     const std::string &publish_topic,
				     const std::string &host,
				     int port,
				     int keepalive,
				     int qos,
				     const timeout_duration_t &timeout) :
	MQTT_communicator(id, publish_topic, host, port, keepalive, timeout)
{
	// Subscribe to default topic.
	FASTLIB_LOG(comm_log, trace) << "Add default subscription.";
	default_subscribe_topic = subscribe_topic;
	add_subscription(default_subscribe_topic, qos);
}

MQTT_communicator::~MQTT_communicator()
{
	FASTLIB_LOG(comm_log, trace) << "Destructing MQTT_communicator.";
	try {
		disconnect_from_broker();
		stop_mosq_loop();
		cleanup_mosq_lib();
	} catch(const std::exception &e) {
		FASTLIB_LOG(comm_log, warn) << e.what();
	} catch(...) {
		FASTLIB_LOG(comm_log, warn) << "Something was thrown.";
	}
	FASTLIB_LOG(comm_log, trace) << "MQTT_communicator destructed.";
}

void MQTT_communicator::add_subscription(const std::string &topic, int qos) const
{
	// Save subscription in unordered_map.
	std::shared_ptr<MQTT_subscription> ptr = std::make_shared<MQTT_subscription_get>(qos);
	std::unique_lock<std::mutex> lock(subscriptions_mutex);
	subscriptions.emplace(std::make_pair(topic, ptr));
	lock.unlock();
	// Send subscribe to MQTT broker.
	if (connected) {
		auto ret = subscribe(nullptr, topic.c_str(), qos);
		if (ret != MOSQ_ERR_SUCCESS)
			throw std::runtime_error(mosq_err_string("Error subscribing to topic \"" + topic + "\": ", ret));
	}
}

void MQTT_communicator::add_subscription(const std::string &topic, std::function<void(std::string)> callback, int qos) const
{
	// Save subscription in unordered_map.
	std::shared_ptr<MQTT_subscription> ptr = std::make_shared<MQTT_subscription_callback>(qos, std::move(callback));
	std::unique_lock<std::mutex> lock(subscriptions_mutex);
	subscriptions.emplace(std::make_pair(topic, ptr));
	lock.unlock();
	// Send subscribe to MQTT broker.
	if (connected) {
		auto ret = subscribe(nullptr, topic.c_str(), qos);
		if (ret != MOSQ_ERR_SUCCESS)
			throw std::runtime_error(mosq_err_string("Error subscribing to topic \"" + topic + "\": ", ret));
	}
}

void MQTT_communicator::remove_subscription(const std::string &topic) const
{
	// Delete subscription from unordered_map.
	// This does not invalidate references used by other threads due to use of shared_ptr.
	std::unique_lock<std::mutex> lock(subscriptions_mutex);
	subscriptions.erase(topic);
	lock.unlock();
	// Send unsubscribe to MQTT broker.
	if (connected) {
		auto ret = unsubscribe(nullptr, topic.c_str());
		if (ret != MOSQ_ERR_SUCCESS)
			throw std::runtime_error(mosq_err_string("Error subscribing to topic \"" + topic + "\": ", ret));
	}
}

void MQTT_communicator::on_connect(int rc)
{
	(void) rc;
	FASTLIB_LOG(comm_log, trace) << "Callback: on_connect(" << std::to_string(rc) << ")";
	if (rc == 0) {
		FASTLIB_LOG(comm_log, trace) << "Setting connected flag and notify constructor.";
		std::unique_lock<std::mutex> lock(connected_mutex);
		connected = true;
		lock.unlock();
		connected_cv.notify_one();
		FASTLIB_LOG(comm_log, trace) << "Connected flag is set and constructor is notified.";
	} else {
		FASTLIB_LOG(comm_log, trace) << "Error on connect: " << mosqpp::connack_string(rc);
	}
}

void MQTT_communicator::on_disconnect(int rc)
{
	(void) rc;
	FASTLIB_LOG(comm_log, trace) << "Callback: on_disconnect(" << std::to_string(rc) << ")";
	if (rc == 0) {
		FASTLIB_LOG(comm_log, trace) << "Disconnected.";
	} else {
		FASTLIB_LOG(comm_log, trace) << mosq_err_string("Unexpected disconnect: ", rc);
	}
	FASTLIB_LOG(comm_log, trace) << "Unsetting connected flag.";
	std::lock_guard<std::mutex> lock(connected_mutex);
	connected = false;
	FASTLIB_LOG(comm_log, trace) << "Connected flag is unset.";
}


void MQTT_communicator::on_message(const mosquitto_message *msg)
{
	FASTLIB_LOG(comm_log, trace) << "Callback: on_message with topic: " << msg->topic;
	try {
		std::vector<decltype(subscriptions)::mapped_type> matched_subscriptions;
		// Get all subscriptions matching the topic
		std::unique_lock<std::mutex> lock(subscriptions_mutex);
		for (auto &subscription : subscriptions) {
			if (std::regex_match(msg->topic, topic_to_regex(subscription.first)))
				matched_subscriptions.push_back(subscription.second);
		}
		lock.unlock();
		if (matched_subscriptions.size() == 0)
			throw std::runtime_error("No matching subscriptions.");
		// Add message to all matched subscriptions
		for (auto &subscription : matched_subscriptions)
			subscription->add_message(msg);
	} catch (const std::exception &e) { // Catch exceptions and do nothing to not break mosquitto loop.
		FASTLIB_LOG(comm_log, trace) << "Exception in on_message: " << e.what();
	}

}

void MQTT_communicator::send_message(const std::string &message) const
{
	send_message(message, "", 1);
}

void MQTT_communicator::send_message(const std::string &message, const std::string &topic, int qos) const
{
	FASTLIB_LOG(comm_log, trace) << "Sending message.";
	if (!connected)
		throw std::runtime_error("No connection established.");
	// Use default topic if empty string is passed.
	auto &real_topic = topic == "" ? default_publish_topic : topic;
	// Publish message to topic.
	int ret = publish(nullptr, real_topic.c_str(), static_cast<int>(message.size()), message.c_str(), qos, false);
	if (ret != MOSQ_ERR_SUCCESS)
		throw std::runtime_error(mosq_err_string("Error sending message: ", ret));
	FASTLIB_LOG(comm_log, trace) << "Message sent to topic " << real_topic << ".";
}

std::string MQTT_communicator::get_message(std::string *actual_topic) const
{
	return get_message(default_subscribe_topic, std::chrono::duration<double>::max(), actual_topic);
}

std::string MQTT_communicator::get_message(const std::string &topic, std::string *actual_topic) const
{
	return get_message(topic, std::chrono::duration<double>::max(), actual_topic);
}

std::string MQTT_communicator::get_message(const std::chrono::duration<double> &duration, std::string *actual_topic) const
{
	return get_message(default_subscribe_topic, duration, actual_topic);
}

std::string MQTT_communicator::get_message(const std::string &topic, const std::chrono::duration<double> &duration, std::string *actual_topic) const
{
	FASTLIB_LOG(comm_log, trace) << "Getting message for topic " << topic << ".";
	if (!connected)
		throw std::runtime_error("No connection established.");
	try {
		std::unique_lock<std::mutex> lock(subscriptions_mutex);
		auto &subscription = subscriptions.at(topic);
		lock.unlock();
		return subscription->get_message(duration, actual_topic);
	} catch (const std::out_of_range &/*e*/) {
		throw std::out_of_range("Topic not found in subscriptions.");
	}
}


void MQTT_communicator::init_mosq_lib() const
{
	std::lock_guard<std::mutex> lock(ref_count_mutex);
	if (ref_count++ == 0) {
		FASTLIB_LOG(comm_log, trace) << "Initialize mosquitto library.";
		mosqpp::lib_init();
	}
}

void MQTT_communicator::cleanup_mosq_lib() const
{
	std::lock_guard<std::mutex> lock(ref_count_mutex);
	if (--ref_count == 0) {
		FASTLIB_LOG(comm_log, trace) << "Clean mosquitto library up.";
		mosqpp::lib_cleanup();
	}
}

// Connect to MQTT broker. Uses condition variable that is set in on_connect, because
// (re-)connect returning MOSQ_ERR_SUCCESS does not guarantee an fully established connection.
void MQTT_communicator::connect_to_broker(
		const std::string &host,
		int port,
		int keepalive,
		const timeout_duration_t &timeout) const
{
	FASTLIB_LOG(comm_log, trace) << "Connect to MQTT broker.";
	if (connected)
		throw std::runtime_error("Already connected.");
	auto start = std::chrono::high_resolution_clock::now();
	int ret = connect(host.c_str(), port, keepalive);
	FASTLIB_LOG(comm_log, trace) << "Called connect api call.";
	while (ret != MOSQ_ERR_SUCCESS) {
		FASTLIB_LOG(comm_log, trace) << mosq_err_string("Failed connecting to MQTT broker: ", ret);
		if (std::chrono::high_resolution_clock::now() - start > timeout)
			throw std::runtime_error("Timeout while trying to connect to MQTT broker.");
		std::this_thread::sleep_for(std::chrono::seconds(1));
		FASTLIB_LOG(comm_log, trace) << "Retry connecting.";
		ret = reconnect();
	}
	FASTLIB_LOG(comm_log, trace) << "Waiting for on_connect callback to signal success.";
	std::unique_lock<std::mutex> lock(connected_mutex);
	auto time_left = timeout - (std::chrono::high_resolution_clock::now() - start);
	// Branch between wait and wait_for because if time_left is max wait_for does not work
	// (waits until now + max -> overflow?).
	if (time_left != std::chrono::duration<double>::max()) {
		if (!connected_cv.wait_for(lock, time_left, [this]{return connected;}))
			throw std::runtime_error("Timeout while trying to connect to MQTT broker.");
	} else {
		connected_cv.wait(lock, [this]{return connected;});
	}
	resubscribe();
}

void MQTT_communicator::disconnect_from_broker() const
{
	// Disconnect from MQTT broker.
	if (connected) {
		disconnect();
	}
}

bool MQTT_communicator::is_connected() const
{
	return connected;
}

void MQTT_communicator::resubscribe() const
{
	if (!connected)
		throw std::runtime_error("No connection established.");
	std::lock_guard<std::mutex> lock(subscriptions_mutex);
	for (auto &iter : subscriptions) {
		// Send subscribe to MQTT broker.
		auto &topic = iter.first;
		auto &qos = iter.second->qos;
		auto ret = subscribe(nullptr, topic.c_str(), qos);
		if (ret != MOSQ_ERR_SUCCESS)
			throw std::runtime_error(mosq_err_string("Error subscribing to topic \"" + topic + "\": ", ret));
	}
}

void MQTT_communicator::start_mosq_loop() const
{
	FASTLIB_LOG(comm_log, trace) << "Start mosquitto loop";
	int ret;
	if ((ret = loop_start()) != MOSQ_ERR_SUCCESS)
		throw std::runtime_error(mosq_err_string("Error starting mosquitto loop: ", ret));

}

void MQTT_communicator::stop_mosq_loop() const
{
	FASTLIB_LOG(comm_log, trace) << "Stop mosquitto loop.";
	loop_stop();
}

std::mutex MQTT_communicator::ref_count_mutex;

unsigned int MQTT_communicator::ref_count = 0;

} // namespace fast
