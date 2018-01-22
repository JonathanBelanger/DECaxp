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
 *	Integer Control functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		22-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		23-Jun-2017	Jonathan D. Belanger
 *	On branches, we need to save the branched to PC until the instruction is
 *	retired.  The original code was updating it immediately.  This would cause
 *	huge problems, especially around exception and interrupt handling.
 *
 *	V01.002		25-Jun-2017	Jonathan D. Belanger
 *	Change registers to be 64-bit structures to aid in coding.  Needed to
 *	update all the register references.
 */
#include "AXP_21264_Ebox_Control.h"
#include "AXP_21264_Ibox_PCHandling.h"

/*
 * AXP_BEQ
 *	This function implements the Control Branch if Register Equal to Zero
 *	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.sq == 0)
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
 * AXP_BGE
 *	This function implements the Control Branch if Register Greater than or
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
AXP_EXCEPTIONS AXP_BGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.sq >= 0)
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
 * AXP_BGT
 *	This function implements the Control Branch if Register Greater Than Zero
 *	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.sq > 0)
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
 * AXP_BLBC
 *	This function implements the Control Branch if Low Bit Is Clear instruction
 *	of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BLBC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if ((instr->src1v.r.uq & 0x01) == 0x00)
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
 * AXP_BLBS
 *	This function implements the Control Branch if Low Bit Is Set instruction
 *	of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BLBS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if ((instr->src1v.r.uq & 0x01) == 0x01)
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
 * AXP_BLE
 *	This function implements the Control Branch if Register Less Than or Equal
 *	to Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.sq <= 0)
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
 * AXP_BLT
 *	This function implements the Control Branch if Register Less Than Zero
 *	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.sq < 0)
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
 * AXP_BNE
 *	This function implements the Control Branch if Register Not Equal to Zero
 *	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	if (instr->src1v.r.uq != 0)
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
 * AXP_BR
 *	This function implements the Unconditional Branch  instruction of the Alpha
 *	AXP processor.
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
AXP_EXCEPTIONS AXP_BR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->branchPC = instr->pc;/* This points to the PC for this instruction. */
	instr->branchPC.pc++;		/* This points to the instruction after this. */
	instr->destv.r.uq = *((u64 *) &instr->branchPC);
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
 * AXP_BSR
 *	This function implements the Unconditional Branch  instruction of the Alpha
 *	AXP processor.
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
AXP_EXCEPTIONS AXP_BSR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 *
	 * TODO:	We need to use the hints for possible branch prediction and
	 *			push the return address onto a branch-prediction stack.
	 */
	instr->branchPC = instr->pc;	/* This points to the PC for this instruction. */
	instr->branchPC.pc++;			/* This points to the instruction after this. */
	instr->destv.r.uq = *((u64 *) &instr->branchPC);
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
 * AXP_JMP
 *	This function implements the Jump instructions of the Alpha AXP processor.
 *	This instruction is unique, in that the displacement field is used to
 *	contain the type of Jump is being performed.  The following table indicates
 *	who the displacement field will be interpretted:
 *
 *		disp<15:14>	Meaning			Predicted Target	Prediction Stack Action
 *		-----------	-------------	-------------------	------------------------------
 *		00			JMP				PC + (4*disp<13:0>)	--
 *		01			JSR				PC + (4*disp<13:0>)	Push return PC onto
 *														stack
 *		10			RET				Prediction stack	Pop new (return) PC
 *														from stack
 *		11			JSR_COROUTINE	Prediction stack	Pop, push return
 *
 *	For the last 2, RET(10) and JSR_COROUTINE(11), the encoding for disp<13:0>
 *	have the following meaining:
 *
 *		Encoding	Meaning
 *		--------	------------------------------
 *		0x0000		Indicates non-procedure return
 *		0x0001		Indicates procedure return
 *					All other encodings are reserved.
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
AXP_EXCEPTIONS AXP_JMP(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 *
	 * TODO:	We need to use the hints for possible branch prediction and
	 *			push/pop the return address onto a branch-prediction stack.
	 */
	instr->branchPC = instr->pc;	/* This points to the PC for this instruction. */
	instr->branchPC.pc++;			/* This points to the instruction after this. */
	instr->destv.r.uq = *((u64 *) &instr->branchPC);
	instr->branchPC = AXP_21264_GetVPC(cpu, instr->src1v.r.uq, AXP_NORMAL_MODE);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
