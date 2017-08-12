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
 *	This header file contains the definitions to implemented the ITB, DTB,
 *	Icache and Dcache.
 *
 * Revision History:
 *
 *	V01.000		29-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_CACHE_
#define _AXP_21264_CACHE_

#include "AXP_Utility.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Ibox_InstructionInfo.h"

/*
 * Cache Prototypes
 */
AXP_21264_TLB *AXP_findTLBEntry(AXP_21264_CPU *, u64, bool);
AXP_21264_TLB *AXP_getNextFreeTLB(AXP_21264_TLB *, u32 *);
void AXP_addTLBEntry(AXP_21264_CPU *, u64, u64, bool);
void AXP_tbia(AXP_21264_CPU *, bool);
void AXP_tbiap(AXP_21264_CPU *, bool);
void AXP_tbis(AXP_21264_CPU *, u64, bool);
bool AXP_21264_checkMemoryAccess(
						AXP_21264_CPU *,
						AXP_21264_TLB *,
						AXP_21264_ACCESS);
u64 AXP_va2pa(
		AXP_21264_CPU *,
		u64,
		AXP_PC,
		bool,
		AXP_21264_ACCESS,
		bool *,
		u32 *);
void AXP_DcacheAdd(AXP_21264_CPU *, u64, u64, u8 *);
void AXP_DcacheFlush(AXP_21264_CPU *);
u8 *AXP_DcacheFetch(AXP_21264_CPU *, u64, u64, u8 *);
void AXP_IcacheAdd(
				AXP_21264_CPU *,
				AXP_PC ,
				AXP_INS_FMT *,
				AXP_21264_TLB *);
void AXP_IcacheFlush(AXP_21264_CPU *, bool);
bool AXP_IcacheFetch(AXP_21264_CPU *, AXP_PC, AXP_INS_LINE *);
bool AXP_IcacheValid(AXP_21264_CPU *, AXP_PC);

#endif /* _AXP_21264_CACHE_ */
