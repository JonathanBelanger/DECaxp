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
 *	functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		02-Nov-2017	Jonathan D. Belanger
 *	Coded the initialization code.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox.h"

/*
 * AXP_21264_Ebox_Init
 *	This function is called to initialize the Ebox.  It will set the IPRs
 *	associated with the Ebox to their initial/reset values.
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
bool AXP_21264_Ebox_Init(AXP_21264_CPU *cpu)
{
	bool	retVal = false;
	int		ii;

	/*
	 * This bit us used when emulating the RC and BC VAX Compatibility
	 * instructions used by VAX-to-Alpha translator software.  ARM 4.12
	 */
	cpu->VAXintrFlag = false;

	/*
	 * Set up the initial register map.  We do not map R31.
	 */
	for (ii = 0; ii < (AXP_MAX_REGISTERS-1); ii++)
	{
		cpu->pr[ii] = 0;
		cpu->prMap[ii].pr = 11;
		cpu->prMap[ii].prevPr = AXP_UNMAPPED_REG;
		cpu->prState[ii] = Valid;
	}

 
	/* 
	 * The above loop initialized the prMap array entries from 0 to 30 to be
	 * mapped to the physical registers also from 0 to 30.  The next two lines
	 * of code initialize the mapping for R31 to be mapped to an invalid 
	 * physical register.  This is used to indicate to the code that implements
	 * the Alpha AXP instructions that, as a source register is always a value
	 * of 0, and as a destination register, never updated.  This will greatly 
	 * simplify the register (architectural and physical) handling. 
	 */ 
	cpu->prMap[AXP_MAX_REGISTERS-1].pr = AXP_INT_PHYS_REG + 1; 
	cpu->prMap[AXP_MAX_REGISTERS-1].prevPr = AXP_INT_PHYS_REG + 1; 

	/*
	 * The remaining physical registers need to be put on the free list.
	 */
	cpu->prFlStart = 0;
	cpu->prFlEnd = 0;
	for (ii = AXP_MAX_REGISTERS; ii < AXP_INT_PHYS_REG; ii++)
	{
		cpu->prState[ii] = Free;
		cpu->prFreeList[cpu->prFlEnd++] = ii;
	}

	/*
	 * Initialize the instruction queue for the Ebox.
	 */
	AXP_INIT_CQUE(cpu->iq, AXP_IQ_LEN);

	/*
	 * Initialize the instruction queue cache.  These are pre-allocated queue
	 * entries for the above.
	 */
	for (ii = 0; ii < AXP_IQ_LEN; ii++)
	{
		cpu->iqEFreelist[cpu->iqEFlEnd++] = ii;
		AXP_INIT_CQENTRY(cpu->iqEntries[ii].header, cpu->iq);
		cpu->iqEntries[ii].index = ii;
	}

	/*
	 * Initialize the Ebox IPRs.
	 * NOTE: These will get real values from the PALcode.
	 */
	cpu->cc.counter = 0;
	cpu->cc.offset = 0;
	cpu->ccCtl.res_1 = 0;
	cpu->ccCtl.counter = 0;
	cpu->ccCtl.cc_ena = 0;
	cpu->ccCtl.res_2 = 0;
	cpu->va = 0;
	cpu->vaCtl.b_endian = 0;
	cpu->vaCtl.va_48 = 0;
	cpu->vaCtl.va_form_32 = 0;
	cpu->vaCtl.res = 0;
	cpu->vaCtl.vptb = 0;
	cpu->vaForm.form00.res = 0;
	cpu->vaForm.form00.va = 0;
	cpu->vaForm.form00.vptb = 0;

	return(retVal);
}
