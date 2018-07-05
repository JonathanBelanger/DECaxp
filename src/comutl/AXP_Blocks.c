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
    "VHDX"
};

void *AXP_Allocate_Block(AXP_BLOCK_TYPE blockType)
{
    void	*retBlock = NULL;
    size_t	size;

    switch (blockType)
    {
	case AXP_21264_CPU_BLK:
	    {
		AXP_21264_CPU *cpu = NULL;

		size = sizeof(AXP_21264_CPU);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{

		    /*
		     * The calloc cleared all the memory.
		     */
		    cpu = (AXP_21264_CPU *) retBlock;
		    cpu->header.type = blockType;
		    cpu->header.size = size;
		    cpu->cpuState = Cold;
		}
	    }
	    break;

	case AXP_21274_SYS_BLK:
	    {
		AXP_21274_SYSTEM *sys = NULL;

		size = sizeof(AXP_21274_SYSTEM);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{
		    sys = (AXP_21274_SYSTEM *) retBlock;

		    sys->header.type = blockType;
		    sys->header.size = size;
		}
	    }
	    break;

	case AXP_TELNET_SES_BLK:
	    {
		AXP_TELNET_SESSION *ses = NULL;

		size = sizeof(AXP_TELNET_SESSION);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{
		    ses = (AXP_TELNET_SESSION *) retBlock;

		    ses->header.type = blockType;
		    ses->header.size = size;
		}
	    }
	    break;

	case AXP_DISK_BLK:
	    {
		AXP_Disk *dsk = NULL;

		size = sizeof(AXP_Disk);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{
		    dsk = (AXP_Disk *) retBlock;

		    dsk->header.type = blockType;
		    dsk->header.size = size;
		}
	    }
	    break;

	case AXP_SSD_BLK:
	    {
		AXP_SSD_Handle *ssd = NULL;

		size = sizeof(AXP_SSD_Handle);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{
		    ssd = (AXP_SSD_Handle *) retBlock;

		    ssd->header.type = blockType;
		    ssd->header.size = size;
		}
	    }
	    break;

	case AXP_VHDX_BLK:
	    {
		AXP_VHDX_Handle *vhdx = NULL;

		size = sizeof(AXP_VHDX_Handle);
		retBlock = calloc(1, size);
		if (retBlock != NULL)
		{
		    vhdx = (AXP_VHDX_Handle *) retBlock;

		    vhdx->header.type = blockType;
		    vhdx->header.size = size;
		}
	    }
	    break;

	default:
	    break;
    }

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "AXP_Allocate_Block allocated %s of size %d at 0x%016llx",
	    blockNames[blockType],
	    size,
	    retBlock);
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller with the block allocated.
     */
    return (retBlock);
}

void AXP_Deallocate_Block(AXP_BLOCK_DSC *block)
{

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
	    "AXP_Deallocate_Block deallocating %s of size %d at 0x%016llx",
	    blockNames[block->type],
	    block->size,
	    block);
	AXP_TRACE_END();
    }

    /*
     * Deallocate the block based on its type.
     */
    switch (block->type)
    {
	case AXP_21264_CPU_BLK:
	case AXP_21274_SYS_BLK:
	case AXP_TELNET_SES_BLK:
	    free(block);
	    break;

	case AXP_SSD_BLK:
	    {
		AXP_SSD_Handle *ssd = (AXP_SSD_Handle *) block;

		if (ssd->memory != NULL)
		    free(ssd->memory);
		free(block);
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
		free(block);
	    }
	    break;

	case AXP_DISK_BLK:
	    {
		AXP_Disk *dsk = (AXP_Disk *) block;

		if (dsk->ssd != NULL)
		    AXP_Deallocate_Block((AXP_BLOCK_DSC *) dsk->ssd);
		else if (dsk->vhdx != NULL)
		    AXP_Deallocate_Block((AXP_BLOCK_DSC *) dsk->vhdx);
		free(block);
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
