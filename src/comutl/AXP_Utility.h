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
 *	V01.002		17-Jun-2017	Jonathan D. Belanger
 *	Added counted queues and counted queue entries.
 *
 *	V01.003		05-Jul-2017	Jonathan D. Belanger
 *	Added definitions for unsigned/signed 128-bit values.
 *
 *	V01.004		10-Jul-2017	Jonathan D. Belanger
 *	I tried to utilize the MPFR - Multi-Precision Float Rounding utility,
 *	but had problems getting it to link in correctly.  Additionally, this
 *	utility did not support subnormal values directly (there was a way to
 *	emulate it, but was not very efficient), as well as the performance
 *	of this utility has been questioned.  Therefore, I'm reverting to what
 *	I was doing before.
 *
 *	V01.004		18-Nov-2017	Jonathan D. Belanger
 *	Uncommented the pthread.h include.  We are going threading.
 */
#ifndef _AXP_UTIL_DEFS_
#define _AXP_UTIL_DEFS_

/*
 * Includes used throughout the code.
 */
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <fenv.h>

/*
 * Define some regularly utilized definitions.
 */
#define ONE_K				1024	/* 1K */
#define FOUR_K				4096	/* 4K */
#define EIGHT_K				8192	/* 8K */
#define ONE_M				1048576	/* 1M */

/*
 * Define some standard data types.
 */
typedef unsigned char		u8;		/* 1 byte (8 bits) in length */
typedef unsigned short		u16;	/* 2 bytes (16 bits) in length */
typedef unsigned int		u32;	/* 4 bytes (32 bits) in length */
typedef unsigned long long	u64;	/* 8 bytes (64 bits) in length */
typedef unsigned __int128	u128;	/* 16 bytes (128 bits) in length */
typedef char				i8;		/* 1 byte (8 bits) in length */
typedef short				i16;	/* 2 bytes (16 bits) in length */
typedef int					i32;	/* 4 bytes (32 bits) in length */
typedef long long			i64;	/* 8 bytes (64 bits) in length */
typedef __int128			i128;	/* 16 bytes (128 bits) in length */

/*
 * Various values that are used throughout the code.
 */
#define AXP_LOW_BYTE		0x00000000000000ffll
#define AXP_LOW_WORD		0x000000000000ffffll
#define AXP_LOW_LONG		0x00000000ffffffffll
#define AXP_LOW_QUAD		0xffffffffffffffffll
#define AXP_LOW_6BITS		0x000000000000003fll
#define AXP_LOW_3BITS		0x0000000000000007ll

/*
 * Define a basic queue.  This will be used to define a number of other queue
 * types.
 */
typedef struct
{
	void 			*flink;
	void 			*blink;
} AXP_QUEUE_HDR;

/*
 * Macros to use with the basic queue.
 */
#define AXP_INIT_QUE(queue)		queue.flink = queue.blink = (void *) &queue.flink
#define AXP_INIT_QUEP(queue)	queue->flink = queue->blink = (void *) &queue->flink
#define AXP_QUE_EMPTY(queue)	(queue.flink == (void *) &queue.flink)
#define AXP_QUEP_EMPTY(queue)	(queue->flink == (void *) &queue->flink)

/*
 * A counted queue.  If maximum is specified as zero at initialization, then
 * the number of entries in the queue has no limit.
 */
typedef struct
{
	AXP_QUEUE_HDR	header;
	u32				count;
	u32				max;
} AXP_COUNTED_QUEUE;

typedef struct
{
	AXP_QUEUE_HDR		header;
	AXP_COUNTED_QUEUE	*parent;
} AXP_CQUE_ENTRY;

#define AXP_INIT_CQUE(queue, maximum)	\
	AXP_INIT_QUE(queue.header);			\
	queue.max = maximum;				\
	queue.count = 0
#define AXP_INIT_CQUEP(queue, maximum)	\
	AXP_INIT_QUE(queue->header);		\
	queue->max = maximum;				\
	queue->count = 0
#define AXP_INIT_CQENTRY(queue, prent)	\
		AXP_INIT_QUE(queue.header);		\
		queue.parent = &prent;
#define AXP_CQUE_EMPTY(queue)	(queue.count == 0)
#define AXP_CQUEP_EMPTY(queue)	(queue->count == 0)
#define AXP_CQUE_FULL(queue)	((queue.maximum !=0 ? (queue.count == queue.maximum) ? false)
#define AXP_CQUEP_FULL(queue)	((queue->maximum !=0 ? (queue->count == queue->maximum) ? false)
#define AXP_CQUE_COUNT(queue)	(queue.count)
#define AXP_CQUEP_COUNT(queue)	(queue->count)

/*
 * Conditional queues (using pthreads)
 *
 * Unlike the above, we are not going to use macros to initialize these queues.
 * We'll use function calls.
 */
typedef enum
{
	AXPCondQueue,
	AXPCountedCondQueue
} AXP_COND_Q_TYPE;

typedef struct
{
	void			*flink;
	void			*blink;
	void			*parent;
	AXP_COND_Q_TYPE type;
} AXP_COND_Q_HDR;

typedef struct
{
	void			*flink;
	void			*blink;
	void			*parent;
	AXP_COND_Q_TYPE type;
	pthread_mutex_t	qMutex;
	pthread_cond_t	qCond;
} AXP_COND_Q_ROOT;

typedef struct
{
	void			*flink;
	void			*blink;
	void			*parent;
} AXP_COND_Q_LEAF;

typedef struct
{
	void			*flink;
	void			*blink;
	void			*parent;
	AXP_COND_Q_TYPE type;
	pthread_mutex_t	qMutex;
	pthread_cond_t	qCond;
	u32				count;
	u32				max;
} AXP_COND_Q_ROOT_CNT;

/*
 * Error returns for AXP_LoadExecutable
 */
#define AXP_E_FNF 			-1
#define AXP_E_BUFTOOSMALL	-2

/*
 * Prototype Definitions
 *
 * Least Recently Used (LRU) queue functions.
 */
void AXP_LRUAdd(AXP_QUEUE_HDR *lruQ, AXP_QUEUE_HDR *entry);
void AXP_LRURemove(AXP_QUEUE_HDR *entry);
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ);

/*
 * Counted queue functions.
 */
i32 AXP_InsertCountedQueue(AXP_QUEUE_HDR *, AXP_CQUE_ENTRY *);
i32 AXP_RemoveCountedQueue(AXP_CQUE_ENTRY *);

/*
 * Conditional queue (non-counted and counted) Functions.
 */
bool AXP_CondQueue_Init(AXP_COND_Q_ROOT *);
bool AXP_CondQueueCnt_Init(AXP_COND_Q_ROOT_CNT *, u32);
int AXP_CondQueue_Insert(AXP_COND_Q_LEAF *, AXP_COND_Q_LEAF *);
bool AXP_CondQueue_Remove(AXP_COND_Q_LEAF *, AXP_COND_Q_LEAF **);
bool AXP_CondQueue_Wait(AXP_COND_Q_HDR *);
bool AXP_CondQueue_Empty(AXP_COND_Q_HDR *);

int AXP_LoadExecutable(char *, u8 *, u32);
bool AXP_LoadROM(AXP_21264_CPU *, char *, u32, u32);
bool AXP_UnoadROM(AXP_21264_CPU *, char *, u32, u32);

#endif /* _AXP_UTIL_DEFS_ */
