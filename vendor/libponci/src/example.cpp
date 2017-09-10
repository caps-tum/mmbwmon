/**
 * Simple ponci test application.
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#include <iostream>
#include <string>
#include <thread>

#include <cstddef>

#include "ponci/ponci.hpp"
#include "ponri/ponri.hpp"

static void sleeper(const std::string &name) {
	cgroup_create(name);

	size_t arr[] = {0};
	cgroup_set_cpus(name, arr, 1);
	cgroup_set_mems(name, arr, 1);
	cgroup_add_me(name);

	std::cout << "Going to freeze me. brrrr" << std::endl;
	std::cout << "Please press return to thaw me." << std::endl;
	cgroup_freeze(name);

	std::cout << "Aaaah, its warm again!" << std::endl;
}

int main(/*int argc, char const *argv[]*/) {
	const std::string name("poncri_test");

	cgroup_create(name);

	size_t arr[] = {0, 2, 3};

	cgroup_set_cpus(name, arr, 3);
	cgroup_set_mems(name, arr, 1);
	cgroup_add_me(name);

	cgroup_set_memory_migrate(name, 1);
	cgroup_set_cpus_exclusive(name, 0);
	cgroup_set_mem_hardwall(name, 1);
	cgroup_set_scheduling_domain(name, -1);

	const std::string sleeper_name = name + std::string("/ponci_sleeper");
	std::thread t_freeze(sleeper, sleeper_name);

	// wait for return from keyboard
	std::cin.ignore();

	cgroup_thaw(sleeper_name);

	t_freeze.join();

	cgroup_delete(sleeper_name);

	// remove me from cgroup name and add me to the root before deleting the cgroup
	cgroup_add_me("");
	cgroup_delete(name);

	std::cout << "CBM max: " << std::hex << get_cbm_mask_as_uint() << std::endl;
	std::cout << "CBM min bits: " << get_min_cbm_bits() << std::endl;
	std::cout << "Number of closids: " << get_num_closids() << std::endl;

	resgroup_create(name);

	std::vector<size_t> cpus;
	cpus.push_back(0);
	cpus.push_back(5);
	resgroup_set_cpus(name, cpus);
	resgroup_add_me(name);

	std::vector<size_t> schematas;
	schematas.push_back(0xf0);
	schematas.push_back(0xf);
	resgroup_set_schemata(name, schematas);

	resgroup_delete(name);

	return 0;
}
