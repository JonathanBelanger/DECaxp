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

#include "AXP_Utility.h"
#include "AXP_21264_CPU.h"

u32 AXP_21264_Mbox_GetLQSlot(AXP_21264_CPU *);
void AXP_21264_Mbox_ReadMem(AXP_21264_CPU *, AXP_INSTRUCTION *, u32, u64, u32);
u32 AXP_21264_Mbox_GetSQSlot(AXP_21264_CPU *);
void AXP_21264_Mbox_WriteMem(AXP_21264_CPU *, AXP_INSTRUCTION *, u32, u64, u64, u32);
bool AXP_21264_Mbox_Init(AXP_21264_CPU *);
bool AXP_21264_Mbox_Init(AXP_21264_CPU *);
#endif /* _AXP_21264_MBOX_DEFS_ */
