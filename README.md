# mmbwmon
Main Memory Bandwidth Monitor

[![Build Status](https://travis-ci.org/caps-tum/mmbwmon.svg?branch=master)](https://travis-ci.org/caps-tum/mmbwmon)

Mmbwmon is an agent that provides an estimate on the main memory bandwidth used
on a system. Communication between the agent and other applications is based on
MQTT with YAML messages. Take a look at the
[fast-lib](https://github.com/fast-project/fast-lib) if you are looking for a
C++ library to communicate with mmbwmon.

## Requirements
The source code in this repository is essential self contained and all unusual
dependencies are automatically build when compiling mmbowmon. Nonetheless, you
require:

* A compiler supporting C++14
* CMake (version >= 3.1)

If you wand to use mmbwmon to stop/restart cgroups (on by default), you also
need:
* cgroupfs mounted at `/sys/fs/cgroup` (`mount -t cgroup cgroup /sys/fs/cgroup/`).


## Setup
To use mmbwmon you must:

1. compile mmbwmon as every other CMake based project. Just make sure to use the
   correct compiler (i.e. by calling `CC=clang CXX=clang++ cmake ..`). If you
   want to disable cgroup support use `... cmake -DBUILD_CGROUP_SUPPORT=FALSE ..`
2. run `./mmbwmon --help` and read the available options.
3. run `./mmbwmon` with the options matching your hardware and MQTT configuration.
