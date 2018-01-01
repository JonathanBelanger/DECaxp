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
 *	AXP_21264_Cbox.c module.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		12-Oct-2017	Jonathan D. Belanger
 *	Change the CTAG structure to contain the DTAG index for the corresponding
 *	entry.  This should make keeping these 2 arrays, along with the Dcache
 *	itself, in synch.  NOTE: The set information is the same for all.
 *
 *	V01.002		14-Oct-2017	Jonathan D. Belanger
 *	Started defining the CPU to System and System to CPU messages.
 *
 *	V01.003		22-Oct-2017	Jonathan D. Belanger
 *	Differentiated between messages that come over the SysAddIn_L[14:0] pins
 *	and SysAddOut_L[14:0] pins.  Also defined a number of the remaining
 *	structures needed by the Cbox,
 *
 *	V01.004		06-Nov-2017	Jonathan D. Belanger
 *	Split this fine into 2 separate files.  One to include the definitions for
 *	the Cbox CSRs and one to be included by AXP_21264_Cbox.c so that it will
 *	compile cleanly.  This file is the one that will allow the source module
 *	to compile cleanly.
 *
 *	V01.005		31-Dec-2017	Jonathan D. Belanger
 *	Added Cbox function prototypes.
 */
#ifndef _AXP_21264_CBOX_DEFS_DEFS_
#define _AXP_21264_CBOX_DEFS_DEFS_

#include "AXP_21264_CboxDefs.h"
#include "AXP_Configure.h"
#include "AXP_21264_CPU.h"

/*
 * Prototype definitions for functions in the Cbox.
 *
 * AXP_21264_Cbox_Bcache.c
 */
void AXP_21264_Bcache_Evict(AXP_21264_CPU *, u64);
void AXP_21264_Bcache_Flush(AXP_21264_CPU *);
bool AXP_21264_Bcache_Valid(AXP_21264_CPU *, u64);
u32 AXP_21264_Bcache_Status(AXP_21264_CPU *, u64);
bool AXP_21264_Bcache_Read(AXP_21264_CPU *, u64, u8 *, bool *, bool *);
void AXP_21264_Bcache_Write(AXP_21264_CPU *, u64, u8 *);
void AXP_21264_Bcache_SetShared(AXP_21264_CPU *, u64);
void AXP_21264_Bcache_ClearShared(AXP_21264_CPU *, u64);
void AXP_21264_Bcache_SetDirty(AXP_21264_CPU *, u64);
void AXP_21264_Bcache_ClearDirty(AXP_21264_CPU *, u64);

/*
 * AXP_21264_Cbox_IOWB.c
 */
int AXP_21264_IOWB_Empty(AXP_21264_CPU *);
void AXP_21264_Process_IOWB(AXP_21264_CPU *, int);
bool AXP_21264_Merge_IOWB(AXP_21264_CBOX_IOWB *, u64, i8, u8 *, int, int);
void AXP_21264_Add_IOWB(AXP_21264_CPU *, u64, i8, u8 *, int);
void AXP_21264_Free_IOWB(AXP_21264_CPU *, u8);

/*
 * AXP_21264_Cbox_MAF.c
 */
int AXP_21264_MAF_Empty(AXP_21264_CPU *);
void AXP_21264_Process_MAF(AXP_21264_CPU *, int);
void AXP_21264_Complete_MAF(AXP_21264_CPU *, int, AXP_21264_SYSDC_RSP, u8 *);
bool AXP_21265_Check_MAFAddrSent(AXP_21264_CPU *, u64, u8 *);
bool AXP_21264_Add_MAF_Mem(AXP_21264_CPU *, AXP_CBOX_MAF_TYPE, u64, i8, int, bool);
bool AXP_21264_Add_MAF_IO(AXP_21264_CPU *, AXP_CBOX_MAF_TYPE, u64, i8, int, bool);
void AXP_21264_Add_MAF(AXP_21264_CPU *, AXP_CBOX_MAF_TYPE, u64, i8, int, bool);
void AXP_21264_Free_MAF(AXP_21264_CPU *, u8);

/*
 * AXP_21264_Cbox_PQ.c
 */
void AXP_21264_OldestPQFlags(AXP_21264_CPU *, bool *, bool *, bool *);
int AXP_21264_PQ_Empty(AXP_21264_CPU *);
void AXP_21264_Process_PQ(AXP_21264_CPU *, int);
void AXP_21264_SendRsps_PQ(AXP_21264_CPU *);
void AXP_21264_Add_PQ(
				AXP_21264_CPU *, int, AXP_21264_SYSDC_RSP, u64, u8, u8 *,
				bool, bool, bool, bool);
void AXP_21264_Free_PQ(AXP_21264_CPU *, u8);

/*
 * AXP_21264_Cbox_VDB.c
 */
int AXP_21264_VDB_Empty(AXP_21264_CPU *);
void AXP_21264_Process_VDB(AXP_21264_CPU *, int);
u8 AXP_21264_Add_VDB(AXP_21264_CPU *, AXP_21264_VDB_TYPE, u64, u8 *, bool, bool);
bool AXP_21264_IsSetP_VDB(AXP_21264_CPU *, u64);
void AXP_21264_ClearP_VDB(AXP_21264_CPU *, u8);
void AXP_21264_Free_VDB(AXP_21264_CPU *, u8);

/*
 * AXP_21264_Cbox.c
 */
void AXP_21264_Cbox_IQArbiter(AXP_21264_CPU *);
void AXP_21264_Cbox_FQArbiter(AXP_21264_CPU *);
bool AXP_21264_Cbox_Config(AXP_21264_CPU *);
void AXP_21264_Process_IRQ(AXP_21264_CPU *);
void AXP_21264_Set_IRQ(AXP_21264_CPU *, u8);
bool AXP_21264_Cbox_Init(AXP_21264_CPU *);
void *AXP_21264_CboxMain(void *);

#endif /* _AXP_21264_CBOX_DEFS_DEFS_ */
