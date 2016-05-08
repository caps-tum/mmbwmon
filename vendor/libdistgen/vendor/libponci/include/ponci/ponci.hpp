/**
 * po     n  c       i
 * poor mans cgroups interface
 *
 * There are just some C++ convenience functions in this file. Please read
 *           ponci.h
 * for a detailed functionality overview.
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef ponci_hpp
#define ponci_hpp

#ifdef __cplusplus
extern "C" {
#endif

#include "ponci.h"

/* Start of the C++ only functions. */
#ifdef __cplusplus
} /* end extern "C" */

#include <string>
#include <vector>

inline void cgroup_create(const std::string &name) { cgroup_create(name.c_str()); }

inline void cgroup_delete(const std::string &name) { cgroup_delete(name.c_str()); }

inline void cgroup_add_me(const std::string &name) { cgroup_add_me(name.c_str()); }

inline void cgroup_add_task(const std::string &name, const pid_t pid) { cgroup_add_task(name.c_str(), pid); }

inline void cgroup_set_cpus(const std::string &name, const size_t *cpus, size_t size) {
	cgroup_set_cpus(name.c_str(), cpus, size);
}

inline void cgroup_set_cpus(const std::string &name, const std::vector<size_t> &cpus) {
	cgroup_set_cpus(name.c_str(), &cpus[0], cpus.size());
}

void cgroup_set_cpus(const std::string &name, const std::vector<unsigned char> &cpus);

inline void cgroup_set_mems(const std::string &name, const size_t *mems, size_t size) {
	cgroup_set_mems(name.c_str(), mems, size);
}

inline void cgroup_set_mems(const std::string &name, const std::vector<size_t> &mems) {
	cgroup_set_mems(name.c_str(), &mems[0], mems.size());
}

void cgroup_set_mems(const std::string &name, const std::vector<unsigned char> &mems);

inline void cgroup_set_memory_migrate(const std::string &name, size_t flag) {
	cgroup_set_memory_migrate(name.c_str(), flag);
}

inline void cgroup_set_cpus_exclusive(const std::string &name, size_t flag) {
	cgroup_set_cpus_exclusive(name.c_str(), flag);
}

inline void cgroup_set_mem_hardwall(const std::string &name, size_t flag) {
	cgroup_set_mem_hardwall(name.c_str(), flag);
}

inline void cgroup_set_scheduling_domain(const std::string &name, int flag) {
	cgroup_set_scheduling_domain(name.c_str(), flag);
}

inline void cgroup_freeze(const std::string &name) { cgroup_freeze(name.c_str()); }
inline void cgroup_thaw(const std::string &name) { cgroup_thaw(name.c_str()); }
inline void cgroup_wait_thawed(const std::string &name) { cgroup_wait_thawed(name.c_str()); }
inline void cgroup_wait_frozen(const std::string &name) { cgroup_wait_frozen(name.c_str()); }

inline void cgroup_kill(const std::string &name) { cgroup_kill(name.c_str()); }

#endif /* end of the c++ only functions */

#endif /* end of include guard: ponci_hpp */
