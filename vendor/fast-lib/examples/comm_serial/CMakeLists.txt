#
# This file is part of fast-lib.
# Copyright (C) 2015 RWTH Aachen University - ACS
#
# This file is licensed under the GNU Lesser General Public License Version 3
# Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
#

set(FASTLIB_EXAMPLE_COMM_SERIAL "fastlib_example_comm_serial")

### Define source files.
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# Include directories
include_directories(SYSTEM "${MOSQUITTO_INCLUDE_DIR}")
include_directories(SYSTEM "${YAMLCPP_INCLUDE_DIR}")

### Build and installation targets
# Add executable
add_executable(${FASTLIB_EXAMPLE_COMM_SERIAL} ${SRC})

# Link libraries
target_link_libraries(${FASTLIB_EXAMPLE_COMM_SERIAL} ${FASTLIB})
target_link_libraries(${FASTLIB_EXAMPLE_COMM_SERIAL} -lpthread)

# Link boost.log
#add_dependencies(${FASTLIB_EXAMPLE_COMM_SERIAL} ${BoostLogLibs})
#target_link_libraries(${FASTLIB_EXAMPLE_COMM_SERIAL} ${EXAMPLE_LIBS})

# Add test
add_test(comm_serial_tests ${FASTLIB_EXAMPLE_COMM_SERIAL})
