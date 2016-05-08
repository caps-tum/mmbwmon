/**
 * po     n  c       i
 * poor mans cgroups interface
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#ifdef __cplusplus
#ifndef ponci_hpp
#error "Include ponci.hpp when compiling for C++!"
#endif
#endif

#ifndef ponci_h
#define ponci_h

#include <sys/types.h>

/**
 * Creates a new cgroup with the @p name.
 */
void cgroup_create(const char *name);

/**
 * Deletes a new cgroup with the @p name.
 * Only works if the cgroup is empty.
 */
void cgroup_delete(const char *name);

/**
 * Add the calling thread to the cgroup @p name.
 */
void cgroup_add_me(const char *name);

/**
 * Adds the thread/process with @p pid to the cgroup @p name.
 */
void cgroup_add_task(const char *name, const pid_t pid);

/**
 * Allows the @p cpus to be used in the cgroup @p name. @p size is the
 * length of the array @p cpus.
 */
void cgroup_set_cpus(const char *name, const size_t *cpus, size_t size);

/**
 * Allows the memory modules @p mems to be used in the cgroup @p name.
 * @p size is the length of the array @p mems.
 */
void cgroup_set_mems(const char *name, const size_t *mems, size_t size);

/**
 * Change the memory migrate option for cgroup @p name.
 * @p flag:
 * 0 (default): Pages will not be migrated to a new mem node if cgroup
 *              set_mems is called.
 * 1          : Pages will be migrated to a the new memory nodes after
 *              calling cgroup_set_mems. The relative placement regarding
 *              the mems array is maintained (if possible), i.e.
 *              pages allocated on the second mems entry will be moved to
 *              the second entry in the new mems array.
 */
void cgroup_set_memory_migrate(const char *name, size_t flag);

/**
 * Controlls if the CPUs set via cgroup_set_cpus are exclusive to @name cgroup
 * and its parents and children.
 * @p flag
 *   0 (default): Not exclusive
 *   1          : Exclusive
 */
void cgroup_set_cpus_exclusive(const char *name, size_t flag);

/**
 * Controlls if kernel allocations (memory pages / buffer data) is restricted to
 * the memory nodes set via cgroup_set_mems.
 * @p flag
 *   0 (default): data can be shared among users / processes / cgroups and be placed on
 *                'worng' memory nodes.
 *   1          : allocation is kept separate and memory is only allocated on
 *                the memory nodes set via cgroup_set_mems.
 */
void cgroup_set_mem_hardwall(const char *name, size_t flag);

/**
 * Set the scheduling domain for cgroup @p name, i.e. the range the kernel
 * searches for idle core when scheduling tasks.
 * The default value is system dependent.
 * @p flag
 *   -1  : no request. use system default or follow request of others.
 *    0  : no search.
 *    1  : search siblings (hyperthreads in a core).
 *    2  : search cores in a package.
 *    3  : search cpus in a node [= system wide on non-NUMA system]
 *    4  : search nodes in a chunk of node [on NUMA system]
 *    5  : search system wide [on NUMA system]
 */
void cgroup_set_scheduling_domain(const char *name, int flag);

/**
 * Freezes all tasks in the cgroup.
 */
void cgroup_freeze(const char *name);

/**
 * Resumes all tasks in a cgroup.
 */
void cgroup_thaw(const char *name);

/**
 * Blocks until the cgroup is frozen.
 */
void cgroup_wait_frozen(const char *name);

/**
 * Blocks until the cgroup is thawed.
 */
void cgroup_wait_thawed(const char *name);

/**
 * Kills all processes in the cgroup.
 */
void cgroup_kill(const char *name);

#endif /* end of include guard: ponci_h */
