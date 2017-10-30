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
 *	This header file contains the structures and definitions required to
 *	implement the Load and Store Queues of the Mbox.
 *
 *	Revision History:
 *
 *	V01.000		28-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_MBOX_QUEUES_DEFS_
#define _AXP_21264_MBOX_QUEUES_DEFS_

#include "AXP_21264_Cbox.h"

typedef enum
{
	QNotInUse,
	Assigned,
	ReadPending,
	WritePending
} AXP_MBOX_QUEUE_STATE;

typedef struct
{
	u64						value;	/* Only used in a store operations */
	u32						length;
	u64						virtAddress;
	AXP_INSTRUCTION			*instr;
	AXP_MBOX_QUEUE_STATE	state;
} AXP_MBOX_QUEUE;

typedef enum
{
	MNotInUse,
	Dcache,
	Icache,
	IOread			/* These are not stored in the Dcache */
} AXP_MBOX_MAF_TYPE;

typedef enum
{
	LDx,
	STx,
	STx_C,
	WH64,
	ECB,
	Istream
} AXP_MBOX_MERGE_INS;

/*
 * In HRM Section 2, the MAF is documented as being in the Mbox/Memory
 * Reference Unit.  In HRM Section 4.1.1.1, it states the following:
 *
 *		The Cbox contains an 8-entry miss buffer (MAF) and an 8-entry victim
 *		buffer (VAF).
 */
typedef struct
{
	AXP_MBOX_MAF_TYPE			type;
	AXP_21264_SYSADD_OUT_CMD	rq;
	AXP_21264_SYSADD_IN_SYSDC	rsp;
	u64							pa;
	bool						valid;
	bool						complete;	/* cleared by Mbox, set by Cbox */
} AXP_MBOX_MAF;

#endif /* _AXP_21264_MBOX_QUEUES_DEFS_ */
