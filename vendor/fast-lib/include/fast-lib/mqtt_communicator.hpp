/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_MQTT_COMMUNICATOR_HPP
#define FAST_LIB_MQTT_COMMUNICATOR_HPP

#include <fast-lib/communicator.hpp>

#include <mosquittopp.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace fast {

/**
 * \brief A handler for subscriptions.
 *
 * Used internally to handle subsriptions.
 */
class MQTT_subscription;

/**
 * \brief A specialized Communicator to provide communication using the MQTT framework mosquitto.
 *
 * This class is threadsafe.
 */
class MQTT_communicator :
	public Communicator,
	private mosqpp::mosquittopp
{
public:
	/**
	 * \brief The type of the timeout duration.
	 *
	 * The type must provide a max() method, which is reserved for no timeout.
	 */
	using timeout_duration_t = std::chrono::duration<double>;

	/**
	 * \brief Constructor for MQTT_communicator.
	 *
	 * This constructor only initializes the mosquitto library and starts the mosquitto loop.
	 * It does not establish a connection.
	 * Use the connect_to_broker() method to connect.
	 * \param id The id of this client. Must be unique, so the broker can identify this client. An empty string ("") can be passed for a random id.
	 * \param publish_topic The topic to publish messages to by default.
	 */
	MQTT_communicator(const std::string &id,
			  const std::string &publish_topic);

	/**
	 * \brief Constructor for MQTT_communicator.
	 *
	 * Establishes a connection and starts async mosquitto loop.
	 * If the connect attempt fails, it tries to reconnect every second until success or timeout.
	 * To disable the timeout it has to be set to "timeout_duration_t::max()" (default).
	 * \param id The id of this client. Must be unique, so the broker can identify this client. An empty string ("") can be passed for a random id.
	 * \param publish_topic The topic to publish messages to by default.
	 * \param host The host to connect to.
	 * \param port The port to connect to.
	 * \param keepalive The number of seconds the broker sends periodically ping messages to test if client is still alive.
	 * \param timeout The timeout of establishing a connection to the MQTT broker e.g. std::chrono::seconds(10). timeout_duration_t::max() is reserved for no timeout.
	 */
	MQTT_communicator(const std::string &id,
			  const std::string &publish_topic,
			  const std::string &host,
			  int port,
			  int keepalive,
			  const timeout_duration_t &timeout = timeout_duration_t::max());

	/**
	 * \brief Constructor for MQTT_communicator.
	 *
	 * Establishes a connection, starts async mosquitto loop and subscribes to topic.
	 * If the connect attempt fails, it tries to reconnect every second until success or timeout.
	 * To disable the timeout it has to be set to "timeout_duration_t::max()" (default).
	 * This overload also adds an default subscription to a topic.
	 * \param id The id of this client. Must be unique, so the broker can identify this client. An empty string ("") can be passed for a random id.
	 * \param subscribe_topic The topic to subscribe to by default.
	 * \param publish_topic The topic to publish messages to by default.
	 * \param host The host to connect to.
	 * \param port The port to connect to.
	 * \param keepalive The number of seconds the broker sends periodically ping messages to test if client is still alive.
	 * \param qos The quality of service (0, 1, or 2). See mosquitto documentation for further information.
	 * \param timeout The timeout of establishing a connection to the MQTT broker e.g. std::chrono::seconds(10). timeout_duration_t::max() is reserved for no timeout.
	 */
	MQTT_communicator(const std::string &id,
			  const std::string &subscribe_topic,
			  const std::string &publish_topic,
			  const std::string &host,
			  int port,
			  int keepalive,
			  int qos = 2,
			  const timeout_duration_t &timeout = timeout_duration_t::max());

	/**
	 * \brief Destructor for MQTT_communicator.
	 *
	 * Disconnects and stops async mosquitto loop.
	 */
	~MQTT_communicator();

	/**
	 * \brief Add a subscription to listen on for messages.
	 *
	 * Adds a subscription on a topic. The messages can be retrieved by calling get_message().
	 * Messages are queued seperate per topic. Therefore multiple topics can be subscribed simultaneously.
	 * \param topic The topic to listen on.
	 * \param qos The quality of service (0|1|2 - see mosquitto documentation for further information)
	 */
	void add_subscription(const std::string &topic, int qos = 2) const;

	/**
	 * \brief Add a subscription with a callback to retrieve messages.
	 *
	 * Adds a subscription on a topic. On each message that arrives the callback is called with the payload
	 * string as parameter. All exceptions derived from std::exception are caught if thrown by callback.
	 * \param topic The topic to listen on.
	 * \param callback The function to call when a new message arrives on topic.
	 * \param qos The quality of service (see mosquitto documentation for further information)
	 */
	void add_subscription(const std::string &topic, std::function<void(std::string)> callback, int qos = 2) const;

	/**
	 * \brief Remove a subscription.
	 *
	 * \param topic The topic the subscription was listening on.
	 */
	void remove_subscription(const std::string &topic) const;

	/**
	 * \brief Send a message to the default publish topic.
	 *
	 * The default publish topic can be set in the constructor.
	 * \param message The message string to send on the default topic.
	 */
	void send_message(const std::string &message) const override;

	/**
	 * \brief Send a message to a specific topic.
	 *
	 * \param message The message string to send on the topic.
	 * \param topic The topic to send the message on.
	 * \param qos The quality of service (0|1|2 - see mosquitto documentation for further information)
	 */
	void send_message(const std::string &message, const std::string &topic, int qos = 2) const;

	/**
	 * \brief Get a message from the default subscribe topic.
	 *
	 * This is a blocking method, which waits until a message is received.
	 * The default subscribe topic can be set in the constructor.
	 */
	std::string get_message(std::string *actual_topic = nullptr) const override;

	/**
	 * \brief Get a message from a specific topic.
	 *
	 * This is a blocking method, which waits until a message is received.
	 * \param topic The topic to listen on for a message.
	 */
	std::string get_message(const std::string &topic, std::string *actual_topic = nullptr) const;

	/**
	 * \brief Get a message from the default subscribe topic with timeout.
	 *
	 * This is a blocking method, which waits until a message is received or timeout is exceeded.
	 * The default subscribe topic can be set in the constructor.
	 * \param duration The duration until timeout.
	 */
	std::string get_message(const std::chrono::duration<double> &duration, std::string *actual_topic = nullptr) const;

	/**
	 * \brief Get a message from a specific topic with timeout.
	 *
	 * This is a blocking method, which waits until a message is received or timeout is exceeded.
	 * \param topic The topic to listen on for a message.
	 * \param duration The duration until timeout.
	 */
	std::string get_message(const std::string &topic,
				const std::chrono::duration<double> &duration, std::string *actual_topic = nullptr) const;

	/**
	 * \brief Connect to the mosquitto broker.
	 *
	 * Establishes a connection and starts async mosquitto loop.
	 * If the connect attempt fails, it tries to reconnect every second until success or timeout.
	 * To disable the timeout it has to be set to "timeout_duration_t::max()" (default).
	 * If a previous connection is still active, std::runtime_error is thrown.
	 * \param host The host to connect to.
	 * \param port The port to connect to.
	 * \param keepalive The number of seconds the broker sends periodically ping messages to test if client is still alive.
	 * \param timeout The timeout of establishing a connection to the MQTT broker e.g. std::chrono::seconds(10). timeout_duration_t::max() is reserved for no timeout.
	 */
	void connect_to_broker(const std::string &host,
				int port,
				int keepalive,
				const timeout_duration_t &timeout = timeout_duration_t::max()) const;

	/**
	 * \brief Disconnect from the mosquitto broker.
	 *
	 * Only disconnects if a connection was previously established, else nothing happens.
	 */
	void disconnect_from_broker() const;

	/**
	 * \brief Check if a connection is established.
	 */
	bool is_connected() const;
private:
	void resubscribe() const;
	/**
	 * \brief Callback for established connections.
	 */
	void on_connect(int rc) override;

	/**
	 * \brief Callback for disconnected connections.
	 */
	void on_disconnect(int rc) override;

	/**
	 * \brief Callback for received messages.
	 */
	void on_message(const mosquitto_message *msg) override;

	/**
	 * \brief Initializes the mosquitto library if necessary.
	 *
	 * Uses a reference counter, so mosquitto library is only initialized, if there is no other
	 * MQTT_communicator instance.
	 * Unlike the mosquitto library function for initializing, this method is thread safe being
	 * protected by a mutex.
	 */
	void init_mosq_lib() const;

	/**
	 * \brief Cleans the mosquitto library up if necessary.
	 *
	 * Uses a reference counter, so mosquitto library is only cleaned up, if this is the last
	 * MQTT_communicator instance.
	 * Unlike the mosquitto library function for cleanup, this method is thread safe being
	 * protected by a mutex.
	 */
	void cleanup_mosq_lib() const;

	/**
	 * \brief Starts the async mosquitto loop.
	 */
	void start_mosq_loop() const;

	/**
	 * \brief Stops the async mosquitto loop.
	 */
	void stop_mosq_loop() const;

	/**
	 * \brief The topic to get messages from by default.
	 */
	std::string default_subscribe_topic;

	/**
	 * \brief The topic to send messages to by default.
	 */
	std::string default_publish_topic;

	/**
	 * \brief A map with a topic as key and the associated subscription handler as value.
	 */
	mutable std::unordered_map<std::string, std::shared_ptr<MQTT_subscription>> subscriptions;

	/**
	 * \brief The mutex for safe access to the subscriptions map.
	 */
	mutable std::mutex subscriptions_mutex;

	/**
	 * \brief This flag states, if this MQTT_communicator is successfully connected.
	 */
	mutable bool connected;

	/**
	 * The mutex for safe access to the connected flag.
	 */
	mutable std::mutex connected_mutex;

	/**
	 * The condition variable to signal an established connection (connected set to true).
	 */
	mutable std::condition_variable connected_cv;

	/**
	 * The mutex for safe access to the ref_count.
	 */
	static std::mutex ref_count_mutex;

	/**
	 * \brief The reference counter used for init/cleanup of the mosquitto library.
	 */
	static unsigned int ref_count;
};

} // namespace fast
#endif
