#
# This file is part of fast-lib.
# Copyright (C) 2015 RWTH Aachen University - ACS
#
# This file is licensed under the GNU Lesser General Public License Version 3
# Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
#

# Default build type: Debug
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
cmake_minimum_required(VERSION 2.8)
project(fast-lib)
if(POLICY CMP0026)
	cmake_policy(SET CMP0026 OLD) # Used to get location of boost libs
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Cache
set(BUILD_SHARED_LIBS OFF CACHE BOOL "If ON build shared, else static libraries.")
if(BUILD_SHARED_LIBS)
	message(FATAL_ERROR "BUILD_SHARED_LIBS=ON is not supported yet. Use static libs instead.")
endif()

set(ENABLE_LOGGING ON CACHE BOOL "Enable logging.")
if(ENABLE_LOGGING)
	add_definitions(-DFASTLIB_ENABLE_LOGGING)
endif()

set(BUILD_TESTS ON CACHE BOOL "Enable build of tests.")

# Library names
set(FASTLIB "fastlib")

# Define version number
set(FASTLIB_VERSION_MAJOR "0")
set(FASTLIB_VERSION_MINOR "1")
set(FASTLIB_VERSION_PATCH "0")
set(FASTLIB_VERSION "${FASTLIB_VERSION_MAJOR}.${FASTLIB_VERSION_MINOR}.${FASTLIB_VERSION_PATCH}")

# Check compiler version
set(MIN_GCC_VERSION "4.9.0")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_C_COMPILER_VERSION LESS "${MIN_GCC_VERSION}")
		message(FATAL_ERROR "gcc must be of version ${MIN_GCC_VERSION} or higher.")
	endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION LESS "${MIN_GCC_VERSION}")
		message(FATAL_ERROR "g++ must be of version ${MIN_GCC_VERSION} or higher.")
	endif()
endif()

# Set compiler flags
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" CXX11_SUPPORTED)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" CXX0X_SUPPORTED)
if(CXX11_SUPPORTED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(CXX0X_SUPPORTED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

### Define header, source and lib files
# Headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
set(HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/communicator.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/mqtt_communicator.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/serializable.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/log.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/optional.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/init.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/init_agent.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/stop_monitor.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/mmbwmon/ack.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/mmbwmon/request.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/mmbwmon/reply.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/mmbwmon/stop.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/agent/mmbwmon/restart.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/migfra/pci_id.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/migfra/time_measurement.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/migfra/result.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fast-lib/message/migfra/task.hpp"
)

# Source
set(SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt_communicator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/serializable.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/init.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/init_agent.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/stop_monitor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/mmbwmon/ack.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/mmbwmon/request.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/mmbwmon/reply.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/mmbwmon/stop.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/agent/mmbwmon/restart.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/migfra/pci_id.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/migfra/time_measurement.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/migfra/result.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/message/migfra/task.cpp"
)

# Add library
add_library(${FASTLIB} ${SRC} ${HEADERS})

# Enable support for external projects
include(ExternalProject)
set(EXTERNAL_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
file(MAKE_DIRECTORY ${EXTERNAL_INCLUDES}/fast-lib/external)
include_directories(SYSTEM ${EXTERNAL_INCLUDES}/fast-lib/external)

########
# mosquitto
ExternalProject_Add(mosquitto
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mosquitto-1.3.5
	CMAKE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake
	CMAKE_ARGS -DWITH_SRV=OFF -DWITH_TLS=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_C_FLAGS=-fpic -DCMAKE_CXX_FLAGS=-fpic
)

add_dependencies(${FASTLIB} mosquitto)

# Store mosquitto variables
ExternalProject_Get_Property(mosquitto install_dir)
set(MOSQUITTO_LIBRARIES ${install_dir}/lib/libmosquittopp.a ${install_dir}/lib/libmosquitto.a)
set(MOSQUITTO_INCLUDE_DIR "${install_dir}/include")
set(LIBS "${LIBS}" "${MOSQUITTO_LIBRARIES}")

#include_directories(SYSTEM "${MOSQUITTO_INCLUDE_DIR}")
add_custom_command(
	TARGET mosquitto
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h" "${EXTERNAL_INCLUDES}/fast-lib/external/mosquitto.h"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${MOSQUITTO_INCLUDE_DIR}/mosquittopp.h" "${EXTERNAL_INCLUDES}/fast-lib/external/mosquittopp.h"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${MOSQUITTO_INCLUDE_DIR}/mosquitto_plugin.h" "${EXTERNAL_INCLUDES}/fast-lib/external/mosquitto_plugin.h")

# Install headers
install(DIRECTORY "${MOSQUITTO_INCLUDE_DIR}/" DESTINATION include/fast-lib/external)
########

########
# yaml-cpp
ExternalProject_Add(yaml-cpp
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/yaml-cpp
	CMAKE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake
	CMAKE_ARGS -DCMAKE_CXX_FLAGS=-fpic -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DYAML_CPP_BUILD_TOOLS=OFF)

add_dependencies(${FASTLIB} yaml-cpp)

# Store yaml-cpp variables
ExternalProject_Get_Property(yaml-cpp install_dir)
set(YAMLCPP_LIBRARY "${install_dir}/lib/libyaml-cpp.a")
set(YAMLCPP_INCLUDE_DIR "${install_dir}/include")
set(LIBS "${LIBS}" "${YAMLCPP_LIBRARY}")

#include_directories(SYSTEM "${YAMLCPP_INCLUDE_DIR}")
add_custom_command(
	TARGET yaml-cpp
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${YAMLCPP_INCLUDE_DIR}/yaml-cpp" "${EXTERNAL_INCLUDES}/fast-lib/external/yaml-cpp")

# Install headers
install(DIRECTORY "${YAMLCPP_INCLUDE_DIR}/" DESTINATION include/fast-lib/external)
########

########
# spdlog
add_custom_target(spdlog)
add_dependencies(${FASTLIB} spdlog)
set(SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include")
#include_directories(SYSTEM "${SPDLOG_INCLUDE_DIR}")
add_custom_command(
	TARGET spdlog
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${SPDLOG_INCLUDE_DIR}/spdlog" "${EXTERNAL_INCLUDES}/fast-lib/external/spdlog")

# Install headers
install(DIRECTORY "${SPDLOG_INCLUDE_DIR}/" DESTINATION include/fast-lib/external)
########

# Merge with dependent libraries
include(MergeStaticLibraries)
MERGE_STATIC_LIBRARIES(${FASTLIB} ALL "${LIBS}")

# As librt should always be available it is linked dynamically and therefore has to be linked in applications
# that use fast-lib as well.
target_link_libraries(${FASTLIB} -lrt)

# Install
install(TARGETS "${FASTLIB}"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib")
install(DIRECTORY "include/"
	DESTINATION "include")

# Testing
if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
