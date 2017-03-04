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

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

// TODO We currently allocate the buffers once, should we change this?

// GByte/s measured for i cores is stored in [i-1]
static double distgen_mem_bw_results[DISTGEN_MAXTHREADS];

// threads, barrier, and attributes
static pthread_t threads[DISTGEN_MAXTHREADS];
static pthread_barrier_t barrier;
static pthread_attr_t thread_attr[DISTGEN_MAXTHREADS];

// the configuration of the system
static distgend_initT system_config;

// arguments to thread functions
typedef struct thread_args {
	distgend_configT *config;
	size_t tid;
} thread_argsT;

// Prototypes
static void set_affinity(distgend_initT init);
static double bench(distgend_configT config);
static void internal_init(distgend_initT init);

static void internal_init(distgend_initT init) {
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

	set_affinity(init);

	initBufs();
}

void distgend_init(distgend_initT init) {
	internal_init(init);

	// fill distgen_mem_bw_results
	distgend_configT config;
	for (unsigned char i = 0; i < init.number_of_threads / init.SMT_factor; ++i) {
		config.number_of_threads = i + 1;
		config.threads_to_use[i] = i;

		distgen_mem_bw_results[i] = bench(config);
	}
}

void distgend_init_without_bench(distgend_initT init, const double *const membw) {
	internal_init(init);
	for (unsigned char i = 0; i < init.number_of_threads / init.SMT_factor; ++i) {
		// TODO no range check.
		distgen_mem_bw_results[i] = membw[i];
	}
}

double distgend_get_max_bandwidth(distgend_configT config) {
	assert(config.number_of_threads > 0);

	const size_t phys_cores_per_numa =
		system_config.number_of_threads / (system_config.NUMA_domains * system_config.SMT_factor);

	double res = 0.0;

	size_t cores_per_numa_domain[DISTGEN_MAXTHREADS];
	assert(system_config.NUMA_domains < DISTGEN_MAXTHREADS);
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

double distgend_get_measured_idle_bandwidth(size_t core_num) {
	assert(core_num - 1 < system_config.number_of_threads / system_config.SMT_factor);
	return distgen_mem_bw_results[core_num - 1];
}

double distgend_is_membound(distgend_configT config) {
	// run benchmark on given cores
	// compare the result with distgend_get_max_bandwidth();
	const double m = bench(config);
	const double c = distgend_get_max_bandwidth(config);
	const double res = m / c;

	return (res > 1.0) ? 1.0 : res;
}

double distgend_scale(distgend_configT config, double input) {
	// there is no need to scale the value if all cores have been used to run distgen
	if (config.number_of_threads == system_config.number_of_threads) return input;

	// we scale the value based on the following idea:
	// distgen only reads from memory during measurements.
	// the other application typically uses read + write from memory
	// the hardware treats reads and write entries in the memory controller
	// queue identical
	// in use : measured => minimum value returned by distgen_is_membound
	// 1 : 1 => 1 / (2 + 1) => 0.33 minimum
	// 1 : 2 => 2*1 / (2 + 2*1) => 0.55
	// 2 : 1 => 1 / (2*2 +1) => 0.2
	const size_t in_use = system_config.number_of_threads / system_config.SMT_factor - config.number_of_threads;
	const double min = static_cast<double>(config.number_of_threads) / (2.0 * in_use + config.number_of_threads);
	const double max = 1;

	if (input > 1.0) input = 1.0;
	if (input < min) input = min;

	//        (1 - 0)(x - min)
	// f(x) = ----------------  + 0
	//            max - min
	return (input - min) / (max - min);
}

double distgend_is_membound_scaled(distgend_configT config) {
	return distgend_scale(config, distgend_is_membound(config));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////

static void *thread_benchmark(void *arg) {
	thread_argsT *thread_args = (thread_argsT *)arg;
	double *ret = (double *)calloc(1, sizeof(double));

	pthread_barrier_wait(&barrier);
	for (size_t i = 0; i < thread_args->config->number_of_threads; ++i) {
		if (thread_args->tid == thread_args->config->threads_to_use[i]) {
			double tsum = 0.0;
			u64 taCount = 0;

			const double t1 = wtime();
			runBench(buffer[thread_args->tid], iter, depChain, doWrite, &tsum, &taCount);
			const double t2 = wtime();

			const double temp = taCount * 64.0 / 1024.0 / 1024.0 / 1024.0;
			*ret += temp / (t2 - t1);
		}
	}

	pthread_exit((void *)ret);
}

static double bench(distgend_configT config) {
	double ret = 0.0;

	thread_argsT thread_args[system_config.number_of_threads];

	// inititalize barrier
	int res = pthread_barrier_init(&barrier, NULL, system_config.number_of_threads);
	assert(res == 0);

	for (size_t i = 0; i < system_config.number_of_threads; ++i) {
		thread_args[i].tid = i;
		thread_args[i].config = &config;
		int res = pthread_create(&threads[i], &thread_attr[i], thread_benchmark, &thread_args[i]);
		assert(res == 0);
	}

	for (size_t i = 0; i < system_config.number_of_threads; ++i) {
		double *ret_tmp;
		int res = pthread_join(threads[i], (void **)&ret_tmp);
		assert(res == 0);
		ret += *ret_tmp;
		free(ret_tmp);
	}

	// destroy barrier
	res = pthread_barrier_destroy(&barrier);
	assert(res == 0);

	return ret;
}

static void set_affinity(distgend_initT init) {
	size_t *arr = (size_t *)malloc(sizeof(size_t) * init.number_of_threads);

	// binding is created as following
	// say we have 2 NUMA * 4 cores * 2 SMT
	// 0-3  = 0. HTC on NUMA 0
	// 4-7  = 0. HTC on NUMA 1
	// 8-11 = 1. HTC on NUMA 1
	// ...
	for (size_t i = 0; i < init.number_of_threads; ++i) {
		arr[i] = i;
	}

	// set thread affinity
	for (size_t i = 0; i < init.number_of_threads; ++i) {
		cpu_set_t set;
		CPU_ZERO(&set);
		CPU_SET(arr[i], &set);
		int res = pthread_attr_setaffinity_np(&thread_attr[i], sizeof(cpu_set_t), &set);
		assert(res == 0);
	}

	free(arr);
}
