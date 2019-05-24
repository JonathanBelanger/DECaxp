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

#set (CMAKE_C_COMPILER "/cygdrive/g/git/llvm-project/build/bin/clang")
#set (CMAKE_CXX_COMPILER "/cygdrive/g/git/llvm-project/build/bin/clang++")

project(DECaxp VERSION 1.0.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -std=gnu99 -Wall -D_AXP_COMPILER_PRESENT_")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Executables")

add_subdirectory(CommonUtilities)
add_subdirectory(CPU)
add_subdirectory(Motherboard)
add_subdirectory(Devices)
add_subdirectory(DECaxp)
add_subdirectory(Tests)
