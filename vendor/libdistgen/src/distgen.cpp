/**
* Distgen
* Multi-threaded memory access pattern generator
*
* Every thread traverses its own nested series of arrays
* with array sizes as specified. Array sizes correlate
* to distances in a reuse distance histogram, and fit into
* given layers (caches) of the memory hierarchy.
* The idea is to approximate the access pattern of real codes.
*
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

#include <omp.h>

#include <malloc.h>

static u64 clockFreq = 0; // assumed frequency for printing cycles
static u64 iters_perstat = 0;

static const char *clockFreqDef = "2.4G";

static u64 toU64(const char *s, int isSize) {
	u64 num = 0, denom = 1;
	u64 f = isSize ? 1024 : 1000;

	while ((*s >= '0') && (*s <= '9')) {
		num = 10 * num + (*s - '0');
		s++;
	}
	if (*s == '.') {
		s++;
		while ((*s >= '0') && (*s <= '9')) {
			num = 10 * num + (*s - '0');
			denom = 10 * denom;
			s++;
		}
	}

	if ((*s == 'k') || (*s == 'K'))
		num = num * f;
	else if ((*s == 'm') || (*s == 'M'))
		num = num * f * f;
	else if ((*s == 'g') || (*s == 'G'))
		num = num * f * f * f;
	num = num / denom;

	return num;
}

static void printStats(size_t ii, double tDiff, u64 rDiff, u64 wDiff) {
	u64 aDiff = rDiff + wDiff;
	double avg = tDiff * tcount / aDiff * 1000000000.0;
	double cTime = 1000.0 / clockFreq;

	fprintf(stderr, " at%5zu: ", ii);
	fprintf(stderr, " %5.3fs for %4.1f GB => %5.3f GB/s"
					" (per core: %6.3f GB/s)\n",
			tDiff, aDiff * 64.0 / 1000000000.0, aDiff * 64.0 / tDiff / 1000000000.0,
			aDiff * 64.0 / (tDiff * tcount) / 1000000000.0);
	if (verbose > 1)
		fprintf(stderr, "  per access (%llu accesses): %.3f ns (%.1f cycles @ %.1f GHz)\n", aDiff, avg, avg / cTime,
				1.0 / 1000.0 * clockFreq);
}

static size_t get_tcount() {
	static size_t tc = 0;

	if (tc > 0) return tc;

#ifdef _OPENMP
#pragma omp parallel
#pragma omp master
	tc = (size_t)omp_get_num_threads();
#else
	tc = 1;
#endif

	return tc;
}

__attribute__((noreturn)) static void usage(char *argv0) {
	fprintf(stderr, "Benchmark with threads accessing their own nested arrays at cache-line granularity\n\n"
					"Usage: %s [Options] [-<iter>] [<dist1> [<dist2> ... ]]\n"
					"\nParameters:\n"
					"  <iter>       number of times (iterations) accessing arrays (def: 1000)\n"
					"  <dist1>, ... different reuse distances (def: 1 dist with 16MB)\n"
					"\nOptions:\n"
					"  -h           show this help\n"
					"  -p           use pseudo-random access pattern\n"
					"  -d           traversal by dependency chain\n"
					"  -w           write after read on each access\n"
					"  -c <freq>    clock frequency in Hz to show cycles per access (def: %s)\n"
					"  -t <count>   set number of threads to use (def: %zu)\n"
					"  -s <iter>    print perf.stats every few iterations (def: 0 = none)\n"
					"  -v           be verbose\n",
			argv0, clockFreqDef, get_tcount());
	fprintf(stderr, "\nNumbers can end in k/m/g for Kilo/Mega/Giga factor\n");
	exit(1);
}

// this sets global options
void parseOptions(int argc, char *argv[]) {
	int arg;
	u64 dist;

	for (arg = 1; arg < argc; arg++) {
		if (argv[arg][0] == '-') {
			if (argv[arg][1] == 'h') usage(argv[0]);
			if (argv[arg][1] == 'v') {
				verbose++;
				continue;
			}
			if (argv[arg][1] == 'p') {
				pseudoRandom = 1;
				continue;
			}
			if (argv[arg][1] == 'd') {
				depChain = 1;
				continue;
			}
			if (argv[arg][1] == 'w') {
				doWrite = 1;
				continue;
			}
			if (argv[arg][1] == 'c') {
				if (arg + 1 < argc) {
					clockFreq = toU64(argv[arg + 1], 0);
					arg++;
				}
				continue;
			}
			if (argv[arg][1] == 't') {
				if (arg + 1 < argc) {
					tcount = atoi(argv[arg + 1]);
					arg++;
				}
				continue;
			}
			if (argv[arg][1] == 's') {
				if (arg + 1 < argc) {
					iters_perstat = toU64(argv[arg + 1], 0);
					arg++;
				}
				continue;
			}
			iter = (int)toU64(argv[arg] + 1, 0);
			if (iter == 0) {
				fprintf(stderr, "ERROR: expected iteration count, got '%s'\n", argv[arg] + 1);
				usage(argv[0]);
			}
			continue;
		}
		dist = toU64(argv[arg], 1);
		if (dist == 0) {
			fprintf(stderr, "ERROR: expected distance, got '%s'\n", argv[arg]);
			usage(argv[0]);
		}
		addDist(dist);
	}

	// set to defaults if values were not provided

	if (distsUsed == 0) addDist(16 * 1024 * 1024);
	if (iter == 0) iter = 1000;
	if (clockFreq == 0) clockFreq = toU64(clockFreqDef, 0);

	if (tcount == 0) {
		// thread count is the default as given by OpenMP runtime
		tcount = get_tcount();
	} else {
// overwrite thread count of OpenMP runtime
#ifdef _OPENMP
		omp_set_num_threads(tcount);
#else
		// compiled without OpenMP, cannot use more than 1 thread
		if (tcount > 1) {
			fprintf(stderr, "WARNING: OpenMP not available, running sequentially.\n");
			tcount = 1;
		}
#endif
	}

	if (iters_perstat == 0) {
		// no intermediate output
		iters_perstat = iter;
	}
}

int main(int argc, char *argv[]) {
	u64 aCount = 0, aCount1;
	double sum = 0.0;
	double tt, t1, t2;
	double avg, cTime, gData, gFlops, flopsPA;

	parseOptions(argc, argv);

	if (verbose) fprintf(stderr, "Multi-threaded Distance Generator (C) 2015 LRR-TUM\n");

	//--------------------------
	// Initialization
	//--------------------------
	initBufs();

	//--------------------------
	// Run benchmark
	//--------------------------

	fprintf(stderr, "Running %zu iterations, %zu thread(s) ...\n", iter, tcount);
	if (verbose) fprintf(stderr, "  printing statistics every %llu iterations\n", iters_perstat);

	aCount = 0;
	tt = wtime();
	t1 = tt;
	size_t ii = 0;

	// loop over chunks of iterations after which statistics are printed
	while (1) {
		aCount1 = aCount;

#pragma omp parallel reduction(+ : sum) reduction(+ : aCount)
		{
			double tsum = 0.0;
			u64 taCount = 0;

			runBench(buffer[omp_get_thread_num()], iters_perstat, depChain, doWrite, &tsum, &taCount);

			sum += tsum;
			aCount += taCount;
		}

		t2 = wtime();
		ii += iters_perstat;
		if (ii >= iter) break;

		printStats(ii, t2 - t1, aCount - aCount1, doWrite ? (aCount - aCount1) : 0);

		t1 = t2;
	}
	tt = t2 - tt;

	//--------------------------
	// Summary
	//--------------------------

	flopsPA = 1.0;
	if (doWrite) {
		aCount = 2 * aCount;
		flopsPA = .5;
	}

	avg = tt * tcount / aCount * 1000000000.0;
	cTime = 1000000000.0 / clockFreq;
	gData = aCount * 64.0 / 1024.0 / 1024.0 / 1024.0;
	gFlops = aCount * flopsPA / 1000000000.0;

	fprintf(stderr, "Summary: throughput %7.3f GB in %.3f s (per core: %.3f GB)\n", gData, tt, gData / tcount);
	fprintf(stderr, "         bandwidth  %7.3f GB/s (per core: %.3f GB/s)\n", gData / tt, gData / tt / tcount);
	fprintf(stderr, "         GFlop/s    %7.3f GF/s (per core: %.3f GF/s)\n", gFlops / tt, gFlops / tt / tcount);
	fprintf(stderr, "         per acc.   %7.3f ns   (%.1f cycles @ %.2f GHz)\n", avg, avg / cTime,
			1.0 / 1000000000.0 * clockFreq);

	if (verbose) fprintf(stderr, "         accesses   %llu, sum: %g\n", aCount, sum);

	return 0;
}
