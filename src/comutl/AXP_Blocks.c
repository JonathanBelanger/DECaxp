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
 *  This source file contains the functions needed to allocate, deallocate, and
 *  initialize various data blocks used throughout the Alpha AXP Emulator.
 *
 *  NOTE:	This code does not need to be thread safe, as all the calls made
 *		from this module are thread safe.
 *
 * Revision History:
 *
 *  V01.000		16-May-2017	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001		19-Jul-2018	Jonathan D. Belanger
 *  I am changing the way memory is allocated and deallocated.  In the initial
 *  implementation, a memory block needed to have the below structure defined
 *  at it's top.  The new implementation will have a hidden block before and
 *  after the block being requested.  The header and trailer blocks will have
 *  magic numbers in them at the boundary of the memory block being requested.
 *  This will help to determine when memory was overwritten.  Also, the header
 *  header block will contain a place for the block to be queued up, so that we
 *  can detect when a block is deallocated more than once, or not deallocated
 *  at all.
 */
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_21264_CPU.h"
#include "AXP_21274_System.h"
#include "AXP_Telnet.h"
#include "AXP_Disk.h"
#include "AXP_VHDX.h"
#include "AXP_Ethernet.h"

static const char *_blockNames[] =
{
    "Unknown",
    "CPU",
    "System",
    "Telnet",
    "Ethernet",
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
static AXP_QUEUE_HDR	_blkQ =
    {
	.flink = (AXP_QUEUE_HDR *) &_blkQ.flink,
	.blink = (AXP_QUEUE_HDR *) &_blkQ.flink
    };
static u32 		_blksAllocCalls = 0;
static u32 		_blksDeallocCalls = 0;
static u64 		_blksBytesAlloc = 0;
static u64 		_blksBytesDealloc = 0;
static const size_t	_cpu_blk_size = sizeof(_CPU_BLK);
static const size_t	_sys_blk_size = sizeof(_SYS_BLK);
static const size_t	_ses_blk_size = sizeof(_SES_BLK);
static const size_t	_eth_blk_size = sizeof(_ETH_BLK);
static const size_t	_disk_blk_size = sizeof(_DISK_BLK);
static const size_t	_ssd_blk_size = sizeof(_SSD_BLK);
static const size_t	_vhdx_blk_size = sizeof(_VHDX_BLK);
static const size_t	_head_tail_size = sizeof(AXP_BLOCK_HD) +
					  sizeof(AXP_BLOCK_TL);

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

/*
 * AXP_Allocate_Block
 */
void *AXP_Allocate_Block(i32 blockType, ...)
{
    void		*retBlock = NULL;
    void		*allocBlock;
    va_list		ap;
    AXP_BLOCK_TYPE	type;
    size_t		size = 0;

    pthread_once(&_blksOnce, _AXP_BlocksInit_Once);

    if (blockType < 0)
	type = AXP_VOID_BLK;
    else
	type = blockType;

    switch (type)
    {
	case AXP_21264_CPU_BLK:
	    {
		_CPU_BLK *cpu = NULL;

		size = _cpu_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    cpu = (_CPU_BLK *) allocBlock;

		    cpu->head.type = cpu->tail.type = type;
		    cpu->head.size = cpu->tail.size = size;
		    cpu->head.magicNumber = AXP_HD_MAGIC;
		    cpu->head.tail = &cpu->tail;
		    cpu->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &cpu->head.head);

		    /*
		     * Set the initial CPU state.
		     */
		    cpu->cpu.cpuState = Cold;

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &cpu->cpu;
		}
	    }
	    break;

	case AXP_21274_SYS_BLK:
	    {
		_SYS_BLK *sys = NULL;

		size = _sys_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    sys = (_SYS_BLK *) allocBlock;

		    sys->head.type = sys->tail.type = type;
		    sys->head.size = sys->tail.size = size;
		    sys->head.magicNumber = AXP_HD_MAGIC;
		    sys->head.tail = &sys->tail;
		    sys->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &sys->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &sys->sys;
		}
	    }
	    break;

	case AXP_TELNET_SES_BLK:
	    {
		_SES_BLK *ses = NULL;

		size = _ses_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    ses = (_SES_BLK *) allocBlock;

		    ses->head.type = ses->tail.type = type;
		    ses->head.size = ses->tail.size = size;
		    ses->head.magicNumber = AXP_HD_MAGIC;
		    ses->head.tail = &ses->tail;
		    ses->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &ses->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &ses->ses;
		}
	    }
	    break;

	case AXP_ETHERNET_BLK:
	    {
		_ETH_BLK *eth = NULL;

		size = _eth_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    eth = (_ETH_BLK *) allocBlock;

		    eth->head.type = eth->tail.type = type;
		    eth->head.size = eth->tail.size = size;
		    eth->head.magicNumber = AXP_HD_MAGIC;
		    eth->head.tail = &eth->tail;
		    eth->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &eth->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &eth->eth;
		}
	    }
	    break;

	case AXP_DISK_BLK:
	    {
		_DISK_BLK *dsk = NULL;

		size = _disk_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{

		    dsk->head.type = dsk->tail.type = type;
		    dsk->head.size = dsk->tail.size = size;
		    dsk->head.magicNumber = AXP_HD_MAGIC;
		    dsk->head.tail = &dsk->tail;
		    dsk->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &dsk->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &dsk->disk;
		}
	    }
	    break;

	case AXP_SSD_BLK:
	    {
		_SSD_BLK *ssd = NULL;

		size = _ssd_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    ssd->head.type = ssd->tail.type = type;
		    ssd->head.size = ssd->tail.size = size;
		    ssd->head.magicNumber = AXP_HD_MAGIC;
		    ssd->head.tail = &ssd->tail;
		    ssd->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &ssd->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &ssd->ssd;
		}
	    }
	    break;

	case AXP_VHDX_BLK:
	    {
		_VHDX_BLK *vhdx = NULL;

		size = _vhdx_blk_size;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    vhdx->head.type = vhdx->tail.type = type;
		    vhdx->head.size = vhdx->tail.size = size;
		    vhdx->head.magicNumber = AXP_HD_MAGIC;
		    vhdx->head.tail = &vhdx->tail;
		    vhdx->tail.magicNumber = AXP_TL_MAGIC;
		    AXP_INSQUE(_blkQ.blink, &vhdx->head.head);

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &vhdx->vhdx;
		}
	    }
	    break;

	case AXP_VOID_BLK:
	    {
		u8		*blk = NULL;
		void		*replaceBlk = NULL;
		size_t		bytes = abs(blockType);

		va_start(ap, blockType);
		replaceBlk = va_arg(ap, void *);

		size = _head_tail_size + bytes;
		allocBlock = calloc(1, size);
		if (allocBlock != NULL)
		{
		    AXP_BLOCK_HD *head;
		    AXP_BLOCK_TL *tail;

		    blk = (u8 *) allocBlock;

		    /*
		     * If this is a realloc, then we just copy the contents of
		     * the old block into the new and then deallocate the old
		     * block.
		     */
		    if (replaceBlk != NULL)
		    {
			AXP_BLOCK_HD *oldHead =
			    (AXP_BLOCK_HD *) ((u8 *) replaceBlk -
				sizeof(AXP_BLOCK_HD));

			head = (AXP_BLOCK_HD *) &blk[0];

			/*
			 * Copy the old block into the new block, but just from
			 * the head to the byte just before the tail in the
			 * old block.
			 */
			memcpy(
			    replaceBlk,
			    allocBlock,
			    (head->size - sizeof(AXP_BLOCK_TL)));

			/*
			 * Determine the location of the tail block in the
			 * newly allocated block.
			 */
			head->tail =
			    (AXP_BLOCK_TL *) &blk[sizeof(AXP_BLOCK_HD) + bytes];

			/*
			 * Copy the old tail into the new tail location.
			 */
			memcpy(head->tail, oldHead->tail, sizeof(AXP_BLOCK_TL));

			/*
			 * Correct the sizes.
			 */
			head->size = head->tail->size = size;

			/*
			 * We no longer need the old block.
			 */
			AXP_Deallocate_Block(replaceBlk);
		    }
		    else
		    {

			head = (AXP_BLOCK_HD *) &blk[0];
			tail = (AXP_BLOCK_TL *) &blk[sizeof(AXP_BLOCK_HD) + bytes];

			head->type = tail->type = type;
			head->size = tail->size = size;
			head->magicNumber = AXP_HD_MAGIC;
			head->tail = tail;
			tail->magicNumber = AXP_TL_MAGIC;
			AXP_INSQUE(_blkQ.blink, &head->head);
		    }

		    /*
		     * Set the return value to the correct item.
		     */
		    retBlock = (void *) &blk[sizeof(AXP_BLOCK_HD)];
		}
		va_end(ap);
	    }
	    break;

	default:
	    break;
    }

    /*
     * If we are returning something, then we need to do some initialization
     * for accounting purposes.
     */
    if (retBlock != NULL)
    {

	/*
	 * Maintain some statistics
	 */
	pthread_mutex_lock(&_blksMutex);
	_blksAllocCalls++;
	if (retBlock != NULL)
	    _blksBytesAlloc += size;
	pthread_mutex_unlock(&_blksMutex);
    }

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "AXP_Allocate_Block allocated %s of size %d at 0x%016llx",
	    _blockNames[blockType],
	    size,
	    retBlock);
	AXP_TraceWrite(
	    "Calls to AXP_Allocate_Block = %u; "
	    "Calls to AXP_Deallocate_Block = %u; "
	    "Memory Allocated = %u",
	    "Memory Deallocated = %u",
	    "Memory Outstanding = %u",
	    _blksAllocCalls,
	    _blksDeallocCalls,
	    _blksBytesAlloc,
	    _blksBytesDealloc,
	    (_blksBytesAlloc - _blksBytesDealloc));
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller with the block allocated.
     */
    return (retBlock);
}

/*
 * AXP_Dellocate_Block
 */
void AXP_Deallocate_Block(void *block)
{
    AXP_BLOCK_HD	*head;
    AXP_BLOCK_TL	*tail;

    head = (AXP_BLOCK_HD *) ((u8 *) block - sizeof(AXP_BLOCK_HD));
    tail = head->tail;

    if ((head->head.flink != NULL) && (head->head.blink != NULL) &&
	(head->type == tail->type) && (head->size == tail->size) &&
	(head->magicNumber == AXP_HD_MAGIC) &&
	(tail->magicNumber == AXP_TL_MAGIC))
    {
	AXP_REMQUE(&head->head);
	head->head.flink = head->head.blink = NULL;

	/*
	 * Maintain some statistics
	 */
	pthread_mutex_lock(&_blksMutex);
	_blksDeallocCalls++;
	_blksBytesDealloc += head->size;
	pthread_mutex_unlock(&_blksMutex);

	if (AXP_UTL_OPT1)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite(
		"AXP_Deallocate_Block deallocating %s of size %d at 0x%016llx",
		_blockNames[head->type],
		head->size,
		block);
	    AXP_TraceWrite(
		"Calls to AXP_Allocate_Block = %u; "
		"Calls to AXP_Deallocate_Block = %u; "
		"Memory Allocated = %u; "
		"Memory Deallocated = %u; "
		"Memory Outstanding = %u",
		_blksAllocCalls,
		_blksDeallocCalls,
		_blksBytesAlloc,
		_blksBytesDealloc,
		(_blksBytesAlloc - _blksBytesDealloc));
	    AXP_TRACE_END();
	}

	/*
	 * Deallocate the block based on its type.
	 */
	switch (head->type)
	{
	    case AXP_21264_CPU_BLK:
	    case AXP_21274_SYS_BLK:
	    case AXP_TELNET_SES_BLK:
	    case AXP_VOID_BLK:
		free(head);
		break;

	    case AXP_ETHERNET_BLK:
		{
		    AXP_Ethernet_Handle *eth = (AXP_Ethernet_Handle *) block;
		    if (eth->handle != NULL)
			AXP_EthernetClose(eth);
		    else
			free(head);
		}
		break;

	    case AXP_SSD_BLK:
		{
		    AXP_SSD_Handle *ssd = (AXP_SSD_Handle *) block;

		    if (ssd->memory != NULL)
			AXP_Deallocate_Block(ssd->memory);
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
			AXP_Deallocate_Block(vhdx->filePath);
		    free(head);
		}
		break;

	    case AXP_DISK_BLK:
		{
		    AXP_Disk *dsk = (AXP_Disk *) block;

		    if (dsk->ssd != NULL)
			AXP_Deallocate_Block(dsk->ssd);
		    else if (dsk->vhdx != NULL)
			AXP_Deallocate_Block(dsk->vhdx);
		    free(head);
		}
	    break;

	default:
	    break;
	}
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_ReturnType_Block
 */
AXP_BLOCK_TYPE AXP_ReturnType_Block(void *block)
{
    AXP_BLOCK_HD	*head;
    AXP_BLOCK_TL	*tail;
    AXP_BLOCK_TYPE	retVal = AXP_UNKNOWN_BLK;

    /*
     * Let's try and determine the type of block we are dealing with here.
     */
    if (block != NULL)
    {
	head = (AXP_BLOCK_HD *) ((u8 *) block - sizeof(AXP_BLOCK_HD));
	tail = head->tail;

	/*
	 * We have a valid block if all are true:
	 *
	 *  1)	The forward and backward links in the head of the block are
	 *	not NULL (they need to be pointing to something).
	 *  2)	The types saved in the head and tail of the block are the same,
	 *  	and a valid type.
	 *  3)	The sizes saved in the head and tail are the same, and also
	 *	valid.
	 *  4)	The magic numbers at the head and tail are correct.
	 */
	if ((head->head.flink != NULL) && (head->head.blink != NULL) &&
	    (head->type == tail->type) &&
	    ((head->type > AXP_UNKNOWN_BLK) && (head->type < AXP_BLOCK_MAX)) &&
	    (head->size == tail->size) &&
	    (((head->type == AXP_21264_CPU_BLK) &&
	      (head->size == _cpu_blk_size)) ||
	     ((head->type == AXP_21274_SYS_BLK) &&
	      (head->size == _sys_blk_size)) ||
	     ((head->type == AXP_TELNET_SES_BLK) &&
	      (head->size == _ses_blk_size)) ||
	     ((head->type == AXP_DISK_BLK) &&
	      (head->size == _disk_blk_size)) ||
	     ((head->type == AXP_SSD_BLK) &&
	      (head->size == _ssd_blk_size)) ||
	     ((head->type == AXP_VHDX_BLK) &&
	      (head->size == _vhdx_blk_size)) ||
	     ((head->type == AXP_VOID_BLK) &&
	      (head->size >= _head_tail_size))) &&
	    (head->magicNumber == AXP_HD_MAGIC) &&
	    (tail->magicNumber == AXP_TL_MAGIC))
	{
	    retVal = head->type;
	}
    }

    /*
     * Return the block type back to the caller.
     */
    return(retVal);
}
