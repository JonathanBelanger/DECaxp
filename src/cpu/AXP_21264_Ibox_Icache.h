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
 *	This header file contains the definitions needed by the Icache
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		22-Jun-2017	Jonathan D. Belanger
 *	Initially written, migrated from AXP_21264_Ibox.c.
 */
#ifndef _AXP_21264_IBOX_ICACHE_DEFS_
#define _AXP_21264_IBOX_ICACHE_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Ibox_InstructionInfo.h"
#include "AXP_21264_Icache.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_RegisterRenaming.h"

AXP_CACHE_FETCH AXP_ICacheFetch(AXP_21264_CPU *, AXP_PC, AXP_INS_LINE *);
AXP_CACHE_FETCH AXP_ICacheValid(AXP_21264_CPU *, AXP_PC, u32 *, u32 *);
void AXP_ICacheAdd(AXP_21264_CPU *, AXP_PC, AXP_INS_FMT *, AXP_ICACHE_ITB *);
void AXP_ITBAdd(AXP_21264_CPU *, AXP_IBOX_ITB_TAG, AXP_IBOX_ITB_PTE *);

#endif	/* _AXP_21264_IBOX_ICACHE_DEFS_ */
