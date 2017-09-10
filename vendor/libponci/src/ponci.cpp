/**
 * po     n  c       i
 * poor mans cgroups interface
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#include "ponci/ponci.hpp"

#include "fileIO_helper.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>

// placeholder to be substituted by the names of a subsystem
static const auto &SUBSYSTEM_PLACEHOLDER = *new std::string("%SUBSYSTEM%");

// list of subsystems, updated in constructor
static std::vector<std::string> *subsystems;

/////////////////////////////////////////////////////////////////
// PROTOTYPES
/////////////////////////////////////////////////////////////////
static inline std::string cgroup_path(const char *name);

static std::vector<int> get_tids_from_pid(int pid);

static bool check_is_systemd();
static void replace_subsystem_in_path(std::string &str, const std::string &to);

static void _constructor() __attribute__((constructor));

/////////////////////////////////////////////////////////////////
// EXPORTED FUNCTIONS
/////////////////////////////////////////////////////////////////
void cgroup_create(const char *name) {
	const auto cgp = cgroup_path(name);
	for (const auto &sub : *subsystems) {
		auto temp = cgp;
		replace_subsystem_in_path(temp, sub);
		const int err = mkdir(temp.c_str(), S_IRWXU | S_IRWXG);

		if (err != 0 && errno != EEXIST) throw std::runtime_error(strerror(errno));
	}

	errno = 0;
}

void cgroup_delete(const char *name) {
	const auto cgp = cgroup_path(name);
	for (const auto &sub : *subsystems) {
		auto temp = cgp;
		replace_subsystem_in_path(temp, sub);
		const int err = rmdir(temp.c_str());

		if (err != 0) throw std::runtime_error(strerror(errno));
	}
}

void cgroup_add_me(const char *name) {
	auto me = static_cast<pid_t>(syscall(SYS_gettid));
	cgroup_add_task(name, me);
}

void cgroup_add_task(const char *name, const pid_t tid) {
	const auto cgp = cgroup_path(name);
	for (const auto &sub : *subsystems) {
		auto temp = cgp;
		replace_subsystem_in_path(temp, sub);

		temp += std::string("tasks");
		append_value_to_file(temp, tid);
	}
}

void cgroup_set_cpus(const char *name, const size_t *cpus, size_t size) {
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.cpus");

	write_array_to_file(filename, cpus, size);
}

void cgroup_set_cpus(const std::string &name, const std::vector<unsigned char> &cpus) {
	auto cgp = cgroup_path(name.c_str());
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.cpus");

	write_vector_to_file(filename, cpus);
}

void cgroup_set_mems(const char *name, const size_t *mems, size_t size) {
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.mems");

	write_array_to_file(filename, mems, size);
}

void cgroup_set_mems(const std::string &name, const std::vector<unsigned char> &mems) {
	auto cgp = cgroup_path(name.c_str());
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.mems");

	write_vector_to_file(filename, mems);
}

void cgroup_set_memory_migrate(const char *name, size_t flag) {
	assert(flag == 0 || flag == 1);

	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.memory_migrate");

	write_value_to_file(filename, flag);
}

void cgroup_set_cpus_exclusive(const char *name, size_t flag) {
	assert(flag == 0 || flag == 1);

	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.cpu_exclusive");

	write_value_to_file(filename, flag);
}

void cgroup_set_mem_hardwall(const char *name, size_t flag) {
	assert(flag == 0 || flag == 1);
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.mem_hardwall");

	write_value_to_file(filename, flag);
}

void cgroup_set_scheduling_domain(const char *name, int flag) {
	assert(flag >= -1 && flag <= 5);
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");
	std::string filename = cgp + std::string("cpuset.sched_relax_domain_level");

	write_value_to_file(filename, flag);
}

void cgroup_freeze(const char *name) {
	// never freeze top level cgroup
	assert(strcmp(name, "") != 0);

	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "freezer");
	std::string filename = cgp + std::string("freezer.state");

	write_value_to_file(filename, "FROZEN");
}

void cgroup_thaw(const char *name) {
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "freezer");
	std::string filename = cgp + std::string("freezer.state");

	write_value_to_file(filename, "THAWED");
}

void cgroup_wait_frozen(const char *name) {
	// never freeze top level cgroup
	assert(strcmp(name, "") != 0);

	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "freezer");
	std::string filename = cgp + std::string("freezer.state");

	std::string temp;
	while (temp != "FROZEN\n") {
		temp = read_line_from_file(filename);
	}
}

void cgroup_wait_thawed(const char *name) {
	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "freezer");
	std::string filename = cgp + std::string("freezer.state");

	std::string temp;
	while (temp != "THAWED\n") {
		temp = read_line_from_file(filename);
	}
}

void cgroup_kill(const char *name) {
	auto tids = get_tids_from_pid(getpid());

	auto cgp = cgroup_path(name);
	replace_subsystem_in_path(cgp, "cpuset");

	// get all pids
	std::vector<__pid_t> pids = read_lines_from_file<__pid_t>(cgp + std::string("tasks"));

	// send kill
	for (__pid_t pid : pids) {
		if (std::find(tids.begin(), tids.end(), pid) != tids.end()) {
			// pid in tids -> we should not kill ourself :)
		} else {
			// pid not in tids
			if (kill(pid, SIGTERM) != 0) {
				throw std::runtime_error(strerror(errno));
			}
		}
	}

	// wait until tasks empty
	while (!pids.empty()) {
		pids = read_lines_from_file<int>(cgp + std::string("tasks"));
	}

	cgroup_delete(name);
}

/////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
/////////////////////////////////////////////////////////////////

static inline std::string cgroup_path(const char *name) {
	static const char *env = std::getenv("PONCI_PATH");

	std::string res(env != nullptr ? std::string(env) : std::string("/sys/fs/cgroup/"));

	res.append(SUBSYSTEM_PLACEHOLDER);
	res.append("/");

	if (strcmp(name, "") != 0) {
		res.append(name);
		res.append("/");
	}

	return res;
}

#if 0
template <typename T> static inline void write_vector_to_file(const std::string &filename, const std::vector<T> &vec) {
	write_array_to_file(filename, &vec[0], vec.size());
}

template <typename T> static inline void write_array_to_file(const std::string &filename, T *arr, size_t size) {
	assert(size > 0);
	assert(filename != "");

	std::string str;
	for (size_t i = 0; i < size; ++i) {
		str.append(std::to_string(arr[i]));
		str.append(",");
	}

	write_value_to_file(filename, str.c_str());
}

template <typename T> static inline void append_value_to_file(const std::string &filename, T val) {
	assert(filename != "");

	FILE *f = fopen(filename.c_str(), "a+");
	if (f == nullptr) {
		throw std::runtime_error(strerror(errno));
	}
	std::string str = std::to_string(val);

	if (fputs(str.c_str(), f) == EOF && ferror(f) != 0) {
		// try to close the file, but return the old error
		auto err = errno;
		fclose(f);

		throw std::runtime_error(strerror(err));
	}
	if (fclose(f) != 0) {
		throw std::runtime_error(strerror(errno));
	}
}

template <typename T> static inline void write_value_to_file(const std::string &filename, T val) {
	write_value_to_file(filename, std::to_string(val).c_str());
}

template <> void write_value_to_file<const char *>(const std::string &filename, const char *val) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "w+");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	int status = fputs(val, file);
	if (status <= 0 || ferror(file) != 0) {
		// try to close the file, but return the old error
		auto err = errno;
		fclose(file);

		throw std::runtime_error(strerror(err));
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}
}

static inline std::string read_line_from_file(const std::string &filename) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "r");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	char temp[buf_size];
	if (fgets(temp, buf_size, file) == nullptr && (feof(file) == 0)) {
		// something is wrong. let's try to close the file and go home
		fclose(file);
		throw std::runtime_error("Error while reading file in libponci. Buffer to small?");
	}

	if (feof(file) != 0) {
		memset(temp, 0, buf_size);
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return std::string(temp);
}

template <typename T> static inline std::vector<T> read_lines_from_file(const std::string &filename) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "r");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	std::vector<T> ret;

	char temp[buf_size];
	while (true) {
		if (fgets(temp, buf_size, file) == nullptr && !feof(file)) {
			// something is wrong. let's try to close the file and go home
			fclose(file);
			throw std::runtime_error("Error while reading file in libponci. Buffer to small?");
		}
		if (feof(file)) break;
		std::size_t done = 0;
		T i = string_to_T<T>(std::string(temp), done);
		if (done != 0) ret.push_back(i);
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return ret;
}

template <> int string_to_T<int>(const std::string &s, std::size_t &done) { return stoi(s, &done); }
#endif

#if 0
template <> unsigned long string_to_T<unsigned long>(const std::string &s, std::size_t &done) {
	return stoul(s, &done);
}
#endif

static std::vector<int> get_tids_from_pid(const int pid) {
	std::string path("/proc/" + std::to_string(pid) + "/task/");
	dirent *dent;
	DIR *srcdir = opendir(path.c_str());

	if (srcdir == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	std::vector<int> ret;

	while ((dent = readdir(srcdir)) != nullptr) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) continue;

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			std::size_t i;
			ret.push_back(string_to_T<int>(std::string(dent->d_name), i));
		}
	}
	closedir(srcdir);
	return ret;
}

static void replace_subsystem_in_path(std::string &str, const std::string &to) {
	size_t start_pos = str.find(SUBSYSTEM_PLACEHOLDER);
	assert(start_pos != std::string::npos);
	str.replace(start_pos, SUBSYSTEM_PLACEHOLDER.length(), to);
}

// check if the system is using systemd
static bool check_is_systemd() {
	bool ret = false;

	// read /etc/mtab and check if it contains "cgroup /sys/fs/cgroup/systemd"
	FILE *file = fopen("/etc/mtab", "r");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	char temp[buf_size];
	while (true) {
		if (fgets(temp, buf_size, file) == nullptr && (feof(file) == 0)) {
			// something is wrong. let's try to close the file and go home
			fclose(file);
			throw std::runtime_error("Error while reading file in libponci. Buffer to small?");
		}
		if (feof(file) != 0) break;
		if (std::string(temp).find("cgroup /sys/fs/cgroup/systemd") != std::string::npos) {
			ret = true;
			break;
		}
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return ret;
}

void _constructor() {
	bool is_systemd = check_is_systemd();

	subsystems = new std::vector<std::string>;
	if (is_systemd) {
		subsystems->emplace_back("cpuset");
		subsystems->emplace_back("freezer");
	} else {
		subsystems->emplace_back("");
	}
}
