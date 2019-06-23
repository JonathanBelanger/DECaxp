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
#   V01.001 09-Jun-2019 Jonathan D. Belanger
#   Added a define to the compile flags to specify the path the the directory
#   containing the test data.
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAXP_TEST_DATA_FILES=\\\"${CMAKE_CURRENT_SOURCE_DIR}/DataFiles\\\"")

add_executable(AXP_21264_Cache_Test
    AXP_21264_Cache_Test.c)

target_include_directories(AXP_21264_Cache_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

target_link_libraries(AXP_21264_Cache_Test PRIVATE
    Caches
    Cbox
    Ibox
    Mbox
    Ebox
    Fbox
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll
    ${compiler-rt})

add_executable(AXP_21264_Dump_Test
    AXP_21264_Dump_Test.c)

target_link_libraries(AXP_21264_Dump_Test PRIVATE
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

target_include_directories(AXP_21264_Dump_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_FloatingPointTest_Test
    AXP_21264_FloatingPointTest.c)

target_link_libraries(AXP_21264_FloatingPointTest_Test PRIVATE
    Fbox
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

target_include_directories(AXP_21264_FloatingPointTest_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_Icache_Test
    AXP_21264_Icache_Test.c)

target_link_libraries(AXP_21264_Icache_Test PRIVATE
    Caches
    Cbox
    Ibox
    Mbox
    Ebox
    Fbox
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll
    ${compiler-rt})

target_include_directories(AXP_21264_Icache_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_21264_IntegerLoadTest
    AXP_21264_IntegerLoadTest.c)

target_include_directories(AXP_21264_IntegerLoadTest PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

target_link_libraries(AXP_21264_IntegerLoadTest PRIVATE
    Ebox
    Mbox
    Cbox
    Ibox
    Ebox
    Fbox
    Caches
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll
    ${compiler-rt})

add_executable(AXP_21264_Prediction_Test
    AXP_21264_Prediction_Test.c)

target_link_libraries(AXP_21264_Prediction_Test PRIVATE
    Ibox
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

target_include_directories(AXP_21264_Prediction_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Disk_Test
    AXP_Disk_Test.c)

target_include_directories(AXP_Disk_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

target_link_libraries(AXP_Disk_Test PRIVATE
    VirtualDisks
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    -luuid
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

add_executable(AXP_DS12887A_Test
    AXP_DS12887A_Test.c)

target_include_directories(AXP_DS12887A_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

target_link_libraries(AXP_DS12887A_Test PRIVATE
    TOYClock
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

add_executable(AXP_Telnet_Test
    AXP_Telnet_Test.c)

target_include_directories(AXP_Telnet_Test PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

target_link_libraries(AXP_Telnet_Test PRIVATE
    Console
    CommonUtilities
    Ethernet
    -lxml2
    -lm
    -liconv
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

add_executable(AXP_Test_Queues
    AXP_Test_Queues.c)

target_include_directories(AXP_Test_Queues PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Test_Structure_Sizes
    AXP_Test_Structure_Sizes.c)

target_include_directories(AXP_Test_Structure_Sizes PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)

add_executable(AXP_Test_Npcap
    AXP_Test_Npcap.c)

target_link_libraries(AXP_Test_Npcap PRIVATE
    /cygdrive/c/WINDOWS/system32/Npcap/wpcap.dll)

target_include_directories(AXP_Test_Npcap PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)
