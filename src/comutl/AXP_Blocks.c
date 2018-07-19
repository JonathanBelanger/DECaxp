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
 *	This source file contains the functions needed to allocate, deallocate, and
 *	initialize various data blocks used throughout the Alpha AXP Emulator.
 *
 *	NOTE:	This code does not need to be thread safe, as all the calls made
 *			from this module are thread safe.
 *
 *	Revision History:
 *
 *	V01.000		16-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 */
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_21264_CPU.h"
#include "AXP_21274_System.h"
#include "AXP_Telnet.h"
#include "AXP_Disk.h"
#include "AXP_VHDX.h"

static const char *blockNames[] =
{
    "CPU",
    "System",
    "Telnet",
    "Disk",
    "SSD",
    "VHDX",
	"Void"
};

/*
 * Define some local variables to be used to keep track of memory allocated
 * and deallocated.  These are also going to have a mutex associated with them.
 */
static pthread_once_t 	_blksOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t	_blksMutex;
static AXP_QUEUE_HDR	_blkQ = {.flink = .blink = (AXP_QUEUE_HDR *) &_blkQ};
static u32 				_blksAllocCalls = 0;
static u32 				_blksDeallocCalls = 0;
static u64 				_blksBytesAlloc = 0;
static u64 				_blksBytesDealloc = 0;

/*
 * _AXP_BlocksInit_Once
 *  This function is called using the pthread_once function to make sure that
 *  only one thread can call this function ever.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void _AXP_BlocksInit_Once(void)
{

    /*
     * Allocate the mutex,then initialize the variables.
     */
    pthread_mutex_init(&_blksMutex, NULL);

    /*
     * Return back to the caller.
     */
    return;
}


void *AXP_Allocate_Block(i32 blockType)
{
    void			*retBlock = NULL;
	u8				*allocBlock;
	AXP_BLOCK_HD	*head = NULL;
	AXP_BLOCK_TL	*tail;
	AXP_BLOCK_TYPE	type;
    size_t			size = sizeof(AXP_BLOCK_HD) + sizeof(AXP_BLOCK_TL);
	size_t			reqSize;

    pthread_once(&_blksOnce, _AXP_BlocksInit_Once);

	if (blockType < 0)
		type = AXP_VOID_BLK;
	else
		type = blockType;

    switch (blockType)
    {
	case AXP_21264_CPU_BLK:
	    {
		AXP_21264_CPU *cpu = NULL;

		reqSize = sizeof(AXP_21264_CPU);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			cpu = (AXP_21264_CPU *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);

		    /*
		     * Set the initial CPU state.
		     */
		    cpu->cpuState = Cold;
		}
	    }
	    break;

	case AXP_21274_SYS_BLK:
	    {
		AXP_21274_SYSTEM *sys = NULL;

		reqSize = sizeof(AXP_21274_SYSTEM);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			sys = (AXP_21274_SYSTEM *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	case AXP_TELNET_SES_BLK:
	    {
		AXP_TELNET_SESSION *ses = NULL;

		reqSize = sizeof(AXP_TELNET_SESSION);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			ses = (AXP_TELNET_SESSION *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	case AXP_DISK_BLK:
	    {
		AXP_Disk *dsk = NULL;

		reqSize = sizeof(AXP_Disk);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			dsk = (AXP_Disk *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	case AXP_SSD_BLK:
	    {
		AXP_SSD_Handle *ssd = NULL;

		reqSize = sizeof(AXP_SSD_Handle);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			ssd = (AXP_SSD_Handle *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	case AXP_VHDX_BLK:
	    {
		AXP_VHDX_Handle *vhdx = NULL;

		reqSize = sizeof(AXP_VHDX_Handle);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			vhdx = (AXP_VHDX_Handle *) &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	case AXP_VOID_BLK:
	    {
		u8		*blk = NULL;

		reqSize = abs(blockType);
		size += reqSize;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
			head = (AXP_BLOCK_HD *) &allocBlock[0];
			blk = &allocBlock[sizeof(AXP_BLOCK_HD)];
			tail = (AXP_BLOCK_TL *) &allocBlock[sizeof(AXP_BLOCK_HD) + reqSize);
		}
	    }
	    break;

	default:
	    break;
    }

    /*
     * If we are returning something, then we need to do some initialization
     * for accounting purposes.
     */
    if (head != NULL)
    {

	/*
	 * Set the return value.
	 */
	retBlock = allocBlock;

	/*
	 * Perform some final initializations.
	 */
	AXP_INSQUE(_blkQ.blink, head->head);
	head->type = tail->type = type;
	head->size = tail->size = size;
	head->magicNumber = 0x5555deadbeefaaaa;
	tail->magicNumber = ~head->magicNumber;

	/*
	 * Maintain some statistics
	 */
	pthread_mutex_lock(&blksMutex);
	blksAllocCalls++;
	if (retBlock != NULL)
	    blksBytesAlloc += size;
	pthread_mutex_unlock(&blksMutex);
	}

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "AXP_Allocate_Block allocated %s of size %d at 0x%016llx",
	    blockNames[blockType],
	    size,
	    retBlock);
	AXP_TraceWrite(
	    "Calls to AXP_Allocate_Block = %u; "
	    "Calls to AXP_Deallocate_Block = %u; "
	    "Memory Allocated = %u",
	    "Memory Deallocated = %u",
	    "Memory Outstanding = %u",
	    blksAllocCalls,
	    blksDeallocCalls,
	    blksBytesAlloc,
	    blksBytesDealloc,
	    (blksBytesAlloc - blksBytesDealloc));
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller with the block allocated.
     */
    return (retBlock);
}

void AXP_Deallocate_Block(void *block)
{
    AXP_BLOCK_HD	*head;
    AXP_BLOCK_TL	*tail;

    head = (AXP_BLOCK_HD *) ((u8 *) block - sizeof(AXP_BLOCK_HD));
    tail = (AXP_BLOCK_TL *) ((u8 *) block + head->size - sizeof(AXP_BLOCK_TL));

    if ((head->head.flink != NULL) &&
		(head->head.blink != NULL) &&
		(head->type == tail->type) &&
		(head->size == tail->size) &&
		(tail->magicNumber == ~head->magicNumber))
	{
	AXP_REMQUE(&head->head);
	head->head.flink = head->head.blink = NULL;

	/*
	 * Maintain some statistics
	 */
	pthread_mutex_lock(&blksMutex);
	blksDeallocCalls++;
	blksBytesDelloc -= block->size;
	pthread_mutex_unlock(&blksMutex);

	if (AXP_UTL_OPT1)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite(
		"AXP_Deallocate_Block deallocating %s of size %d at 0x%016llx",
		blockNames[block->type],
		block->size,
		block);
	    AXP_TraceWrite(
		"Calls to AXP_Allocate_Block = %u; "
		"Calls to AXP_Deallocate_Block = %u; "
		"Memory Allocated = %u; "
		"Memory Deallocated = %u; "
		"Memory Outstanding = %u",
		blksAllocCalls,
		blksDeallocCalls,
		blksBytesAlloc,
		blksBytesDealloc,
		(blksBytesAlloc - blksBytesDealloc));
	    AXP_TRACE_END();
	}

	/*
	 * Deallocate the block based on its type.
	 */
	switch (header->type)
	{
	    case AXP_21264_CPU_BLK:
	    case AXP_21274_SYS_BLK:
	    case AXP_TELNET_SES_BLK:
		case AXP_VOID_BLK:
		free(head);
		break;

	    case AXP_SSD_BLK:
		{
		AXP_SSD_Handle *ssd = (AXP_SSD_Handle *) block;

		if (ssd->memory != NULL)
		    free(ssd->memory);
		    free(head);
		}
		break;

	    case AXP_VHDX_BLK:
		{
		    AXP_VHDX_Handle *vhdx = (AXP_VHDX_Handle *) block;

		    if (vhdx->fp != NULL)
		    {
			fflush(vhdx->fp);
			fclose(vhdx->fp);
		    }
		    if (vhdx->filePath != NULL)
			free(vhdx->filePath);
		    free(head);
		}
		break;

	    case AXP_DISK_BLK:
		{
		    AXP_Disk *dsk = (AXP_Disk *) block;

		    if (dsk->ssd != NULL)
			AXP_Deallocate_Block((AXP_BLOCK_DSC *) dsk->ssd);
		    else if (dsk->vhdx != NULL)
			AXP_Deallocate_Block((AXP_BLOCK_DSC *) dsk->vhdx);
		    free(head);
		}
	    break;

	default:
	    break;
    }

    /*
     * Return back to the caller.
     */
    return;
}
