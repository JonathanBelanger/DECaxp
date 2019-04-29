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
 *	AXP_21264_Mbox.c module.
 *
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_MBOX_DEFS_
#define _AXP_21264_MBOX_DEFS_

#include "CommonUtilities/AXP_Utility.h"
#include "21264Processor/AXP_21264_CPU.h"

u32 AXP_21264_Mbox_GetLQSlot(AXP_21264_CPU *);
void AXP_21264_Mbox_PutLQSlot(AXP_21264_CPU *, u32);
void AXP_21264_Mbox_ReadMem(AXP_21264_CPU *, AXP_INSTRUCTION *, u32, u64);
u32 AXP_21264_Mbox_GetSQSlot(AXP_21264_CPU *);
void AXP_21264_Mbox_PutSQSlot(AXP_21264_CPU *, u32);
void AXP_21264_Mbox_WriteMem(AXP_21264_CPU *, AXP_INSTRUCTION *, u32, u64, u64);
void AXP_21264_Mbox_CboxCompl(AXP_21264_CPU *, i8, u8 *, int, bool);
void AXP_21264_Mbox_TryCaches(AXP_21264_CPU *, u8);
void AXP_21264_Mbox_LQ_Init(AXP_21264_CPU *, u8);
void AXP_21264_Mbox_SQ_Pending(AXP_21264_CPU *, u8);
void AXP_21264_Mbox_SQ_Init(AXP_21264_CPU *, u8);
void AXP_21264_Mbox_Process_Q(AXP_21264_CPU *);
void AXP_21264_Mbox_RetireWrite(AXP_21264_CPU *, u8);
bool AXP_21264_Mbox_WorkQueued(AXP_21264_CPU *);
void AXP_21264_Mbox_UpdateDcache(AXP_21264_CPU *, i8, u8 *, u8);
bool AXP_21264_Mbox_Init(AXP_21264_CPU *);
void *AXP_21264_MboxMain(void *);

#endif /* _AXP_21264_MBOX_DEFS_ */
