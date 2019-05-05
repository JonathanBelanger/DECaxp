/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	This header file contains the function prototypes for the instruction
 *	decoding functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		15-Jan-2018	Jonathan D. Belanger
 *	Initially written from functions originally defined in AXP_21264_Ibox.c.
 *
 *	V01.001		03-Mar-2018	Jonathan D. Belanger
 *	Moved the instruction pipeline variable out of the decoded instruction
 *	structure.  The instruction really did not need to know, but the Ibox,
 *	Ebox, and Fbox did.  So, it is better located at the queue entry that goes
 *	on the IQ or FQ.  This also simplifies the mutex locking and avoids both
 *	potential deadlocks and multiple threads trying to execute an instruction.
 */
#ifndef _AXP_IBOX_INS_DECODE_DEFS_
#define _AXP_IBOX_INS_DECODE_DEFS_	1

#define AXP_SIGNAL_NONE	0
#define AXP_SIGNAL_EBOX	1
#define AXP_SIGNAL_FBOX	2

void AXP_Decode_Rename(
    AXP_21264_CPU *,
    AXP_INS_LINE *,
    int,
    AXP_INSTRUCTION *,
    AXP_PIPELINE *);
u32 AXP_UpdateRegisters(AXP_21264_CPU *, AXP_INSTRUCTION *);
bool AXP_AbortInstructions(AXP_21264_CPU *, AXP_INSTRUCTION *);
void AXP_RegisterRename_IntegrityCheck(AXP_21264_CPU *);
#endif	/* _AXP_IBOX_INS_DECODE_DEFS_ */
