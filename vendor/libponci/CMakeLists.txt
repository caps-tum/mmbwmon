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
project(poncri)
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
    ADD_DEFINITIONS(-g3 -O0 -Weverything -Wno-c++98-compat -Wno-global-constructors -D_GLIBCXX_DEBUG)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Using GCC
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -O2 -march=native)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra -Wunused -D_GLIBCXX_DEBUG)
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # Using Intel C++

  # CXX_STANDARD feature not working with older cmake, do manually
  ADD_DEFINITIONS(-std=c++11)

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "release")
    ADD_DEFINITIONS(-g -fast)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "debug")
    ADD_DEFINITIONS(-g3 -O0 -Wall -Wextra -D_GLIBCXX_DEBUG)
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

add_library(poncri src/ponci.cpp src/ponri.cpp)
set_property(TARGET poncri PROPERTY CXX_STANDARD 11)
INSTALL(TARGETS poncri DESTINATION "lib")
INSTALL(FILES include/ponci/ponci.h include/ponci/ponci.hpp DESTINATION "include/ponci")
INSTALL(FILES include/ponri/ponri.h include/ponri/ponri.hpp DESTINATION "include/ponri")

add_executable(poncri_example src/example.cpp)
set_property(TARGET poncri_example PROPERTY CXX_STANDARD 11)
target_link_libraries(poncri_example poncri)
target_link_libraries(poncri_example Threads::Threads)

add_executable(cgkill src/cgkill.cpp)
set_property(TARGET cgkill PROPERTY CXX_STANDARD 11)
target_link_libraries(cgkill poncri)
########
