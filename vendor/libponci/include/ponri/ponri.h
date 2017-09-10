/**
 * po     n  r       i
 * poor mans resctrl interface
 *
 * Copyright 2016 by Jens Breitbart
 * Jens Breitbart     <jbreitbart@gmail.com>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifdef __cplusplus
#ifndef ponri_hpp
#error "Include ponri.hpp when compiling for C++!"
#endif
#endif

#ifndef ponri_h
#define ponri_h

#include <inttypes.h>
#include <sys/types.h>

/**
 * Creates a ressource group
 */
void resgroup_create(const char *name);

/**
 * Deletes a ressource group
 */
void resgroup_delete(const char *name);

/**
 * Adds the calling thread to the ressource group
 */
void resgroup_add_me(const char *name);

/**
 * Adss a given thread to the ressource group
 */
void resgroup_add_task(const char *name, pid_t tid);

/**
 * Sets the CPU mask of a ressource group
 */
void resgroup_set_cpus(const char *name, const size_t *cpus, size_t size);

/**
 * Sets the schema for the ressource group. One schemata per NUMA domain
 */
void resgroup_set_schemata(const char *name, const size_t *schematas, size_t size);

/**
 * Returns the maximum bit mask available.
 */
uint64_t get_cbm_mask_as_uint();

/**
 * Returns the minimum number of consecutive bits that must be set.
 */
unsigned int get_min_cbm_bits();

/**
 * Returns the number of unique COS configurations available.
 */
unsigned int get_num_closids();

#endif /* end of include guard: ponri_h */
