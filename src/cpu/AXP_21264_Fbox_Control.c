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
 *	Floating-Point Control functionality of the Fbox.
 *
 *	Revision History:
 *
 *	V01.000		24-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Fbox_Control.h"

/*
 * AXP_FBEQ
 *	This function implements the Floating-Point Control Branch if Register
 *	Equal to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if ((instr->src1v & ~AXP_R_SIGN) == 0)
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_FBGE
 *	This function implements the Floating-Point Control Branch if Register
 *	Greater than or Equal to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v <= AXP_R_SIGN)
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_FBGT
 *	This function implements the Floating-Point Control Branch if Register
 *	Greater Than Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (((instr->src1v & AXP_R_SIGN) == 0) && (instr->src1v != 0))
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_FBLE
 *	This function implements the Floating-Point Control Branch if Register Less
 *	Than or Equal to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (((instr->src1v & AXP_R_SIGN) == 1) && (instr->src1v == 0))
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_FBLT
 *	This function implements the Floating-Point Control Branch if Register Less
 *	Than Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v > AXP_R_SIGN)
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_FBNE
 *	This function implements the Floating-Point Control Branch if Register Not
 *	Equal to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FBNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if ((instr->src1v & ~AXP_R_SIGN) != 0)
		instr->branchPC = AXP_21264_DisplaceVPC(cpu, instr->displacement);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
