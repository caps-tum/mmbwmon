/**
 * distgen internal header file. You should not include this!
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 * Josef Weidendorfer <weidendo@in.tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifndef distgen_internal
#define distgen_internal

#ifdef __cplusplus
extern "C" {
#endif

#include "distgen.h"

#define MAXDISTCOUNT 10
#define BLOCKLEN 64

typedef unsigned long long u64;

struct entry {
	double v;
	struct entry *next;
};

extern u64 distSize[MAXDISTCOUNT];
extern u64 distBlocks[MAXDISTCOUNT];
extern int distIter[MAXDISTCOUNT];

extern struct entry *buffer[DISTGEN_MAXTHREADS];

extern int distsUsed;
extern size_t tcount;
extern int pseudoRandom;
extern int depChain;
extern int doWrite;
extern size_t iter;
extern int verbose;

double wtime(void);

void addDist(u64 size);

void initBufs(void);

void runBench(struct entry *buffer, size_t iter, int depChain, int doWrite, double *sum, u64 *aCount);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: distgen_internal */
