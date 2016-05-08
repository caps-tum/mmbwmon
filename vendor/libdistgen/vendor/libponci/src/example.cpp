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

#include "ponci/ponci.hpp"

static void sleeper(const std::string name) {
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
	const std::string name("ponci_test");

	cgroup_create(name);

	size_t arr[] = {0, 2, 3};

	cgroup_set_cpus(name, arr, 3);
	cgroup_set_mems(name, arr, 1);
	cgroup_add_me(name);

	cgroup_set_memory_migrate(name, 1);
	cgroup_set_cpus_exclusive(name, 1);
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

	return 0;
}
