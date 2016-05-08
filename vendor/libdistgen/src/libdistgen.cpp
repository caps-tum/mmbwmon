/**
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 * Josef Weidendorfer <weidendo@in.tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#include "distgen/distgen.h"
#include "distgen/distgen_internal.h"

#define _GNU_SOURCE
//#define __USE_GNU

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <omp.h>

#include "ponci/ponci.hpp"

// TODO We currently allocate the buffers once, should we change this?

// GByte/s measured for i cores is stored in [i-1]
static double distgen_mem_bw_results[DISTGEN_MAXTHREADS];

// the configuration of the system
static distgend_initT system_config;

// Prototypes
static void set_affinity(distgend_initT init);
static double bench(distgend_configT config);

void distgend_init(distgend_initT init) {
	assert(init.number_of_threads < DISTGEN_MAXTHREADS);
	assert(init.NUMA_domains < init.number_of_threads);
	assert((init.number_of_threads % init.NUMA_domains) == 0);
	assert(init.number_of_threads % (init.NUMA_domains * init.SMT_factor) == 0);

	system_config = init;

	// we currently measure maximum read bandwidth
	pseudoRandom = 0;
	depChain = 0;
	doWrite = 0;

	// number of iterations. currently a magic number
	iter = 1000;

	// set a size of 50 MB
	// TODO we should compute this based on L3 size
	addDist(50000000);

	// set the number of threads to the maximum available in the system
	tcount = init.number_of_threads;
	omp_set_num_threads(tcount);

	set_affinity(init);

	initBufs();

	// fill distgen_mem_bw_results
	distgend_configT config;
	for (unsigned char i = 0; i < init.number_of_threads / init.NUMA_domains; ++i) {
		config.number_of_threads = i + 1;
		config.threads_to_use[i] = i;

		distgen_mem_bw_results[i] = bench(config);
	}
}

double distgend_get_max_bandwidth(distgend_configT config) {
	assert(config.number_of_threads > 0);

	const size_t phys_cores_per_numa =
		system_config.number_of_threads / (system_config.NUMA_domains * system_config.SMT_factor);

	double res = 0.0;

	size_t cores_per_numa_domain[DISTGEN_MAXTHREADS];
	for (size_t i = 0; i < system_config.NUMA_domains; ++i) cores_per_numa_domain[i] = 0;

	// for every NUMA domain we use
	// -> count the cores used
	// TODO how to handle multiple HTs for one core?
	for (size_t i = 0; i < config.number_of_threads; ++i) {
		size_t t = config.threads_to_use[i];
		size_t n = (t / phys_cores_per_numa) % system_config.NUMA_domains;
		++cores_per_numa_domain[n];
	}

	for (size_t i = 0; i < system_config.NUMA_domains; ++i) {
		size_t temp = cores_per_numa_domain[i];
		if (temp > 0) res += distgen_mem_bw_results[temp - 1];
	}

	return res;
}

double distgend_is_membound(distgend_configT config) {
	// run benchmark on given cores
	// compare the result with distgend_get_max_bandwidth();
	const double m = bench(config);
	const double c = distgend_get_max_bandwidth(config);
	const double res = m / c;

	return (res > 1.0) ? 1.0 : res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////

static double bench(distgend_configT config) {
	double ret = 0.0;

#pragma omp parallel reduction(+ : ret)
	{
		size_t tid = (size_t)omp_get_thread_num();
		for (size_t i = 0; i < config.number_of_threads; ++i) {
			if (tid == config.threads_to_use[i]) {
				double tsum = 0.0;
				u64 taCount = 0;

				const double t1 = wtime();
				runBench(buffer[omp_get_thread_num()], iter, depChain, doWrite, &tsum, &taCount);
				const double t2 = wtime();

				const double temp = taCount * 64.0 / 1024.0 / 1024.0 / 1024.0;
				ret += temp / (t2 - t1);
			}
		}
	}

	return ret;
}

static void set_affinity(distgend_initT init) {
	cgroup_create(DISTGEN_CGROUP_NAME);

	size_t *arr = (size_t *)malloc(sizeof(size_t) * init.number_of_threads);

	const size_t phys_cores_per_numa = init.number_of_threads / (init.NUMA_domains * init.SMT_factor);

	size_t i = 0;

	for (size_t n = 0; n < init.NUMA_domains; ++n) {
		size_t next_core = n * phys_cores_per_numa;
		for (size_t s = 0; s < init.SMT_factor; ++s) {
			for (size_t c = 0; c < phys_cores_per_numa; ++c) {
				arr[i] = next_core;
				++next_core;
				++i;
			}
			next_core += phys_cores_per_numa * (init.NUMA_domains - 1);
		}
	}
	assert(i == init.number_of_threads);

	cgroup_set_cpus(DISTGEN_CGROUP_NAME, arr, init.number_of_threads);

	for (size_t n = 0; n < init.NUMA_domains; ++n) arr[n] = n;
	cgroup_set_mems(DISTGEN_CGROUP_NAME, arr, init.NUMA_domains);

	cgroup_add_me(DISTGEN_CGROUP_NAME);
}
