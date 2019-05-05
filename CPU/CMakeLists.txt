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
#   This CMake file is used to build the CommonUtilities static library.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written.
#
add_subdirectory(Caches)
add_subdirectory(Cbox)
add_subdirectory(Ebox)
add_subdirectory(Fbox)
add_subdirectory(Ibox)
add_subdirectory(Mbox)

add_library(CPU STATIC
    AXP_21264_CPU.c)

target_include_directories(CPU PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)
