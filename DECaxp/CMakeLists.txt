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
#   This CMake file is used to build the DECaxp and DECaxp_Generate_SROM
#   executables for the DECaxp project.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written, based off of the original Makefile..
#
add_executable(DECaxp_Generate_SROM
    DECaxp_Generate_SROM.c)

#set(CMAKE_LINK_LIBRARY_FLAG
#    "${CMAKE_LINK_LIBRARY_FLAG} -lxml2 -lm -liconv -luuid -L /cygdrive/c/WINDOWS/system32 -lwpcap")
#

target_link_libraries(DECaxp_Generate_SROM PRIVATE
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/npcap/wpcap.dll)

target_include_directories(DECaxp_Generate_SROM PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(DECaxp
    DECaxp.c)

target_link_libraries(DECaxp PRIVATE
    CPU
    Cbox
    Ibox
    Ebox
    Fbox
    Mbox
    Caches
    Motherboard
    Cchip
    Dchip
    Pchip
    Console
    TOYClock
    CommonUtilities
    Ethernet
    VirtualDisks
    -lxml2
    -lm
    -liconv
    -luuid
    /cygdrive/c/WINDOWS/system32/wpcap.dll
    ${compiler-rt})

target_include_directories(DECaxp PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

