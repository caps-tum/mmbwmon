#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <cstring>

// for perf support
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <syscall.h>
#include <unistd.h>

#include <distgen/distgen.h>
#include <ponri/ponri.hpp>

const std::string res_name("opticat");

/*** config vars **/
static distgend_initT distgen_init;

[[noreturn]] static void print_help(const char *argv) {
	std::cout << argv << " supports the following flags:\n";
	std::cout << "\t --numa \t Number of NUMA domains. \t\t\t Default: 2\n";
	std::cout << "\t --threads \t Number of logical cores. \t\t\t Default: " << std::thread::hardware_concurrency()
			  << "\n";
	std::cout << "\t --smt \t\t Number of logical cores per physical core. \t Default: 2\n";
	exit(0);
}

static void parse_options(size_t argc, const char **argv) {
	// if (argc == 1) {
	//		print_help(argv[0]);
	//}

	distgen_init.number_of_threads = std::thread::hardware_concurrency();
	distgen_init.NUMA_domains = 2;
	distgen_init.SMT_factor = 2;

	for (size_t i = 1; i < argc; ++i) {
		std::string arg(argv[i]);

		if (arg == "--help" || arg == "-h") {
			print_help(argv[0]);
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
}

static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid, size_t cpu, int group_fd, unsigned long flags) {
	return static_cast<int>(syscall(__NR_perf_event_open, hw_event, pid, static_cast<int>(cpu), group_fd, flags));
}

// open event per core for every core
static std::vector<int> init_perf() {

	std::vector<int> fds;
	fds.reserve(distgen_init.number_of_threads);

	perf_event_attr event;
	memset(&event, 0, sizeof(perf_event_attr));

	event.type = PERF_TYPE_HARDWARE;
	event.size = sizeof(struct perf_event_attr);
	event.config = PERF_COUNT_HW_CACHE_MISSES;
	event.disabled = 1;
	event.exclude_kernel = 1;
	event.exclude_hv = 1;

	// for each CPU:
	// create a systemwide monitor restricted to a set of CPUs
	for (size_t i = 0; i < distgen_init.number_of_threads; ++i) {
		auto fd = perf_event_open(&event, -1, i, -1, 0);
		if (fd == -1) {
			std::cerr << "Could not create perf event for core " << i
					  << ". Please set /proc/sys/kernel/perf_event_paranoid to -1." << std::endl;
			return std::vector<int>();
		}
		fds.push_back(fd);
	}

	return fds;
}

static void start_perf_measurement(const std::vector<int> &fds) {
	for (auto fd : fds) {
		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
	}
}

static void stop_perf_measurement(const std::vector<int> &fds) {
	for (auto fd : fds) {
		ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
	}
}

static std::vector<long long> read_perf_measurement(const std::vector<int> &fds) {
	std::vector<long long> counts;
	for (size_t i = 0; i < fds.size(); ++i) {
		auto fd = fds[i];
		long long count;
		auto temp = read(fd, &count, sizeof(long long));
		if (temp == -1) {
			std::cerr << "Could not read from perf event for core " << i << std::endl;
		}
		counts.push_back(count);
	}
	return counts;
}

int main(int argc, char const *argv[]) {
	parse_options(static_cast<size_t>(argc), argv);

	std::cout << "Starting distgen initialization ...";
	std::cout.flush();
	distgend_init(distgen_init);
	std::cout << " done!\n\n";

	std::cout << "CBM max: " << std::hex << get_cbm_mask_as_uint() << std::endl;
	std::cout << "CBM min bits: " << get_min_cbm_bits() << std::endl;
	std::cout << "Number of closids: " << get_num_closids() << std::endl;

	auto perf_ids = init_perf();

	resgroup_create(res_name);
	{
		std::vector<size_t> cpus;
		for (size_t i = 0; i < distgen_init.number_of_threads; ++i) {
			cpus.push_back(i);
		}
		resgroup_set_cpus(res_name, cpus);
	}

	// loop over all possible settings
	auto bits = get_cbm_mask();

	std::cout << "Mask"
			  << " - "
			  << "cache misses" << std::endl;

	while (bits.count() >= get_min_cbm_bits()) {
		std::vector<size_t> schematas;
		for (size_t n = 0; n < distgen_init.NUMA_domains; ++n) schematas.push_back(bits.to_ullong());
		resgroup_set_schemata(res_name, schematas);

		start_perf_measurement(perf_ids);

		// TODO start real application, read command from file?

		// run distgen to evict everything from the cache
		distgend_configT dg;
		dg.number_of_threads = distgen_init.number_of_threads;
		for (size_t i = 0; i < distgen_init.number_of_threads; ++i) dg.threads_to_use[i] = i;
		distgend_is_membound(dg);

		// TODO move stop measurement before distgen call
		stop_perf_measurement(perf_ids);
		auto res = read_perf_measurement(perf_ids);

		double sum = 0;
		for (size_t i = 0; i < res.size(); ++i) {
			sum += res[i];
		}

		std::cout << std::hex << bits.to_ullong() << " - " << std::dec << sum << std::endl;

		// reset one bit
		for (size_t i = 0; i < bits.size(); ++i) {
			if (bits[i]) {
				bits[i] = false;
				break;
			}
		}
	}

	resgroup_delete(res_name);
}
