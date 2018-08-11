/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *  This header file contains useful definitions to be used throughout the
 *  Digital Alpha AXP emulation software.
 *
 * Revision History:
 *
 *  V01.000	10-May-2017	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	14-May-2017	Jonathan D. Belanger
 *  Added includes for a number of standard header files.
 *
 *  V01.002	17-Jun-2017	Jonathan D. Belanger
 *  Added counted queues and counted queue entries.
 *
 *  V01.003	05-Jul-2017	Jonathan D. Belanger
 *  Added definitions for unsigned/signed 128-bit values.
 *
 *  V01.004	10-Jul-2017	Jonathan D. Belanger
 *  I tried to utilize the MPFR - Multi-Precision Float Rounding utility,
 *  but had problems getting it to link in correctly.  Additionally, this
 *  utility did not support subnormal values directly (there was a way to
 *  emulate it, but was not very efficient), as well as the performance
 *  of this utility has been questioned.  Therefore, I'm reverting to what
 *  I was doing before.
 *
 *  V01.004	18-Nov-2017	Jonathan D. Belanger
 *  Uncommented the pthread.h include.  We are going threading.
 *
 *  V01.005	29-Dec-2017	Jonathan D. Belanger
 *  When sending data from the CPU to the System, we do so in upto 64-byte
 *  blocks.  Since these blocks are not necessarily consecutive, there may be
 *  gaps between the end of what section of relevant data and the start of the
 *  next.  In the real CPU, this would be broken down into a series of
 *  WrBytes/WrLWs/WrQWs or ReadBytes/ReadLWs/ReadQWs.  We need to mimic this,
 *  but do it in a single step.  We need some macros to set/decode the
 *  appropriate set of mask bits (each bit represent a single byte).
 *
 *  V01.006	26-Apr-2018	Jonathan D. Belanger
 *  Added macros to INSQUE and REMQUE entries from a doubly linked list.
 */
#ifndef _AXP_UTIL_DEFS_
#define _AXP_UTIL_DEFS_

/*
 * Includes used throughout the code.
 */
#define _FILE_OFFSET_BITS	64
#define __USE_LINUX_IOCTL_DEFS
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <fenv.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "AXP_NoCompilerHack.h"

/*
 * Define some regularly utilized definitions.
 */
#define ONE_K				1024		/* 1K */
#define TWO_K				2048		/* 2K */
#define FOUR_K				4096		/* 4K */
#define EIGHT_K				8192		/* 8K */
#define THIRTYTWO_K			32768		/* 32K */
#define SIXTYFOUR_K			65536		/* 64K */
#define ONE_M				1048576		/* 1M */
#define ONE_G				1073741824	/* 1G */
#define ONE_T				1099511627776	/* 1T */

/*
 * Test for power of 2
 */
#define IS_POWER_OF_2(value)		((value) && !((value) & (value - 1)))

/*
 * Define some standard data types.
 */
typedef unsigned char u8; /* 1 byte (8 bits) in length */
typedef unsigned short u16; /* 2 bytes (16 bits) in length */
typedef unsigned int u32; /* 4 bytes (32 bits) in length */
typedef unsigned long long u64; /* 8 bytes (64 bits) in length */
typedef unsigned __int128 u128; /* 16 bytes (128 bits) in length */
typedef char i8; /* 1 byte (8 bits) in length */
typedef short i16; /* 2 bytes (16 bits) in length */
typedef int i32; /* 4 bytes (32 bits) in length */
typedef long long i64; /* 8 bytes (64 bits) in length */
typedef __int128 i128; /* 16 bytes (128 bits) in length */

/*
 * Define some standard data type lengths (only the unique ones).
 */
#define BYTE_LEN			sizeof(u8)
#define WORD_LEN			sizeof(u16)
#define LONG_LEN			sizeof(u32)
#define QUAD_LEN			sizeof(u64)
#define CACHE_LEN			64

/*
 * Various values that are used throughout the code.
 */
#define AXP_LOW_BYTE		0x00000000000000ffll
#define AXP_LOW_WORD		0x000000000000ffffll
#define AXP_LOW_LONG		0x00000000ffffffffll
#define AXP_LOW_QUAD		(i64) 0xffffffffffffffffll
#define AXP_LOW_6BITS		0x000000000000003fll
#define AXP_LOW_3BITS		0x0000000000000007ll

/*
 * Define the SROM handle for reading in an SROM file.
 */
#define AXP_FILENAME_MAX	131
typedef struct
{
    char fileName[AXP_FILENAME_MAX + 1];
    u32 *writeBuf;
    FILE *fp;
    u32 validPat; /* must be 0x5a5ac3c3 */
    u32 inverseVP; /* must be 0xa5a53c3c */
    u32 hdrSize;
    u32 imgChecksum;
    u32 imgSize; /* memory footprint */
    u32 decompFlag;
    u64 destAddr;
    u8 hdrRev;
    u8 fwID; /* Firmware ID */
    u8 hdrRevExt;
    u8 res;
    u32 romImgSize;
    u64 optFwID; /* Optional Firmware ID */
    u32 romOffset;
    u32 hdrChecksum;
    u32 verImgChecksum;
    bool romOffsetValid;
    bool openForWrite;
} AXP_SROM_HANDLE;

#define AXP_ROM_HDR_LEN		(14*4)
#define AXP_ROM_HDR_CNT		15				/* romOffset[Valid] read as one */
#define AXP_ROM_VAL_PAT		0x5a5ac3c3
#define AXP_ROM_INV_VP_PAT	0xa5a53c3c
#define AXP_ROM_HDR_VER		1
#define AXP_ROM_FWID_DBM	0	/* Alpha Motherboards Debug Monitor firmware */
#define AXP_ROM_FWID_WNT	1	/* Windows NT firmware */
#define AXP_ROM_FWID_SRM	2	/* Alpha System Reference Manual Console */
#define AXP_ROM_FWID_FSB	6	/* Alpha Motherboards Fail-Safe Booter */
#define AXP_ROM_FWID_MILO	7	/* Linux Miniloader */
#define AXP_ROM_FWID_VXWRKS	8	/* VxWorks Real-Time Operating System */
#define AXP_ROM_FWID_SROM	10	/* Serial ROM */

/*
 * CPU <--> System Mask generation macro definitions.
 */
#define AXP_MASK_BYTE				0x0001
#define AXP_MASK_WORD				0x0003
#define AXP_MASK_LONG				0x000f
#define AXP_MASK_QUAD				0x00ff

/*
 * Define a basic queue.  This will be used to define a number of other queue
 * types.
 */
typedef struct queueHeader
{
    struct queueHeader *flink;
    struct queueHeader *blink;
} AXP_QUEUE_HDR;

/*
 * Macros to use with the basic queue.
 */
#define AXP_INIT_QUE(queue)		queue.flink = queue.blink = (void *) &queue
#define AXP_INIT_QUEP(queue)	queue->flink = queue->blink = (void *) queue
#define AXP_QUE_EMPTY(queue)	(queue.flink == (void *) &queue.flink)
#define AXP_QUEP_EMPTY(queue)	(queue->flink == (void *) &queue->flink)
#define AXP_REMQUE(entry) 							\
    {										\
	(entry)->blink->flink = (entry)->flink;					\
	(entry)->flink->blink = (entry)->blink;					\
    }
#define AXP_INSQUE(pred, entry)							\
    {										\
	(entry)->flink = (pred)->flink;						\
	(entry)->blink = (AXP_QUEUE_HDR *) (pred);				\
	(pred)->flink = (AXP_QUEUE_HDR *) (entry);				\
	(entry)->flink->blink = (AXP_QUEUE_HDR *) (entry);			\
    }

/*
 * A counted queue.  If maximum is specified as zero at initialization, then
 * the number of entries in the queue has no limit.
 */
typedef struct countedQueueEntry
{
    struct countedQueueEntry *flink;
    struct countedQueueEntry *blink;
    struct countedQueueRoot *parent;
    int index;
} AXP_CQUE_ENTRY;

typedef struct countedQueueRoot
{
    struct countedQueueEntry *flink;
    struct countedQueueEntry *blink;
    u32 count;
    u32 max;
    pthread_mutex_t mutex;
} AXP_COUNTED_QUEUE;

#define AXP_INIT_CQENTRY(queue, prent)	\
		queue.flink = NULL;				\
		queue.blink = NULL;				\
		queue.parent = &prent;

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

typedef struct condQueueHeader
{
    struct condQueueHeader *flink;
    struct condQueueHeader *blink;
    struct condQueueRoot *parent;
    AXP_COND_Q_TYPE type;
} AXP_COND_Q_HDR;

typedef struct condQueueLeaf
{
    struct condQueueLeaf *flink;
    struct condQueueLeaf *blink;
    struct condQueueRoot *parent;
} AXP_COND_Q_LEAF;

typedef struct condQueueRoot
{
    AXP_COND_Q_LEAF *flink;
    AXP_COND_Q_LEAF *blink;
    struct condQueueRoot *parent;
    AXP_COND_Q_TYPE type;
    pthread_mutex_t qMutex;
    pthread_cond_t qCond;
} AXP_COND_Q_ROOT;

typedef struct condQueueCountRoot
{
    struct condQueueLeaf *flink;
    struct condQueueLeaf *blink;
    struct condQueueCountRoot *parent;
    AXP_COND_Q_TYPE type;
    pthread_mutex_t qMutex;
    pthread_cond_t qCond;
    u32 count;
    u32 max;
} AXP_COND_Q_ROOT_CNT;

/*
 * Error returns for various AXP Utility file load functions.
 */
#define AXP_S_NORMAL		 0
#define AXP_E_FNF 		-1
#define AXP_E_BUFTOOSMALL	-2
#define AXP_E_EOF		-3
#define AXP_E_READERR		-4
#define AXP_E_BADSROMFILE	-5
#define AXP_E_BADCFGFILE	-6

/*
 * Prototype Definitions
 *
 * Trace Functionality
 */
bool AXP_TraceInit(void);

/*
 * Least Recently Used (LRU) queue functions.
 */
void AXP_LRUAdd(AXP_QUEUE_HDR *lruQ, AXP_QUEUE_HDR *entry);
void AXP_LRURemove(AXP_QUEUE_HDR *entry);
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ);

/*
 * Counted queue functions.
 */
bool AXP_InitCountedQueue(AXP_COUNTED_QUEUE *, u32);
bool AXP_LockCountedQueue(AXP_COUNTED_QUEUE *);
bool AXP_UnlockCountedQueue(AXP_COUNTED_QUEUE *);
i32 AXP_InsertCountedQueue(AXP_CQUE_ENTRY *, AXP_CQUE_ENTRY *);
i32 AXP_CountedQueueFull(AXP_COUNTED_QUEUE *, u32);
i32 AXP_RemoveCountedQueue(AXP_CQUE_ENTRY *, bool);

/*
 * Conditional queue (non-counted and counted) Functions.
 */
bool AXP_CondQueue_Init(AXP_COND_Q_ROOT *);
bool AXP_CondQueueCnt_Init(AXP_COND_Q_ROOT_CNT *, u32);
int AXP_CondQueue_Insert(AXP_COND_Q_LEAF *, AXP_COND_Q_LEAF *);
bool AXP_CondQueue_Remove(AXP_COND_Q_LEAF *, AXP_COND_Q_LEAF **);
void AXP_CondQueue_Wait(AXP_COND_Q_HDR *);
bool AXP_CondQueue_Empty(AXP_COND_Q_HDR *);

/*
 * ROM and Executable file reading and writing.
 */
u32 AXP_Crc32(const u8 *, size_t, bool, u32);
int AXP_LoadExecutable(char *, u8 *, u32);
bool AXP_OpenRead_SROM(char *, AXP_SROM_HANDLE *);
bool AXP_OpenWrite_SROM(char *, AXP_SROM_HANDLE *, u64, u32);
i32 AXP_Read_SROM(AXP_SROM_HANDLE *, u32 *, u32);
bool AXP_Write_SROM(AXP_SROM_HANDLE *, u32 *, u32);
bool AXP_Close_SROM(AXP_SROM_HANDLE *);

/*
 * Buffer masking functions.  Used for not necessarily continuous buffer
 * utilization.
 */
void AXP_MaskReset(u8 *);
void AXP_MaskSet(u8 *, u64, u64, int);
void AXP_MaskStartGet(int *);
int AXP_MaskGet(int *, u8, int);

/*
 * ASCII/UTF-16 conversion functions.
 */
i32 AXP_Ascii2UTF_16(char *, size_t, uint16_t *, size_t *);
i32 AXP_UTF16_2Ascii(uint16_t *, size_t, char *, size_t *);

/*
 * VHD and Network to/from Native value conversions.
 */
typedef enum
{
    U8,
    U16,
    U32,
    U64,
    GUID,
    CRC32,
    CVTMax
} AXP_CVT_Types;
void AXP_Convert_To(AXP_CVT_Types, void *, void *);
void AXP_Convert_From(AXP_CVT_Types, void *, void *);

/*
 * File IO functions.
 */
i64 AXP_GetFileSize(FILE *);
bool AXP_WriteAtOffset(FILE *, void *, size_t, u64);
bool AXP_ReadFromOffset(FILE *, void *, size_t *, u64);

#endif /* _AXP_UTIL_DEFS_ */
