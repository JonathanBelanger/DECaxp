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
 *	This header file contains useful definitions to be used throughout the
 *	Digital Alpha AXP emulation software.
 *
 *	Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Added includes for a number of standard header files.
 *
 */
#ifndef _AXP_UTIL_DEFS_
#define _AXP_UTIL_DEFS_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ONE_K				1024
#define FOUR_K				4096

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef char				i8;
typedef short				i16;
typedef int					i32;
typedef long long			i64;

typedef struct
{
	void *flink;
	void *blink;
} AXP_QUEUE_HDR;

#define AXP_INIT_QUE(queue)		queue.flink = queue.blink = (void *) &queue.flink
#define AXP_INIT_QUEP(queue)	queue->flink = queue->blink = (void *) &queue->flink
#define AXP_QUEUE_EMPTY(queue)	(queue.flink == (void *) &queue.flink)
#define AXP_QUEUEP_EMPTY(queue)	(queue->flink == (void *) &queue->flink)

void AXP_LRUAdd(AXP_QUEUE_HDR *lruQ, AXP_QUEUE_HDR *entry);
void AXP_LRURemove(AXP_QUEUE_HDR *entry);
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ);

#endif /* _AXP_UTIL_DEFS_ */
