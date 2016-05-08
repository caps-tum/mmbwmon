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
project(ponci)
########

########
# Compiler/linker options based on the different compilers
#ADD_DEFINITIONS(-std=c++11)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Using Clang
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -Ofast -march=native)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Weverything -Wno-c++98-compat)
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
# Compiling and linking
include_directories(include)

add_library(ponci src/ponci.cpp)
set_property(TARGET ponci PROPERTY CXX_STANDARD 11)
INSTALL(TARGETS ponci DESTINATION "lib")
INSTALL(FILES include/ponci/ponci.h include/ponci/ponci.hpp DESTINATION "include/ponci")


add_executable(ponci_example src/example.cpp)
set_property(TARGET ponci_example PROPERTY CXX_STANDARD 11)
target_link_libraries(ponci_example ponci)
target_link_libraries(ponci_example Threads::Threads)

add_executable(cgkill src/cgkill.cpp)
set_property(TARGET ponci_example PROPERTY CXX_STANDARD 11)
target_link_libraries(cgkill ponci)
########
