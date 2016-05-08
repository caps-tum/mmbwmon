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

#include "ponci/ponci.hpp"

int main(int argc, char const *argv[]) {
	for (int i = 1; i < argc; ++i) {
		std::cout << "Going to kill cgroup \"" << argv[i] << "\" ...";
		std::cout.flush();
		cgroup_kill(argv[i]);
		std::cout << "dead." << std::endl;
	}

	return 0;
}
