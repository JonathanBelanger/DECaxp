/*
 * Copyright (C) Jonathan D. Belanger 2017.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *	This header file contains the function definitions implemented in the
 *	AXP_21264_Fbox.c module.
 *
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_FBOX_DEFS_
#define _AXP_21264_FBOX_DEFS_

#include "AXP_21264_CPU.h"

/*
 * FP Operate Function Field Format
 * 	The formatted function field for FP operations is only done for Opcodes
 * 	0x14 and 0x16.
 */
typedef struct
{
	 u32	fnc : 4;
	 u32	src : 2;
	 u32	rnd : 2;
	 u32	trp : 3;
	 u32	res : 21;
} AXP_FP_FUNC;

#define AXP_F_BIAS				0x080
#define AXP_F_SIGN				0x00008000
#define AXP_F_SIGN_SHIFT		(63 - 15)
#define AXP_F_EXP_SHIFT_IN		52
#define AXP_F_EXP_SHIFT_OUT		7
#define AXP_F_FRAC_SHIFT		16
#define AXP_F_HIGH_MASK			0x0000003f
#define AXP_F_LOW_MASK			0xffff0000

#define AXP_G_BIAS				0x400
#define AXP_G_SIGN_EXP_HI_MASK	0x000000000000ffffll
#define AXP_G_MID_HIGH_MASK		0x00000000ffff0000ll
#define AXP_G_MID_LOW_MASK		0x0000ffff00000000ll
#define AXP_G_LOW_MASK			0xffff000000000000ll

#define AXP_S_BIAS				0x7f
#define AXP_S_NAN				0xff
#define AXP_S_SIGN				0x800000000
#define AXP_S_SIGN_SHIFT		(63 - 31)
#define AXP_S_EXP_SHIFT_IN		52
#define AXP_S_EXP_SHIFT_OUT		23
#define AXP_S_FRAC_MASK			0x007fffff
#define AXP_S_FRAC_SHIFT		29

#define AXP_T_BIAS				0x3ff

#define AXP_R_SIGN				0x8000000000000000ll
#define AXP_R_SIGN_EXP_HI_MASK	0xffff000000000000ll
#define AXP_R_MID_HIGH_MASK		0x0000ffff00000000ll
#define AXP_R_MID_LOW_MASK		0x00000000ffff0000ll
#define AXP_R_LOW_MASK			0x000000000000ffffll
#define AXP_R_EXP_SHIFT_IN		52
#define AXP_R_NAN				0x7ff
#define AXP_R_EXP				0x7ff0000000000000ll
#define AXP_R_HB				0x0010000000000000ll
#define AXP_R_FRAC				0x000fffffffffffffll
#define AXP_R_FRAC_MASK			0x000fffffe0000000ll
#define AXP_R_HIGH_F_MASK		0x000fe00000000000ll
#define AXP_R_LOW_F_MASK		0x00001fffe0000000ll
#define AXP_R_HIGH_F_SHIFT		45
#define AXP_R_LOW_F_SHIFT		18

#endif /* _AXP_21264_FBOX_DEFS_ */
