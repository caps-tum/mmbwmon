#include <iostream>
#include <string>
#include <vector>

#include <cstring>

#include <fast-lib/message/agent/mmbwmon/reply.hpp>
#include <fast-lib/message/agent/mmbwmon/request.hpp>
#include <fast-lib/mqtt_communicator.hpp>

#include "helper.hpp"

[[noreturn]] static void print_help(const char *argv) {
	std::cout << argv << " supports the following flags:\n";
	std::cout << "\t --server \t URI of the MQTT broker. \t\t\t Required!\n";
	std::cout << "\t --port \t Port of the MQTT broker. \t\t\t Default: 1883\n";
	std::cout << "\t --core \t Core to be used by distgen. \t\t\t Can be used multiple times\n";
	exit(0);
}

static std::vector<size_t> cores;

static void parse_options(size_t argc, const char **argv) {
	if (argc == 1) {
		print_help(argv[0]);
	}

	for (size_t i = 1; i < argc; ++i) {
		std::string arg(argv[i]);

		if (arg == "--server") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			server = std::string(argv[i + 1]);
			++i;
			continue;
		}
		if (arg == "--port") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			port = std::stoul(std::string(argv[i + 1]));
			++i;
			continue;
		}
		if (arg == "--core") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			cores.push_back(std::stoul(std::string(argv[i + 1])));
			++i;
			continue;
		}
	}

	if (server == "") print_help(argv[0]);
}

int main(int argc, char const *argv[]) {
	const std::string requestID = "fast/agent/" + get_hostname() + "/mmbwmon/requester";

	parse_options(static_cast<size_t>(argc), argv);

	fast::MQTT_communicator comm(requestID, baseTopic + "/reply", baseTopic + "/request", server,
								 static_cast<int>(port), 60);

	std::cout << "MQTT ready!\n\n";

	fast::msg::agent::mmbwmon::request r(cores);
	std::cout << "Going to send message:\n" << r.to_string() << "\n";
	comm.send_message(r.to_string());

	fast::msg::agent::mmbwmon::reply reply;
	reply.from_string(comm.get_message());
	std::cout << "Got the following reply:\n" << reply.to_string() << "\n";
}
