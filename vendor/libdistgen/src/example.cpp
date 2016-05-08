/**
 * Simple disgend test application.
 *
 * Copyright 2016 by LRR-TUM
 * Jens Breitbart     <j.breitbart@tum.de>
 * Josef Weidendorfer <weidendo@in.tum.de>
 *
 * Licensed under GNU Lesser General Public License 2.1 or later.
 * Some rights reserved. See LICENSE
 */

#include "distgen/distgen.h"

#include <iostream>

int main() {
	distgend_initT init;
	init.SMT_factor = 1;
	init.NUMA_domains = 2;
	init.number_of_threads = 16;

	std::cout << "Starting distgen initialization ...";
	std::cout.flush();
	distgend_init(init);
	std::cout << " done!" << std::endl << std::endl;

	distgend_configT config;
	for (size_t i = 0; i < init.number_of_threads; ++i) {
		config.number_of_threads = i + 1;
		config.threads_to_use[i] = static_cast<unsigned char>(i);
		std::cout << "Using " << i + 1 << " threads:" << std::endl;
		std::cout << "\tMaximum: " << distgend_get_max_bandwidth(config) << " GByte/s" << std::endl;
		std::cout << "\tPortion currently available: " << distgend_is_membound(config) << std::endl;
		std::cout << std::endl;
	}
}
