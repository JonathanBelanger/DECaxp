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
#   This CMake file is used to build the Devices static library for the DECaxp
#   project.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written, based off of the original Makefile..
#
add_library(VirtualDisks STATIC
    AXP_RAW.c
    AXP_SSD.c
    AXP_VHD_Utility.c
    AXP_VHD.c
    AXP_VHDX.c
    AXP_VirtualDisk.c)

target_include_directories(VirtualDisks PRIVATE
    ${PROJECT_SOURCE_DIR}/Include)
