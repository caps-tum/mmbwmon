#include <ponri/ponri.hpp>

#include "fileIO_helper.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>
#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>

static inline std::string resgroup_path(const char *name);

void resgroup_create(const char *name) {
	const auto rgp = resgroup_path(name);
	const int err = mkdir(rgp.c_str(), S_IRWXU | S_IRWXG);

	if (err != 0 && errno != EEXIST) throw std::runtime_error(strerror(errno));

	errno = 0;
}

void resgroup_delete(const char *name) {
	const auto rgp = resgroup_path(name);
	const int err = rmdir(rgp.c_str());

	if (err != 0) throw std::runtime_error(strerror(errno));
}

void resgroup_add_me(const char *name) {
	auto me = static_cast<pid_t>(syscall(SYS_gettid));
	resgroup_add_task(name, me);
}

void resgroup_add_task(const char *name, const pid_t tid) {
	auto cgp = resgroup_path(name);
	cgp += std::string("tasks");
	append_value_to_file(cgp, tid);
}

void resgroup_set_cpus(const char *name, const size_t *cpus, size_t size) {
	assert(size < 64);

	auto cgp = resgroup_path(name);
	std::string filename = cgp + std::string("cpus");

	// TODO magic number
	std::bitset<64> bits;
	for (size_t i = 0; i < size; ++i) {
		bits.set(cpus[i]);
	}

	write_bitset_to_file(filename, bits);
}

void resgroup_set_cpus(const std::string &name, const std::vector<size_t> &cpus) {
	resgroup_set_cpus(name.c_str(), &cpus[0], cpus.size());
}

// TODO support L2
void resgroup_set_schemata(const char *name, const size_t *schematas, size_t size) {
	/*
	 $ cat /sys/fs/resctrl/a/schemata
	 L3:0=fffff;1=fffff
	 */
	auto cgp = resgroup_path(name);
	std::string filename = cgp + std::string("schemata");

	std::string content = "L3:";
	for (size_t i = 0; i < size; ++i) {
		content += std::to_string(i) + "=";

		std::stringstream stream;
		stream << std::hex << schematas[i];
		content += stream.str();

		if (i + 1 != size) content += ";";
	}

	write_value_to_file(filename, content);
}

void resgroup_set_schemata(const std::string &name, const std::vector<size_t> &schematas) {
	resgroup_set_schemata(name.c_str(), &schematas[0], schematas.size());
}

// TODO add enum parameter to select L2 or L3
std::uint64_t get_cbm_mask_as_uint() {
	const std::string filename = resgroup_path("info/L3/") + "cbm_mask";
	const auto line = read_line_from_file(filename);

	std::stringstream converter(line);
	std::uint64_t mask_as_uint;
	converter >> std::hex >> mask_as_uint;

	return mask_as_uint;
}

// TODO add enum parameter to select L2 or L3
unsigned int get_min_cbm_bits() {
	const std::string filename = resgroup_path("info/L3/") + "min_cbm_bits";
	const auto line = read_line_from_file(filename);

	return static_cast<unsigned int>(std::stoi(line));
}

// TODO add enum parameter to select L2 or L3
unsigned int get_num_closids() {
	const std::string filename = resgroup_path("info/L3/") + "num_closids";
	const auto line = read_line_from_file(filename);

	return static_cast<unsigned int>(std::stoi(line));
}

/////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
/////////////////////////////////////////////////////////////////

static inline std::string resgroup_path(const char *name) {
	static const char *env = std::getenv("PONRI_PATH");

	std::string res(env != nullptr ? std::string(env) : std::string("/sys/fs/resctrl"));
	res.append("/");

	if (strcmp(name, "") != 0) {
		res.append(name);
		res.append("/");
	}

	return res;
}
