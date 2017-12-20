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
#ifndef _AXP_21264_MBOX_DEFS_DEFS_
#define _AXP_21264_MBOX_DEFS_DEFS_

#include "AXP_21264_CboxDefs.h"

typedef enum
{
	QNotInUse,
	Assigned,
	Initial,
	CboxPending,
	LQReadPending,
	SQWritePending,
	LQComplete,
	SQComplete
} AXP_MBOX_QUEUE_STATE;

typedef struct
{
	u64						value;
	u64						virtAddress;
	u64						physAddress;
	AXP_INSTRUCTION			*instr;
	AXP_EXCEPTIONS			exception;
	AXP_MBOX_QUEUE_STATE	state;
	AXP_DCACHE_LOC			dcacheLoc;
	u8						len;
	bool					lockCond;
	bool					IOflag;
} AXP_MBOX_QUEUE;

#endif /* _AXP_21264_MBOX_DEFS_DEFS_ */
