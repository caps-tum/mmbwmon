#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <cstdlib>
#include <cstring>

#include <pwd.h> // for getpwuid()

#include <distgen/distgen.h>

#include <fast-lib/message/agent/mmbwmon/ack.hpp>
#include <fast-lib/message/agent/mmbwmon/reply.hpp>
#include <fast-lib/message/agent/mmbwmon/request.hpp>
#include <fast-lib/message/agent/mmbwmon/restart.hpp>
#include <fast-lib/message/agent/mmbwmon/stop.hpp>
#include <fast-lib/mqtt_communicator.hpp>

#ifdef CGROUP_SUPPORT
#include <ponci/ponci.hpp>
#endif

#include "helper.hpp"

const std::string home_dir = std::string(getpwuid(getuid())->pw_dir) + "/.mmbwmon";

/*** config vars **/
static distgend_initT distgen_init;
static bool measure_only = false;
static bool home_dir_available = false;

[[noreturn]] static void print_help(const char *argv) {
	std::cout << argv << " supports the following flags:\n";
	std::cout << "\t --server \t URI of the MQTT broker. \t\t\t Required!\n";
	std::cout << "\t --port \t Port of the MQTT broker. \t\t\t Default: 1883\n";
	std::cout << "\t --numa \t Number of NUMA domains. \t\t\t Default: 2\n";
	std::cout << "\t --threads \t Number of logical cores. \t\t\t Default: " << std::thread::hardware_concurrency()
			  << "\n";
	std::cout << "\t --smt \t\t Number of logical cores per physical core. \t Default: 2\n";
	std::cout << "\t --measure-only  Only runs the initialization measurements. \t Default: false\n";
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
		if (arg == "--measure-only") {
			measure_only = true;
			continue;
		}
	}

	if (server == "" && !measure_only) print_help(argv[0]);
}

static void print_distgen_results(distgend_initT distgen_init) {
	assert(distgen_init.number_of_threads / distgen_init.SMT_factor - 1 < DISTGEN_MAXTHREADS);

	std::ofstream gnuplot_file;
	if (home_dir_available) {
		gnuplot_file.open(std::string(home_dir) + "/" + get_hostname() + ".dat", std::ios::trunc);
		if (!gnuplot_file.is_open()) {
			std::cerr << "Could not create file (" << std::string(home_dir) + "/" + get_hostname() + ".dat"
					  << ") to store benchmark data." << std::endl;
		}
	}

	distgend_configT config;
	std::string gnuplot_data;

	for (unsigned char i = 0; i < distgen_init.number_of_threads / distgen_init.SMT_factor; ++i) {
		config.number_of_threads = i + 1;
		config.threads_to_use[i] = i;
		std::cout << "Using " << i + 1 << " threads:" << std::endl;
		std::cout << "\tMaximum: " << distgend_get_max_bandwidth(config) << " GByte/s" << std::endl;
		std::cout << std::endl;

		gnuplot_data += std::to_string(i + 1) + " ";
		gnuplot_data += std::to_string(distgend_get_max_bandwidth(config)) + "\n";
	}

	std::string gnuplot_command =
		"echo \"" + gnuplot_data + "\"| gnuplot -e \"set terminal dumb; set ytics nomirror; set xtics 1,1," +
		std::to_string(distgen_init.number_of_threads) + " nomirror; set yrange [0:]; set border 3; set "
														 "xlabel 'threads'; set ylabel 'BW'; plot '-' "
														 "with lines notitle;\"";

	int err = system(gnuplot_command.c_str());
	if (err != 0) {
		std::cout << "Could not generate plot. Feel free to execute " << std::endl;
		std::cout << gnuplot_command << std::endl;
		std::cout << "on a system with gnuplot installed.";
	}

	if (home_dir_available && gnuplot_file.is_open()) {
		gnuplot_file << gnuplot_data;
		gnuplot_file.close();
	}
}

[[noreturn]] static void bench_thread(fast::MQTT_communicator &comm) {
	while (true) {
		fast::msg::agent::mmbwmon::request req;
		auto m = comm.get_message();
		std::cout << "Got message:\n" << m << "\n";
		req.from_string(m);

		std::cout << "Running bench on cores ";

		distgend_configT dc;
		dc.number_of_threads = req.cores.size();
		for (size_t i = 0; i < req.cores.size(); ++i) {
			dc.threads_to_use[i] = static_cast<unsigned char>(req.cores[i]);
			std::cout << static_cast<int>(dc.threads_to_use[i]) << ", ";
		}

		std::cout << "\n";

		const double mem = distgend_is_membound(dc);

		std::cout << "Result: " << mem << std::endl;

		fast::msg::agent::mmbwmon::reply reply(req.cores, mem);
		std::cout << "Sending message:\n" << reply.to_string() << "\n";
		comm.send_message(reply.to_string(), baseTopic + "/response");
	}
}

#ifdef CGROUP_SUPPORT
[[noreturn]] static void stop_thread(fast::MQTT_communicator &comm) {
	comm.add_subscription(baseTopic + "/stop");
	while (true) {
		fast::msg::agent::mmbwmon::stop req;
		auto m = comm.get_message(baseTopic + "/stop");
		std::cout << "Got message:\n" << m << "\n";
		req.from_string(m);

		cgroup_freeze(req.cgroup);

		fast::msg::agent::mmbwmon::ack a;
		comm.send_message(a.to_string(), baseTopic + "/stop/ack");
	}
}

[[noreturn]] static void restart_thread(fast::MQTT_communicator &comm) {
	comm.add_subscription(baseTopic + "/restart");
	while (true) {
		fast::msg::agent::mmbwmon::restart req;
		auto m = comm.get_message(baseTopic + "/restart");
		std::cout << "Got message:\n" << m << "\n";
		req.from_string(m);

		cgroup_thaw(req.cgroup);

		fast::msg::agent::mmbwmon::ack a;
		comm.send_message(a.to_string(), baseTopic + "/restart/ack");
	}
}
#endif

int main(int argc, char const *argv[]) {
	const std::string agentID = "fast/agent/" + get_hostname() + "/mmbwmon";

	const int err = system((std::string("mkdir -p ") + home_dir).c_str());
	if (err == 0) {
		home_dir_available = true;
	} else {
		home_dir_available = false;
		std::cerr << "Could not create " << home_dir << ". Not saving any measurements.";
	}

	parse_options(static_cast<size_t>(argc), argv);

	std::cout << "Starting distgen initialization ...";
	std::cout.flush();
	distgend_init(distgen_init);

	std::cout << " done!\n\n";

	print_distgen_results(distgen_init);

	if (measure_only) return 0;

	fast::MQTT_communicator comm(agentID, baseTopic + "/request", baseTopic + "/response", server,
								 static_cast<int>(port), 60);

	std::cout << "MQTT ready!\n\n";

	std::thread bench(bench_thread, std::ref(comm));
#ifdef CGROUP_SUPPORT
	std::thread restart(restart_thread, std::ref(comm));
	std::thread stop(stop_thread, std::ref(comm));
#endif

	bench.join();
#ifdef CGROUP_SUPPORT
	restart.join();
	stop.join();
#endif
}
