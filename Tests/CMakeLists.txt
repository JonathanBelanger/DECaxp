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
#   This CMake file is used to build the test executables for the DECaxp
#   project.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written, based off of the original Makefile..
#
add_executable(AXP_21264_Cache_Test
    AXP_21264_Cache_Test.c)

target_include_directories(AXP_21264_Cache_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_Dump_Test
    AXP_21264_Dump_Test.c)

target_include_directories(AXP_21264_Dump_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_FloatingPointTest_Test
    AXP_21264_FloatingPointTest.c)

target_include_directories(AXP_21264_FloatingPointTest_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_Icache_Test
    AXP_21264_Icache_Test.c)

target_include_directories(AXP_21264_Icache_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_IntegerLoadTest
    AXP_21264_IntegerLoadTest.c)

target_include_directories(AXP_21264_IntegerLoadTest PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_Prediction_Test
    AXP_21264_Prediction_Test.c)

target_include_directories(AXP_21264_Prediction_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Disk_Test
    AXP_Disk_Test.c)

target_include_directories(AXP_Disk_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_DS12887A_Test
    AXP_DS12887A_Test.c)

target_include_directories(AXP_DS12887A_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Telnet_Test
    AXP_Telnet_Test.c)

target_include_directories(AXP_Telnet_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Test_Structure_Sizes
    AXP_Test_Structure_Sizes.c)

target_include_directories(AXP_Test_Structure_Sizes PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Test_WinPcap
    AXP_Test_WinPcap.c)

target_include_directories(AXP_Test_WinPcap PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)
