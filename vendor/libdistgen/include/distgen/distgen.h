/**
 * Distgen daemon header file
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 * Josef Weidendorfer <weidendo@in.tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef distgend_h
#define distgend_h

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif

#define DISTGEN_MAXTHREADS 64
#define DISTGEN_CGROUP_NAME "distgend"

typedef struct {
	size_t number_of_threads;
	size_t NUMA_domains;
	size_t SMT_factor;
} distgend_initT;

typedef struct {
	size_t number_of_threads;
	unsigned char threads_to_use[DISTGEN_MAXTHREADS];
} distgend_configT;

/**
 * This function initializes the daemon.
 * Must be called while the system is idle, as it runs various
 * benchmarks.
 */
void distgend_init(distgend_initT init);

/**
 * Return a values in the range of ~[0-1] with
 * - ~1   == no load on the memory system and
 * - ~0.3 == memory system fully utilized
 */
double distgend_is_membound(distgend_configT config);

/**
 * Returns the GB/s expected for the giving config if the system is idle.
 */
double distgend_get_max_bandwidth(distgend_configT config);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: distgend_h */
