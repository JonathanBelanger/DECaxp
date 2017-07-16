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
#include "AXP_Base_CPU.h"
#include "AXP_21264_CPU.h"

/*
 * IMPLEMENTATION NOTES:
 *
 *	The following definitions, other than the prototypes, are used throughout
 *	the Fbox code.  One of the things these definitions do is to determine what
 *	the value in a floating point value represents (Infinity, Zero, Finite,
 *	Denormal, Dirty-Zero, and NotANumber), as well as being able to mask out
 *	and convert one floating point to another.  Here are some of the possible
 *	conversions:
 *
 *	Conversion			Exponent Bias	Total Bits		Mantissa Bits
 *	FltA		FltB	FltA	FltB	FltA	FltB	FltA	FltB
 *	------		------	----	-----	----	----	----	----
 *	VAX F  <-> 	VAG G	128		1024	32		64		23		52
 *	VAX F  <-> 	IEEE T	128		1023	32		64		23		52
 *	VAX G  <-> 	VAX D	1024	128		64		64		52		55
 *	VAX G  <-> 	IEEE X	1024	16383	64		128		52		112
 *	IEEE S <-> 	IEEE T	127		1023	32		64		23		52
 *
 *	VAX hidden bit is	0.1m
 *	IEEE hidden bit is	1.m
 *	m = mantissa without hidden bit.
 *
 *	For exponent conversions VAX to VAX is simply a Bias A minus Bias B. For
 *	exponent conversions IEEE to IEEE is also the same conversion as VAX to
 *	VAX.  For VAX to IEEE, the location of the hidden bit needs to be
 *	considered, so the bias conversion VAX to IEEE is the exponent minus the
 *	total of one plus the VAX bias minus the IEEE bias.  The conversion from
 *	IEEE is the same as the VAX to IEEE, except the above formula is subtracted
 *	from the exponent.  The special cases, exponent all zeros or all ones, for
 *	IEEE only, need to be handled separately.  Therefore, the following rules
 *	are utilized:
 *
 *		IEEE Special		VAX Special
 *		------------		------------------------
 *		+Zero		<->		+Zero
 *		-Zero		-->		+Zero
 *		Denormal	-->		+Zero
 *		Finite		-->		Finite
 *		NotANumber	-->		Invalid Operation error*
 *		+Infinity	-->		Overflow*
 *		-Infinity	-->		Underflow*
 *		-Zero		<--		Dirty Zero*
 *	*Effort will be made to not allow these to occur.
 *
 *	For fraction conversions, the only consideration is the change in accuracy.
 *	When going from a fraction with a larger number of bits to one with a
 *	smaller number of bits, the bits from bit 0 up until bit 'x', where 'x' is
 *	the difference in number of bits, simply dropped (effectively a shift right
 *	of 'x' bits).  When going from a fraction with fewer bits to one with more,
 *	the opposite is performed and 'x' zero bits are inserted starting at bit 0.
 *	(effectively a shift left of 'x' bits).
 *
 *	For all the VAX floating-point operations, the VAX Float is converted to an
 *	IEEE float of higher exponent and fraction size (see above).  Then the
 *	operation is executed using the IEEE compliant functions provided by the
 *	GCC compiler and operating system.  Finally, the result is converted back
 *	into the VAX Float.  This prevents Infinity from coming into play, as well
 *	as any overflow and underflow considerations (at least as far as the
 *	operation is concerned).  When converting from the larger IEEE float to the
 *	smaller VAX float, then both overflow and underflow do come into play.
 */

/*
 * Fbox Instruction Prototypes
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
AXP_EXCEPTIONS AXP_FCMOVEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVGT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_FCMOVNE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MF_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_MT_FPCR(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_ADDF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_ADDG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_ADDS(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_ADDT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPGEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPGLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPGLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPTEQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPTLE(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPTLT(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CMPTUN(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTGQ(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQF(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTQG(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_EXCEPTIONS AXP_CVTDG(AXP_21264_CPU *, AXP_INSTRUCTION *);

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

#define AXP_FP_ADD		0x0
#define AXP_FP_SUB		0x1
#define AXP_FP_MUL		0x2
#define AXP_FP_DIV		0x3
#define AXP_FP_ITOF		0x4
#define AXP_FP_CMPUN	0x4
#define AXP_FP_CMPEQ	0x5
#define AXP_FP_CMPLT	0x6
#define AXP_FP_CMPLE	0x7
#define AXP_FP_SQRTFG	0xa
#define AXP_FP_SQRTST	0xb
#define AXP_FP_CTVS		0xc
#define AXP_FP_CVTF		0xc
#define AXP_FP_CVTD		0xd
#define AXP_FP_CVTT		0xe
#define AXP_FP_CVTG		0xe
#define AXP_FP_CVTQ		0xf

#define AXP_FP_S		0x0
#define AXP_FP_T		0x2
#define AXP_FP_Q		0x3
#define AXP_FP_F		0x0
#define AXP_FP_D		0x1
#define AXP_FP14_F		0x1
#define AXP_FP_G		0x2
#define AXP_FP_Q		0x3

#define AXP_FP_CHOPPED	0x0
#define AXP_FP_MINUS_INF 0x1
#define AXP_FP_NORMAL	0x2
#define AXP_FP_DYNAMIC	0x3
#define AXP_FP_PLUS_INF 0x3

#define AXP_FP_TRP_V	0x1							/* /V trap */
#define AXP_FP_TRP_U	0x1							/* /U trap */
#define AXP_FP_TRP_I	0x2							/* /I trap - IEEE only */
#define AXP_FP_TRP_S	0x4							/* /S trap */

/*
 * Floating-point macros used for returning and testing various values.
 */
#define AXP_R_SIGN				0x8000000000000000ll
#define AXP_R_EXP				0x7ff0000000000000ll
#define AXP_R_FRAC				0x000fffffffffffffll
#define AXP_R_NM 				0x8000000000000000ll	/* normalized */
#define AXP_R_NMBIT				63
#define AXP_R_HB				0x0010000000000000ll
#define AXP_R_NAN				0x7ff
#define AXP_R_GUARD				(AXP_R_NMBIT - 52)
#define AXP_R_LONG_SMALL		0xffffffff00000000ll
#define AXP_R_LONG_LARGE		0x000000007fffffffll
#define AXP_R_QNAN				0x0008000000000000ll
#define AXP_R_ZERO				0x0000000000000000ll	/* plus zero */
#define AXP_R_MZERO				0x8000000000000000ll	/* minus zero */
#define AXP_R_PINF				0x7ff0000000000000ll	/* plus infinity */
#define AXP_R_MINF				0xfff0000000000000ll	/* minus infinity */
#define AXP_R_PMAX				0x7fefffffffffffffll	/* plus maximum */
#define AXP_R_MMAX				0xffefffffffffffffll	/* minus maximum */
#define AXP_R_EXP_MAX			0x7ff

#define AXP_F_EXP_SIZE			8
#define AXP_F_FRAC_SIZE			23
#define AXP_F_BIAS				(1UL << (AXP_F_EXP_SIZE - 1))
#define AXP_F_HIDDEN_BIT		(1UL << AXP_F_FRAC_SIZE)
#define AXP_F_EXP_MASK			0xff
#define AXP_F_EXP_MAX			0xff
#define AXP_F_RND				0x0000008000000000ll 	/* F rounding bit */

#define AXP_D_EXP_SIZE			8
#define AXP_D_FRAC_SIZE			55
#define AXP_D_BIAS				(1UL << (AXP_D_EXP_SIZE - 1))
#define AXP_D_HIDDEN_BIT		(1UL << AXP_D_FRAC_SIZE)
#define AXP_D_EXP_MASK			0xff
#define AXP_D_EXP_MAX			0xff
#define AXP_D_GUARD				(AXP_R_NMBIT - AXP_D_FRAC_SIZE)
#define AXP_D_RND				0x0000000000000080ll	/* D round */

#define AXP_G_EXP_SIZE			11
#define AXP_G_FRAC_SIZE			52
#define AXP_G_BIAS				(1UL << (AXP_G_EXP_SIZE - 1))
#define AXP_G_HIDDEN_BIT		(1UL << AXP_G_FRAC_SIZE)
#define AXP_G_EXP_MASK			0x7ff
#define AXP_G_EXP_MAX			0x7ff
#define AXP_G_RND				0x0000000000000400ll	/* G rounding bit */

#define AXP_S_EXP_SIZE			8
#define AXP_S_FRAC_SIZE			23
#define AXP_S_BIAS				((1 << (AXP_S_EXP_SIZE - 1)) - 1)
#define AXP_S_HIDDEN_BIT		(1 << AXP_S_FRAC_SIZE)
#define AXP_S_NAN				0xff
#define AXP_S_EXP_MASK			0xff
#define AXP_S_EXP_MAX			0xff
#define AXP_S_CQ_NAN			0xfff8000020000000ll
#define AXP_S_CS_NAN			0x7ff0000020000000ll
#define AXP_S_RND				0x0000008000000000ll	/* S normal round */
#define AXP_S_INF				0x000000ffffffffffll	/* S infinity round */

#define AXP_T_EXP_SIZE			11
#define AXP_T_FRAC_SIZE			52
#define AXP_T_BIAS				((1UL << (AXP_T_EXP_SIZE - 1)) - 1)
#define AXP_T_HIDDEN_BIT		(1UL << AXP_T_FRAC_SIZE)
#define AXP_T_NAN				0x7ff
#define AXP_T_EXP_MASK			0x7ff
#define AXP_T_EXP_MAX			0x7ff
#define AXP_T_CQ_NAN			0xfff8000000000001ll
#define AXP_T_CS_NAN			0x7ff0000000000001ll
#define AXP_T_RND				0x0000000000000400ll	/* T normal round */
#define AXP_T_INF				0x00000000000007ffll	/* T infinity round */

#define AXP_X_EXP_SIZE			15
#define AXP_X_FRAC_SIZE			112
#define AXP_X_BIAS				((1LL << (AXP_X_EXP_SIZE - 1))- 1)
#define AXP_X_HIDDEN_BIT		(1LL << AXP_X_FRAC_SIZE)
#define AXP_X_EXP_MASK			0x7fff
#define AXP_X_EXP_MAX			0x7fff

#define AXP_Q_POSMAX			0x7fffffffffffffffll
#define AXP_Q_NEGMAX			0x8000000000000000ll

#define AXP_R_Q2L_OVERFLOW(val)	(((val) & AXP_R_SIGN) ? 					\
								 ((val) < AXP_R_LONG_SMALL) :				\
								 ((val) > AXP_R_LONG_LARGE))

/*
 * Exploded rounding constants
 */
//#define AXP_F_DT				0						/* type F */
//#define AXP_G_DT				1						/* type G */
//#define AXP_D_DT				2						/* type D */
//#define AXP_L_DT				3						/* type L, Quadword Integer */
//#define AXP_Q_DT				4						/* type Q, Quadword Integer */
//#define AXP_S_DT				0						/* type S */
//#define AXP_T_DT				1						/* type T */
//#define AXP_X_DT				2						/* type X */

/*
 * This structure is used to extract a floating point value's (all forms)
 * constituent parts into separate fields, that are actually the same size or
 * larger, to be used in converting from one floating type to another
 */
typedef struct
{
	u8		type;
	bool	sign;
	i32		exponent;
	u128	fraction;
} AXP_FP_FLOAT_COMPONENTS;;

/*
 * VAX G/F and IEEE S/T returned values, returned on VAX and IEEE Floating
 * Compare instructions.
 */
#define AXP_G_HALF				0x4000000000000000ll	/* VAX G/F */
#define AXP_T_TWO				0x4000000000000000ll	/* IEEE T/S 2.0 */
#define AXP_FPR_ZERO			0x0000000000000000ll	/* True 0.0 */

/*
 * The following definitions are used throughout the rest of the floating-point
 * code.  Register values are "unpacked" into this structure and then "packed"
 * back into the register.  The unpacking will set certain fields, the
 * operation will set others, and finally the packing will use the fields to
 * both round the results (simple for VAX, complex for IEEE) and then pack them
 * into a 64-bit floating point register.
 */
typedef enum
{
	Reserved,
	Zero,
	Finite,
	Denormal,
	Infinity,
	NotANumber,
	DirtyZero
} AXP_FP_ENCODING;

#define AXP_FP_ENCODE(ur, ieeeFP)											\
	((ur)->exponent == AXP_R_EXP_MAX ?										\
		((ieeeFP) ?															\
			 ((ur)->fraction == 0 ?											\
					 Infinity : 											\
					 NotANumber) : 											\
			 Finite) : 														\
		((ur)->exponent == 0 ?												\
			((ieeeFP) ?														\
				((ur)->fraction == 0 ?										\
					Zero :													\
					Denormal) :												\
				((ur)->sign == 1 ?											\
					Reserved :												\
					((ur)->fraction == 0 ?									\
						Zero :												\
						DirtyZero))) :										\
		Finite))

#define AXP_FP_CVT_EXP_F2T(fp)												\
	((fp).exponent ? (fp).exponent - (1 + AXP_F_BIAS - AXP_T_BIAS) : 0) & AXP_T_EXP_MASK
#define AXP_FP_CVT_EXP_T2F(fp)												\
	((fp).exponent ? (fp).exponent + (1 + AXP_F_BIAS - AXP_T_BIAS) : 0)

#define AXP_FP_CVT_EXP_G2F(fp)												\
	((fp).exponent ? (fp).exponent - (AXP_G_BIAS - AXP_F_BIAS) : 0)
#define AXP_FP_CVT_EXP_F2G(fp)												\
	((fp).exponent ? (fp).exponent + (AXP_G_BIAS - AXP_F_BIAS) : 0) & AXP_G_EXP_MASK

#define AXP_FP_CVT_EXP_G2D(fp)												\
	((fp).exponent ? (fp).exponent - (AXP_G_BIAS - AXP_D_BIAS) : 0)
#define AXP_FP_CVT_EXP_D2G(fp)												\
	((fp).exponent ? (fp).exponent + (AXP_G_BIAS - AXP_D_BIAS) : 0) & AXP_G_EXP_MASK

#define AXP_FP_CVT_EXP_G2X(fp)												\
	((fp).exponent ? (fp).exponent - (1 + AXP_G_BIAS - AXP_X_BIAS) : 0) & AXP_X_EXP_MASK
#define AXP_FP_CVT_EXP_X2G(fp)												\
	((fp).exponent ? (fp).exponent + (1 + AXP_G_BIAS + AXP_X_BIAS) : 0)

#define AXP_FP_CVT_EXP_S2T(fp)												\
	((fp).exponent ? (fp).exponent + (AXP_S_BIAS - AXP_T_BIAS) : 0) & AXP_T_EXP_MASK
#define AXP_FP_CVT_EXP_T2S(fp)												\
	((fp).exponent ? (fp).exponent - (AXP_S_BIAS - AXP_T_BIAS) : 0)

#endif /* _AXP_21264_FBOX_DEFS_ */
