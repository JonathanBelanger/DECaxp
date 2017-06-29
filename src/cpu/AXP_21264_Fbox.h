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

#include "AXP_Utility.h"

/*
 * Ebox Instruction Prototypes
 *
 * Floating-Point Control
 */
AXP_EXCEPTIONS AXP_FBEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBGE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBGT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FBNE(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * Floating-Point Load/Store
 */
AXP_EXCEPTIONS AXP_LDF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_LDT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_STT(AXP_21264_CPU *, AXP_INSTRUCTION *);

/*
 * Floating-Point Operate
 */
AXP_EXCEPTIONS AXP_CPYS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CPYSE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CPYSN(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTLQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL_V(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQL_SV(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVNE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MF_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MT_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);

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
#define AXP_F_SIGN_MEM			15
#define AXP_F_SIGN_REG			63
#define AXP_F_SIGN_SHIFT		(AXP_F_SIGN_REG - AXP_F_SIGN_MEM)
#define AXP_F_EXP_SHIFT_IN		52
#define AXP_F_EXP_SHIFT_OUT		7
#define AXP_F_FRAC_SHIFT		16
#define AXP_F_HIGH_MASK			0x0000003f
#define AXP_F_LOW_MASK			0xffff0000
#define AXP_F_UNPACK(val, sign, exp, frac, ret)								\
		sign = ((val) & AXP_R_SIGN) >> AXP_F_SIGN_REG;						\
		exp = ((val) & AXP_R_EXP) >> AXP_F_EXP_SHIFT_IN;					\
		frac = ((val) & AXP_R_FRAC);										\
		if ((exp == 0) && ((val) != 0))										\
			ret = ArithmeticTraps;											\
		frac = (frac | AXP_R_HB) << AXP_R_GUARD;
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
#define AXP_R_SIGN_SHIFT_IN		63
#define AXP_R_SIGN_EXP_HI_MASK	0xffff000000000000ll
#define AXP_R_MID_HIGH_MASK		0x0000ffff00000000ll
#define AXP_R_MID_LOW_MASK		0x00000000ffff0000ll
#define AXP_R_LOW_MASK			0x000000000000ffffll
#define AXP_R_EXP_SHIFT_IN		52
#define AXP_R_NAN				0x7ff
#define AXP_R_EXP				0x7ff0000000000000ll
#define AXP_R_SIGN_EXP			(AXP_R_SIGN|AXP_R_EXP)
#define AXP_R_HB				0x0010000000000000ll
#define AXP_R_FRAC				0x000fffffffffffffll
#define AXP_R_FRAC_MASK			0x000fffffe0000000ll
#define AXP_R_HIGH_F_MASK		0x000fe00000000000ll
#define AXP_R_LOW_F_MASK		0x00001fffe0000000ll
#define AXP_R_HIGH_F_SHIFT		45
#define AXP_R_LOW_F_SHIFT		18
#define AXP_R_LONG_SIGN			0x00000000c0000000ll
#define AXP_R_LONG_FRAC			0x000000003fffffffll
#define AXP_R_LSIGN_SHIFT_OUT	32
#define AXP_R_LFRAC_SHIFT_OUT	29
#define AXP_R_LONG_SMALL		0xffffffff00000000ll
#define AXP_R_LONG_LARGE		0x000000007fffffffll
#define AXP_R_GUARD				(AXP_R_SIGN_SHIFT_IN - AXP_R_EXP_SHIFT_IN)
#define AXP_R_Q2L_OVERFLOW(val)	(((val) & AXP_R_SIGN) ? 					\
								 ((val) < AXP_R_LONG_SMALL) :				\
								 ((val) > AXP_R_LONG_LARGE))

#endif /* _AXP_21264_FBOX_DEFS_ */
