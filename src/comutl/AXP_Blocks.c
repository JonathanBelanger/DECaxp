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
#include "AXP_21264_CPU.h"
#include "AXP_21274_System.h"

void *AXP_Allocate_Block(AXP_BLOCK_TYPE blockType)
{
	void	*retBlock = NULL;

	switch (blockType)
	{
		case AXP_21264_CPU_BLK:
			{
				AXP_21264_CPU *cpu = NULL;

				retBlock = calloc(1, sizeof(AXP_21264_CPU));
				if (retBlock != NULL)
				{

					/*
					 * The calloc cleared all the memory.
					 */
					cpu = (AXP_21264_CPU *) retBlock;
					cpu->header.type = blockType;
					cpu->header.size = sizeof(AXP_21264_CPU);
					cpu->cpuState = Cold;
				}
			}
			break;

		case AXP_21274_SYS_BLK:
			{
				AXP_21274_SYSTEM *sys = NULL;

				retBlock = calloc(1, sizeof(AXP_21274_SYSTEM));
				if (retBlock != NULL)
				{
					sys = (AXP_21274_SYSTEM *) retBlock;
					sys->header.type = blockType;
					sys->header.size = sizeof(AXP_21274_SYSTEM);
				}
			}
			break;

		default:
			break;
	}
	return(retBlock);
}

void AXP_Deallocate_Block(AXP_BLOCK_DSC *block)
{
	switch (block->type)
	{
		case AXP_21264_CPU_BLK:
			free(block);
			break;

		default:
			break;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}
