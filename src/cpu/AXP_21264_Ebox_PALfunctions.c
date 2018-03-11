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
 *	PAL functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		09-Oct-2017	Jonathan D. Belanger
 *	Initially written.
 */

#include "AXP_Configure.h"
#include "AXP_21264_Ebox_PALFunctions.h"

/*
 * AXP_HWLD
 *	This function implements the PAL Load instruction of the Alpha AXP
 *	processor.
 *
 *	PALcode uses this instruction to access memory outside the realm of normal
 *	Alpha memory management and to perform special Dstream load transactions.
 *	Data alignment traps are disabled for the HW_LD instruction.
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
AXP_EXCEPTIONS AXP_HWLD(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_HWST
 *	This function implements the PAL Store instruction of the Alpha AXP
 *	processor.
 *
 *	PALcode uses this instruction to access memory outside the realm of normal
 *	Alpha memory management and to do special forms of Dstream store
 *	instructions.  Data alignment traps are inhibited for this instruction.
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
AXP_EXCEPTIONS AXP_HWST(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;
	u64 va;

	/*
	 * Implement the instruction.
	 */
	va = instr->src1v.r.uq + instr->displacement;

	AXP_21264_Mbox_WriteMem(
		cpu,
		instr,
		instr->slot,
		va,
		instr->src1v.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_HWRET
 *	This function implements the PAL Return instruction of the Alpha AXP
 *	processor.
 *
 *	This instruction is used to return instruction flow to a specified PC.  The
 *	Rb field of the instruction specifies and integer register, which holds the
 *	new value of the PC.  Bit[0] of this register provides the new value of
 *	PALmode after this instruction is executed.  Bits [15:14] determine the
 *	stack action.
 *
 *	Normally, this instruction succeeds a CALL_PAL instruction, or a trap
 *	handler that pushed its PC onto the prediction stack.  In this mode, the
 *	HINT field should be set to '10' to pop the PC and generate a predicted
 *	target address for this instruction.
 *
 *	In some conditions, this instruction is used in the middle of a PALcode
 *	flow to cause a group of instructions to retire.  In these cases, if this
 *	Instruction does not have a corresponding instruction that pushed a PC onto
 *	the stack, the HINT field should be set to '00 to keep the stack from being
 *	modified.
 *
 *	In the rare circumstances that this instruction might be used like a JSR or
 *	JSR_COROUTINE, the stack can be managed by setting the HINT bits
 *	accordingly.
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
AXP_EXCEPTIONS AXP_HWRET(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_PC			pc;

	/*
	 * Implement the HW_RET instruction.
	 */
	switch(instr->type_hint_index)
	{
		case AXP_HW_JMP:
			pc = instr->pc;
			pc.pc++;
			instr->branchPC = AXP_21264_DisplaceVPC(
										cpu,
										pc,
										instr->displacement);
			break;

		case AXP_HW_JSR:
			pc = instr->pc;
			pc.pc++;
			AXP_PUSH(pc);
			instr->branchPC = AXP_21264_DisplaceVPC(
										cpu,
										pc,
										instr->displacement);
			break;

		case AXP_HW_RET:
			AXP_POP(pc);
			instr->branchPC = AXP_21264_MakeVPC(
										cpu,
										instr->src2v.r.uq,
										(instr->src2v.r.uq & AXP_PAL_MODE));
			break;

		case AXP_HW_COROUTINE:
			pc = instr->pc;
			pc.pc++;
			AXP_SWAP(pc);
			instr->branchPC = AXP_21264_MakeVPC(cpu, *((u64 *) &pc), pc.pal);
			break;
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_HWMFPR
 *	This function implements the PAL Move from Internal Processor Register
 *	(IPR) instruction of the Alpha AXP processor.
 *
 *	This instruction reads the value from the specified IPR into the integer
 *	register specified by the Ra field of the instruction.
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
AXP_EXCEPTIONS AXP_HWMFPR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * There is nothing to do here.  Because instructions may be executed out
	 * of order, the only true view of an IPRs value is at the moment of
	 * instruction retirement.  This instruction is actually implemented in the
	 * retirement code in the Ibox.
	 *
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_HWMTPR
 *	This function implements the PAL Move to Internal Processor Register
 *	(IPR) instruction of the Alpha AXP processor.
 *
 *	This instruction writes the value from the integer register specified by
 *	the Rb field into the specified IPR.
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
AXP_EXCEPTIONS AXP_HWMTPR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * There is nothing to do here.  Because instructions may be executed out
	 * of order, a value of an IPR should only be stored at the moment of
	 * instruction retirement.  This instruction is actually implemented in the
	 * retirement code in the Ibox.
	 *
	 * Indicate that the instruction is ready to be retired.
	 *
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
