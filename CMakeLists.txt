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

project(DECaxp VERSION 1.0.0)

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -c -m64 -std=gnu99 -Wall -D_AXP_COMPILER_PRESENT_")
set(CMAKE_LINK_LIBRARY_FLAG
    "${CMAKE_LINK_LIBRARY_FLAG} -lxml2 -lm -liconv -luuid -L /cygdrive/c/WINDOWS/system32 -lwpcap")

add_subdirectory(CommonUtilities)
add_subdirectory(21264Processor)
add_subdirectory(TyphoonTsunami)
add_subdirectory(Devices)
add_subdirectory(DECaxp)
add_subdirectory(Tests)
