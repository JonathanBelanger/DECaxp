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

#include "CommonUtilities/AXP_Utility.h"
#include "21264Processor/AXP_21264_CPU.h"
#include "21264Processor/Caches/AXP_21264_Cache.h"
#include "21264Processor/Ibox/AXP_21264_Ibox_InstructionInfo.h"
#include "21264Processor/AXP_21264_Instructions.h"
#include "21264Processor/Ibox/AXP_21264_RegisterRenaming.h"
#include "21264Processor/Cbox/AXP_21264_Cbox.h"

#define AXP_NONE	0
#define AXP_IQ		1
#define AXP_FQ		2
#define AXP_COND	3

/*
 * Function prototypes
 */
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
void AXP_ReturnIQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);
void AXP_ReturnFQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);
void AXP_21264_Ibox_Event(AXP_21264_CPU *, u32, AXP_PC, u64, u8, u8, bool, bool);
void AXP_21264_Ibox_UpdateIcache(AXP_21264_CPU *, u64, u8 *, bool);
bool AXP_21264_Ibox_Retire(AXP_21264_CPU *);
void *AXP_21264_IboxMain(void *);

#endif /* _AXP_21264_IBOX_DEFS_ */
