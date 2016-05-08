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
project(distgend)
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
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    # Don't use -fast. IPO will remove our test code :)
    ADD_DEFINITIONS(-g -O3 -no-prec-div -static -xHost)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra)
  endif()

endif()
########

########
# OpenMP
find_package(OpenMP REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
########

########
# libponci
ExternalProject_Add(libponci
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libponci
    CONFIGURE_COMMAND  CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libponci
    BUILD_IN_SOURCE 0
    BUILD_COMMAND ${MAKE})

# Store libponci variable in directory
ExternalProject_Get_Property(libponci install_dir)
set (libponci_path ${install_dir})
include_directories(${libponci_path}/include)
link_directories(${libponci_path}/lib)
########

########
include_directories(include)

# Compiling and linking
add_library(distgen src/distgen_internal.cpp src/libdistgen.cpp)
add_dependencies(distgen libponci)
set_property(TARGET distgen PROPERTY CXX_STANDARD 11)
INSTALL(TARGETS distgen DESTINATION "lib")
INSTALL(FILES include/distgen/distgen.h DESTINATION "include/distgen")

add_executable(dgen src/distgen.cpp)
set_property(TARGET dgen PROPERTY CXX_STANDARD 11)
target_link_libraries(dgen distgen)

add_executable(distgen_example src/example.cpp )
add_dependencies(distgen_example libponci)
target_link_libraries(distgen_example distgen ponci)
set_property(TARGET distgen_example PROPERTY CXX_STANDARD 11)
########
