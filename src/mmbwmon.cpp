#include <iostream>
#include <string>
#include <thread>

#include <cstring>

#include <distgen/distgen.h>

#include <fast-lib/message/agent/mmbwmon/reply.hpp>
#include <fast-lib/message/agent/mmbwmon/request.hpp>
#include <fast-lib/mqtt_communicator.hpp>

#include "helper.hpp"

/*** config vars **/
static distgend_initT distgen_init;

[[noreturn]] static void print_help(const char *argv) {
	std::cout << argv << " supports the following flags:\n";
	std::cout << "\t --server \t URI of the MQTT broker. \t\t\t Required!\n";
	std::cout << "\t --port \t Port of the MQTT broker. \t\t\t Default: 1883\n";
	std::cout << "\t --numa \t Number of NUMA domains. \t\t\t Default: 2\n";
	std::cout << "\t --threads \t Number of logical cores. \t\t\t Default: " << std::thread::hardware_concurrency()
			  << "\n";
	std::cout << "\t --smt \t\t Number of logical cores per physical core. \t Default: 2\n";
	exit(0);
}

static void parse_options(size_t argc, const char **argv) {
	if (argc == 1) {
		print_help(argv[0]);
	}
	distgen_init.number_of_threads = std::thread::hardware_concurrency();
	distgen_init.NUMA_domains = 2;
	distgen_init.SMT_factor = 2;

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
		if (arg == "--numa") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			distgen_init.NUMA_domains = std::stoul(std::string(argv[i + 1]));
			++i;
			continue;
		}
		if (arg == "--threads") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			distgen_init.number_of_threads = std::stoul(std::string(argv[i + 1]));
			++i;
			continue;
		}
		if (arg == "--smt") {
			if (i + 1 >= argc) {
				print_help(argv[0]);
			}
			distgen_init.SMT_factor = std::stoul(std::string(argv[i + 1]));
			++i;
			continue;
		}
	}

	if (server == "") print_help(argv[0]);
}

static void print_distgen_results(distgend_initT distgen_init) {
	assert(distgen_init.number_of_threads / distgen_init.SMT_factor - 1 < DISTGEN_MAXTHREADS);
	distgend_configT config;
	for (unsigned char i = 0; i < distgen_init.number_of_threads / distgen_init.SMT_factor; ++i) {
		config.number_of_threads = i + 1;
		config.threads_to_use[i] = i;
		std::cout << "Using " << i + 1 << " threads:" << std::endl;
		std::cout << "\tMaximum: " << distgend_get_max_bandwidth(config) << " GByte/s" << std::endl;
		std::cout << std::endl;
	}
}

int main(int argc, char const *argv[]) {
	parse_options(static_cast<size_t>(argc), argv);

	const std::string agentID = "fast/agent/" + get_hostname() + "/mmbwmon";
	fast::MQTT_communicator comm(agentID, baseTopic + "/request", baseTopic + "/reply", server, static_cast<int>(port),
								 60);

	std::cout << "MQTT ready!\n\n";

	std::cout << "Starting distgen initialization ...";
	std::cout.flush();
	try {
		distgend_init(distgen_init);
	} catch (...) {
		std::cout << "\n\nDistgen init failed. Please make sure that cgroup is mounted at /sys/fs/cgroup/ and you "
					 "have read + write access.";
		exit(-1);
	}

	std::cout << " done!\n\n";

	print_distgen_results(distgen_init);

	while (true) {
		fast::msg::agent::mmbwmon::request req;
		auto m = comm.get_message();
		std::cout << "Got message:\n" << m << "\n";
		req.from_string(m);

		distgend_configT dc;
		dc.number_of_threads = req.cores.size();
		for (int i = 0; i < req.cores.size(); ++i) {
			dc.threads_to_use[i] = req.cores[i];
		}

		const double mem = distgend_is_membound(dc);
		fast::msg::agent::mmbwmon::reply reply(req.cores, mem);
		std::cout << "Sending message:\n" << reply.to_string() << "\n";
		comm.send_message(reply.to_string(), baseTopic + "/reply");
	}
}
