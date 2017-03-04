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

#define DISTGEN_MAXTHREADS 255

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
 * This function initializes the daemon with previous benchmark results.
 */
void distgend_init_without_bench(distgend_initT init, const double *const membw);

/**
 * Return a values in the range of ~[0-1] with
 * - ~1   == no load on the memory system and
 * - ~0.3 == memory system fully utilized
 */
double distgend_is_membound(distgend_configT config);

/**
 * Scales a value returned by distgend_is_membound
 * - ~1   == no load on the memory system and
 * - ~0 == memory system fully utilized
 */
double distgend_scale(distgend_configT config, double input);

/**
 * Identical to distgend_scale(distgend_is_membound)
 */
double distgend_is_membound_scaled(distgend_configT config);

/**
 * Returns the GB/s expected for the giving config if the system is idle.
 */
double distgend_get_max_bandwidth(distgend_configT config);

/**
 * Returns the GB/s measured for core_num cores. Uses compact pinning on cores, not HTCs.
 */
double distgend_get_measured_idle_bandwidth(size_t core_num);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: distgend_h */
