#
# Copyright (C) Jonathan D. Belanger 2019.
# All Rights Reserved.
#
# This software is furnished under a license and may be used and copied only
# in accordance with the terms of such license and with the inclusion of the
# above copyright notice.  This software or any other copies thereof may not
# be provided or otherwise made available to any other person.  No title to
# and ownership of the software is hereby transferred.
#
# The information in this software is subject to change without notice and
# should not be construed as a commitment by the author or co-authors.
#
# The author and any co-authors assume no responsibility for the use or
# reliability of this software.
#
# Description:
#
#   This CMake file is used to build the libraries and executables for the
#   DECaxp project.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written, based off of the original Makefile..
#
cmake_minimum_required(VERSION 3.6)

#
# Set the compilers we want to use.  This needs to occur prior to the project
# statement.
#
#   NOTE: We define both C and C++ compilers, just in case someone adds a
#         module written in C++.
#
#set(CMAKE_C_COMPILER "/cygdrive/g/git/llvm-project/build/bin/clang")
#set(CMAKE_CXX_COMPILER "/cygdrive/g/git/llvm-project/build/bin/clang++")

#
# Set the project and version information.
#
project(DECaxp VERSION 1.0.0)

#
# Define the set of compiler flags we want to use throughout the build
# process.
#
#   NOTE: We define compiler flags for both C and C++ compilers, just in case
#         someone adds a module written in C++.
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -std=gnu99 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=gnu++98 -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Executables")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
    set(compiler-rt "/usr/lib/clang/5.0.1/lib/windows/libclang_rt.builtins-x86_64.a")
endif()

add_subdirectory(CommonUtilities)
add_subdirectory(CPU)
add_subdirectory(Motherboard)
add_subdirectory(Devices)
add_subdirectory(DECaxp)
add_subdirectory(Tests)
