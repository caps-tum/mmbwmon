#
# This file is part of fast-lib.
# Copyright (C) 2015 RWTH Aachen University - ACS
#
# This file is licensed under the GNU Lesser General Public License Version 3
# Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
#

set(FASTLIB_EXAMPLE_COMMUNICATION "fastlib_example_communication")

### Define source files.
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# Include directories
include_directories(SYSTEM "${MOSQUITTO_INCLUDE_DIR}")

### Build and installation targets
# Add executable
add_executable(${FASTLIB_EXAMPLE_COMMUNICATION} ${SRC})

# Link libraries
target_link_libraries(${FASTLIB_EXAMPLE_COMMUNICATION} ${FASTLIB})
target_link_libraries(${FASTLIB_EXAMPLE_COMMUNICATION} -lpthread)

# Link boost.log
#add_dependencies(${FASTLIB_EXAMPLE_COMMUNICATION} ${BoostLogLibs})
#target_link_libraries(${FASTLIB_EXAMPLE_COMMUNICATION} ${EXAMPLE_LIBS})

# Add test
add_test(communication_tests ${FASTLIB_EXAMPLE_COMMUNICATION})
