#
# This file is part of fast-lib.
# Copyright (C) 2015 RWTH Aachen University - ACS
#
# This file is licensed under the GNU Lesser General Public License Version 3
# Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
#

########
# fructose
set(FRUCTOSE_INCLUDE_DIR "../vendor/fructose/include")
include_directories(SYSTEM "${FRUCTOSE_INCLUDE_DIR}")
install(DIRECTORY "${FRUCTOSE_INCLUDE_DIR}/" DESTINATION include/fast-lib/external)
########

set(FASTLIB_COMMUNICATION_TEST "fastlib_communication_test")
set(FASTLIB_TASK_TEST "fastlib_task_test")

# Include directories
include_directories(SYSTEM "${MOSQUITTO_INCLUDE_DIR}")
include_directories(SYSTEM "${YAMLCPP_INCLUDE_DIR}")
include_directories(SYSTEM "${SPDLOG_INCLUDE_DIR}")

### Build and installation targets
# Add executable
add_executable(${FASTLIB_COMMUNICATION_TEST} ${CMAKE_CURRENT_SOURCE_DIR}/communication.cpp)
add_executable(${FASTLIB_TASK_TEST} ${CMAKE_CURRENT_SOURCE_DIR}/task.cpp)

# Link libraries
target_link_libraries(${FASTLIB_COMMUNICATION_TEST} ${FASTLIB} -lpthread)
target_link_libraries(${FASTLIB_TASK_TEST} ${FASTLIB} -lpthread)

# Add test
add_test(communication ${FASTLIB_COMMUNICATION_TEST})
add_test(task ${FASTLIB_TASK_TEST})
