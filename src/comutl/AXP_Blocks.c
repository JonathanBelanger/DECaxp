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
					for (ii = 0; ii < (AXP_REG_MAP_SIZE); ii++)
					{
						cpu->prMap[ii].pr = ii;
						cpu->prMap[ii].prevPr = AXP_UNMAPPED_REG;
						cpu->prState[ii] = Valid;
						cpu->pfMap[ii].pr = ii;
						cpu->pfMap[ii].prevPr = AXP_UNMAPPED_REG;
						cpu->pfState[ii] = Valid;
					}

					/*
					 * The above loop initialized prMap and pfMap array entries
					 * from 0 to 30 to be mapped to the physical registers also
					 * from 0 to 30.  This next two lines of code initializes
					 * the mapping for R31 and F31 to be mapped to an invalid
					 * physical register.  This is used to indicate to the code
					 * that implements the Alpha AXP instructions that, as a
					 * source register is always a value of 0, and as a
					 * destination register, never updated.  This will greatly
					 * simplify the register (architectural and physical)
					 * handling.
					 */
					cpu->prMap[AXP_MAX_REGISTERS].pr = AXP_INT_PHYS_REG + 1;
					cpu->prMap[AXP_MAX_REGISTERS].prevPr = AXP_INT_PHYS_REG + 1;
					cpu->pfMap[AXP_MAX_REGISTERS].pr = AXP_FP_PHYS_REG + 1;
					cpu->pfMap[AXP_MAX_REGISTERS].prevPr = AXP_FP_PHYS_REG + 1;

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

					/*
					 * Initialize the instruction queue cache.  These are
					 * pre-allocated queue entries for the above.
					 */
					for (ii = 0; ii < AXP_IQ_LEN; ii++)
					{
						cpu->iqEFreelist[cpu->iqEFlEnd++] = ii;
						AXP_INIT_CQENTRY(cpu->iqEntries[ii].header, cpu->iq);
						cpu->iqEntries[ii].index = ii;
					}
					for (ii = 0; ii < AXP_FQ_LEN; ii++)
					{
						cpu->fqEFreelist[cpu->fqEFlEnd++] = ii;
						AXP_INIT_CQENTRY(cpu->fqEntries[ii].header, cpu->fq);
						cpu->fqEntries[ii].index = ii;
					}
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
