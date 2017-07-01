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
 *	This source file contains the functions needed to implement the
 *	functionality of the Fbox Floating Point Functions needed for the
 *	Digital Alpha AXP Processor.
 *
 *	Revision History:
 *
 *	V01.000		29-June-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_FBOX_FPFUNCTIONS_DEFS_
#define _AXP_21264_FBOX_FPFUNCTIONS_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_Fbox.h"

#define AXP_FV_HB		0x0010000000000000ll
#define AXP_FD_HB		0x0080000000000000ll
#define AXP_FV_GUARD	(63 - 52)
#define AXP_FD_GUARD	(63 - 55)
#define AXP_QUIET_NAN	0x0008000000000000ll

#define AXP_V_UNPACK(src, dest, status)										\
	(dest).ieeeType = VAXFloat;												\
	(dest).sign = (src).fp.t.sign;											\
	(dest).exponent = (src).fp.t.exponent;									\
	(dest).fraction = ((src).fp.t.fraction | AXP_FV_HB) << AXP_FV_GUARD;	\
	if ((dest).exponent == 0)												\
	{																		\
		if ((src).fp.uq != 0)												\
			(status) = IllegalOperand;										\
		(dest).sign = (dest).fraction = 0;									\
	}
#define AXP_D_UNPACK(src, dest, status)										\
	(dest).ieeeType = VAXFloat;												\
	(dest).sign = (src).fp.d.sign;											\
	(dest).exponent = (src).fp.d.exponent;									\
	(dest).fraction = ((src).fp.d.fraction | AXP_FD_HB) << AXP_FD_GUARD;	\
	if ((dest).exponent == 0)												\
	{																		\
		if ((src).fp.uq != 0)												\
			(status) = IllegalOperand;										\
		(dest).sign = (dest).fraction = 0;									\
	}																		\
	(dest).exponent += (AXP_G_BIAS - AXP_D_BIAS)
#define AXP_I_UNPACK(src, dest, status)										\
	(dest).ieeeType = ieeeFloat;											\
	(dest).sign = (src).fp.t.sign;											\
	(dest).exponent = (src).fp.t.exponent;									\
	(dest).fraction = (src).fp.t.fraction;									\
	if ((dest).exponent == 0)												\
	{																		\
		if ((dest).fraction == 0)											\
			(dest).ieeeType = ieeeZero;										\
		else if (cpu->fpcr.dnz == 1)										\
		{																	\
			(dest).fraction = 0;											\
			(dest).ieeeType = ieeeZero;										\
		}																	\
		else																\
		{																	\
			ieeeNorm((dest));												\
			(dest).ieeeType = ieeeDenormal;									\
			(status) = IllegalOperand;										\
		}																	\
	}																		\
	if ((dest).ieeeType == ieeeFloat)										\
	{																		\
		if ((dest).exponent == AXP_R_NAN)									\
		{																	\
			if ((dest).fraction == 0)										\
				(dest).ieeeType = ieeeInfinity;								\
			else if (((dest).fraction & AXP_QUIET_NAN) == 0)				\
				(status) = IllegalOperand;									\
			(dest).ieeeType = ieeeNotANumber;								\
		}																	\
		else																\
		{																	\
			(dest).fraction = ((dest).fraction | AXP_FV_HB) << AXP_FV_GUARD;\
			(dest).ieeeType = ieeeFinite;									\
		}																	\
	}

#endif	/* _AXP_21264_FBOX_FPFUNCTIONS_DEFS_ */
