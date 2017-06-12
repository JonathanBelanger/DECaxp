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
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"

void *AXP_Allocate_Block(AXP_BLOCK_TYPE blockType)
{
	void	*retBlock = NULL;
	int		ii;

	switch (blockType)
	{
		case AXP_21264_CPU_BLK:
			{
				AXP_21264_CPU *cpu = NULL;

				retBlock = calloc(1, sizeof(AXP_21264_CPU));
				if (retBlock != NULL)
				{

					/*
					 * The calloc cleared all the memory.  We just have to set
					 * the non-zero values.
					 */
					cpu = (AXP_21264_CPU *) retBlock;
					cpu->header.type = blockType;
					cpu->header.size = sizeof(AXP_21264_CPU);

					/*
					 * The initial register mapping is 1 for one.  Also, the
					 * contents of the register are ready.
					 */
					for (ii = 0; ii < (AXP_MAX_REGISTERS - 1); ii++)
					{
						cpu->prMap[ii].pr = ii;
						cpu->prMap[ii].prevPr = AXP_UNMAPPED_REG;
						cpu->prState[ii] = Valid;
						cpu->pfMap[ii].pr = ii;
						cpu->pfMap[ii].prevPr = AXP_UNMAPPED_REG;
						cpu->pfState[ii] = Valid;
					}

					/*
					 * The remaining physical registers need to be put on the
					 * free-list.
					 */
					for (ii = AXP_MAX_REGISTERS; ii < AXP_INT_PHYS_REG; ii++)
					{
						cpu->prState[ii] = Free;
						cpu->prFreeList[cpu->prFlEnd++] = ii;
						if (ii < AXP_FP_PHYS_REG)
						{
							cpu->pfState[ii] = Free;
							cpu->pfFreeList[cpu->pfFlEnd++] = ii;
						}
					}

					/*
					 * Initialize the ReOrder Buffer (ROB).
					 */
					for (ii = 0; ii < AXP_INFLIGHT_MAX; ii++)
					{
						cpu->rob[ii].state = Retired;
					}

					/*
					 * Initialize the instruction queues.
					 */
					AXP_INIT_CQUE(cpu->iq, AXP_IQ_LEN);
					AXP_INIT_CQUE(cpu->fq, AXP_FQ_LEN);
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
	 * Clear the address specified on the call.
	 */
	block = NULL;
	return;
}
