cmake_minimum_required(VERSION 3.1)

########
# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their definition
# and dump it in the cache along with proper documentation, otherwise set CMAKE_BUILD_TYPE
# to Debug prior to calling PROJECT()
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()
string( TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE )
########

########
# General project settings
project(mmbwmon)
# Enable support for external projects
include(ExternalProject)
########

########
# Compiler/linker options based on the different compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Using Clang
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -Ofast -march=native)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Weverything)

    ADD_DEFINITIONS(-Wno-c++98-compat -Wno-shadow -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-vtables -Wno-extra-semi -Wno-exit-time-destructors -Wno-global-constructors)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Using GCC
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -O2 -march=native)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra -Wunused)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # Using Intel C++

  # CXX_STANDARD feature not working with older cmake + Intel
  ADD_DEFINITIONS(-std=c++11)

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -fast)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra)
  endif()

endif()
########

########
# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
########

########
# libdistgen
ExternalProject_Add(libdistgen
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libdistgen
    CONFIGURE_COMMAND  CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libdistgen
    BUILD_IN_SOURCE 0
    BUILD_COMMAND ${MAKE})

# Store distgend variable in directory
ExternalProject_Get_Property(libdistgen install_dir)
set (libdistgen_path ${install_dir})
include_directories(${libdistgen_path}/include)
link_directories(${libdistgen_path}/lib)
########

OPTION(BUILD_CGROUP_SUPPORT "Build with cgroup start/stop support." ON)

########
# libponcri
ExternalProject_Add(libponcri
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libponci
    CONFIGURE_COMMAND  CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libponci
    BUILD_IN_SOURCE 0
    BUILD_COMMAND ${MAKE})

# Store poncri variable in directory
ExternalProject_Get_Property(libponcri install_dir)
set (libponcri_path ${install_dir})
include_directories(${libponcri_path}/include)
link_directories(${libponcri_path}/lib)

IF(BUILD_CGROUP_SUPPORT)
ADD_DEFINITIONS(-DCGROUP_SUPPORT)
ENDIF(BUILD_CGROUP_SUPPORT)
########

########
# fast-lib
ExternalProject_Add(libfast
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fast-lib
    CONFIGURE_COMMAND  CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fast-lib
    BUILD_IN_SOURCE 0
    BUILD_COMMAND ${MAKE})

# Store libmosquitto variable in directory
ExternalProject_Get_Property(libfast install_dir)
set (libfast_path ${install_dir})
include_directories(${libfast_path}/include)
include_directories(${libfast_path}/include/fast-lib/external)
link_directories(${install_dir}/lib)
########

include_directories(include)

########
# Compiling and linking
add_executable(mmbwmon src/mmbwmon.cpp src/helper.cpp)
set_property(TARGET mmbwmon PROPERTY CXX_STANDARD 14)
add_dependencies(mmbwmon libdistgen libfast)
target_link_libraries(mmbwmon distgen fastlib rt ${CMAKE_THREAD_LIBS_INIT})
IF(BUILD_CGROUP_SUPPORT)
    add_dependencies(mmbwmon libponcri)
    target_link_libraries(mmbwmon poncri)
ENDIF(BUILD_CGROUP_SUPPORT)

add_executable(request src/request.cpp src/helper.cpp)
add_dependencies(request libfast)
target_link_libraries(request fastlib rt ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET request PROPERTY CXX_STANDARD 14)

add_executable(opticat src/opticat.cpp)
add_dependencies(opticat libponcri libdistgen)
target_link_libraries(opticat poncri distgen rt ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET opticat PROPERTY CXX_STANDARD 14)
########
