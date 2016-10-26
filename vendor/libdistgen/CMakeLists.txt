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
    ADD_DEFINITIONS(-g3 -O0 -Weverything -Wno-c++98-compat-pedantic)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Using GCC
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -O2 -march=native -pthread)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra -Wunused)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # Using Intel C++

  # CXX_STANDARD feature not working with older cmake + Intel
  ADD_DEFINITIONS(-std=c++11)

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
# pthread
find_package (Threads REQUIRED)
########

########
include_directories(include)

# Compiling and linking
add_library(distgen src/distgen_internal.cpp src/libdistgen.cpp)
set_property(TARGET distgen PROPERTY CXX_STANDARD 11)
INSTALL(TARGETS distgen DESTINATION "lib")
INSTALL(FILES include/distgen/distgen.h DESTINATION "include/distgen")

add_executable(distgen_example src/example.cpp)
target_link_libraries(distgen_example distgen ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET distgen_example PROPERTY CXX_STANDARD 11)
########
