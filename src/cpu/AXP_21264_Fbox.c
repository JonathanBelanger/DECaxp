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
 *	This source file contains the functions needed to implement the
 *	functionality of the Fbox.
 *
 *	Revision History:
 *
 *	V01.000		19-June-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	v01.001		03-Nov-2017		Jonathan D. Belanger
 *	Added Fbox initialization code.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox.h"

/*
 * AXP_21264_Fbox_Init(AXP_21264_CPU *cpu)
 *	This function is called to initialize the Fbox.  The Fbox only contains one
 *	IPR, but there are other items that also need initialization.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to perform all initialization processing.
 *	false:	Normal Successful Completion.
 */
bool AXP_21264_Fbox_Init(AXP_21264_CPU *cpu)
{
	bool		retVal = false;
	int			ii;

	/*
	 * Initlaize the one Fbox IPR.
	 */
	cpu->fpcr.res = 0;
	cpu->fpcr.dnz = 0;
	cpu->fpcr.invd = 0;
	cpu->fpcr.dzed = 0;
	cpu->fpcr.ovfd = 0;
	cpu->fpcr.inv = 0;
	cpu->fpcr.dze = 0;
	cpu->fpcr.ovf = 0;
	cpu->fpcr.unf = 0;
	cpu->fpcr.ine = 0;
	cpu->fpcr.iov = 0;
	cpu->fpcr.dyn = 0;
	cpu->fpcr.undz = 0;
	cpu->fpcr.unfd = 0;
	cpu->fpcr.ined = 0;
	cpu->fpcr.sum = 0;

	/*
	 * Set up the initial register map.  We do not map F31.
	 */
	for (ii = 0; ii < (AXP_MAX_REGISTERS-1); ii++)
	{
		cpu->pf[ii] = 0;
		cpu->pfMap[ii].pr = ii;
		cpu->pfMap[ii].prevPr = AXP_UNMAPPED_REG;
		cpu->pfState[ii] = Valid;
	}


	/*
	 * The above loop initialized the pfMap array entries from 0 to 30 to be
	 * mapped to the physical registers also from 0 to 30.  The next lines of
	 * code initialize the mapping for F31 to be mapped to an invalid physical
	 * register.  This is used to indicate to the code that implements the
	 * Alpha AXP instructions that, as a source register is always a value
	 * of 0, and as a destination register, never updated.  This will greatly
	 * simplify the register (architectural and physical) handling.
	 */
	cpu->pfMap[AXP_MAX_REGISTERS-1].pr = AXP_FP_PHYS_REG + 1;
	cpu->pfMap[AXP_MAX_REGISTERS-1].prevPr = AXP_FP_PHYS_REG + 1;

	/*
	 * The remaining physical registers need to be put on the free list.
	 */
	cpu->pfFlStart = 0;
	cpu->pfFlEnd = 0;
	for (ii = AXP_MAX_REGISTERS; ii < AXP_FP_PHYS_REG; ii++)
	{
		cpu->pfState[ii] = Free;
		cpu->pfFreeList[cpu->pfFlEnd++] = ii;
	}

	/*
	 * Initialize the instruction queue for the Fbox.
	 */
	AXP_INIT_CQUE(cpu->fq, AXP_IQ_LEN);

	/*
	 * Initialize the instruction queue cache.  These are pre-allocated queue
	 * entries for the above.
	 */
	for (ii = 0; ii < AXP_FQ_LEN; ii++)
	{
		cpu->fqEFreelist[cpu->fqEFlEnd++] = ii;
		AXP_INIT_CQENTRY(cpu->fqEntries[ii].header, cpu->fq);
		cpu->fqEntries[ii].index = ii;
	}

	/*
	 * Return a success back to the caller.
	 */
	return(retVal);
}
