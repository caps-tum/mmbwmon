/**
 * po     n  r       i
 * poor mans resctrl interface
 *
 * There are just some C++ convenience functions in this file. Please read
 *           ponri.h
 * for a detailed functionality overview.
 *
 * Copyright 2016 by Jens Breitbart
 * Jens Breitbart     <jbreitbart@gmail.com>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef ponri_hpp
#define ponri_hpp

#include <bitset>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "ponri.h"

#ifdef __cplusplus
} /* end extern "C" */

/* Start of the C++ only functions. */
// TODO add enum parameter to select L2 or L3
inline std::bitset<64> get_cbm_mask() { return std::bitset<64>(get_cbm_mask_as_uint()); }

inline void resgroup_create(const std::string &name) { resgroup_create(name.c_str()); }
inline void resgroup_delete(const std::string &name) { resgroup_delete(name.c_str()); }
inline void resgroup_add_me(const std::string &name) { resgroup_add_me(name.c_str()); }
inline void resgroup_add_task(const std::string &name, const pid_t tid) { resgroup_add_task(name.c_str(), tid); }

void resgroup_set_cpus(const std::string &name, const std::vector<size_t> &cpus);
void resgroup_set_schemata(const std::string &name, const std::vector<size_t> &schematas);

#endif /* end of the c++ only functions */

#endif /* end of include guard: ponri_hpp */
