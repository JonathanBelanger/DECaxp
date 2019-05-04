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
#   This CMake file is used to build the 21264Processor static library.
#
# Revision History:
#   V01.000 28-Apr-2019 Jonathan D. Belanger
#   Initially written.
#
add_library(Fbox STATIC
    AXP_21264_Fbox_Control.c
    AXP_21264_Fbox_FPFunctions.c
    AXP_21264_Fbox_LoadStore.c
    AXP_21264_Fbox_OperateIEEE.c
    AXP_21264_Fbox_OperateMisc.c
    AXP_21264_Fbox_OperateVAX.c
    AXP_21264_Fbox.c)

target_include_directories(Fbox PRIVATE
    ${PROJECT_SOURCE_DIR}/Includes)
