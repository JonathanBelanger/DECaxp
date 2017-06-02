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
 *	AXP_21264_Ibox.c module.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		01-Jun-2017	Jonathan D. Belanger
 *	Added a function prototype to add an Icache line/block.
 */
#ifndef _AXP_21264_IBOX_DEFS_
#define _AXP_21264_IBOX_DEFS_

#include "AXP_21264_CPU.h"

#define AXP_IBOX_INS_FETCHED	4

typedef struct
{
	bool			branch2bTaken;
	bool			linePrediction;
	bool			setPrediction;
	u64				retPredStack;
	AXP_INS_FMT		instructions[AXP_IBOX_INS_FETCHED];
	AXP_INS_TYPE	instrType[AXP_IBOX_INS_FETCHED];
} AXP_IBOX_INS_LINE;

bool AXP_Branch_Prediction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool *localTaken,
				bool *globalTaken,
				bool *choice);
void AXP_Branch_Direction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool taken,
				bool localTaken,
				bool globalTaken);
AXP_INS_TYPE AXP_InstructionType(AXP_INS_FMT inst);
AXP_CACHE_FETCH AXP_ICacheFetch(AXP_21264_CPU *cpu,
								AXP_PC pc,
								AXP_IBOX_INS_LINE *next);
void AXP_ICacheAdd(AXP_21264_CPU *cpu,
				   AXP_PC pc,
				   AXP_INS_FMT *nextInst,
				   AXP_MEMORY_PROTECTION prot);
void AXP_ITBAdd(AXP_21264_CPU *cpu,
				AXP_IBOX_ITB_TAG itbTag,
				AXP_IBOX_ITB_PTE *itbPTE);
#endif /* _AXP_21264_IBOX_DEFS_ */
