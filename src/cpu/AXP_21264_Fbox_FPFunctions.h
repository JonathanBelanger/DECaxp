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

float AXP_FP_CvtFPRToFloat(AXP_FP_REGISTER);
AXP_FP_REGISTER AXP_FP_CvtFloatToFPR(float);
int AXP_FP_SetRoundingMode(AXP_21264_CPU *, AXP_FP_FUNC *, int);
int AXP_FP_SetExceptionMode(AXP_21264_CPU *, int);
void AXP_FP_SetFPCR(AXP_21264_CPU *, AXP_INSTRUCTION *, int, bool);
void AXP_FP_SetExcSum(AXP_INSTRUCTION *, int, bool);

#endif	/* _AXP_21264_FBOX_FPFUNCTIONS_DEFS_ */
