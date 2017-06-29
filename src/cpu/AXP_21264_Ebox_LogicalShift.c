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
 *	Integer Logical and Shift functionality of the Ebox.
 *
 * Revision History:
 *
 *	V01.000		23-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox_LogicalShift.h"

/*
 * AXP_AND
 *	This function implements the Integer Logical And instruction of the Alpha
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
AXP_EXCEPTIONS AXP_AND(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq & Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_BIS
 *	This function implements the Integer Logical Or instruction of the Alpha
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
AXP_EXCEPTIONS AXP_BIS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq | Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_XOR
 *	This function implements the Integer Logical Exclusive Or instruction of
 *	the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_XOR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq ^ Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_BIC
 *	This function implements the Integer Logical And Not instruction of the
 *	Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_BIC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq & ~Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ORNOT
 *	This function implements the Integer Logical Or Not instruction of the
 *	Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_ORNOT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq | ~Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_EQV
 *	This function implements the Integer Logical Equivalence instruction of the
 *	Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_EQV(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq ^ ~Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVEQ
 *	This function implements the Integer Conditional Move if Equal instruction
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
AXP_EXCEPTIONS AXP_CMOVEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq == 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVGE
 *	This function implements the Integer Conditional Move if Greater Than or\
 *	Equal instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CMOVGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq >= 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVGT
 *	This function implements the Integer Conditional Move if Greater Than
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
AXP_EXCEPTIONS AXP_CMOVGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq > 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVLBC
 *	This function implements the Integer Conditional Move if Low Bit Clear
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
AXP_EXCEPTIONS AXP_CMOVLBC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.r.uq & 0x01) == 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVLBS
 *	This function implements the Integer Conditional Move if Low Bit Set
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
AXP_EXCEPTIONS AXP_CMOVLBS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.r.uq & 0x01) == 1)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVLE
 *	This function implements the Integer Conditional Move if Less Than or Equal
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
AXP_EXCEPTIONS AXP_CMOVLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq <= 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVLT
 *	This function implements the Integer Conditional Move if Less Than
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
AXP_EXCEPTIONS AXP_CMOVLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq < 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMOVNE
 *	This function implements the Integer Conditional Move if Not Equal
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
AXP_EXCEPTIONS AXP_CMOVNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.r.sq != 0)
		instr->destv.r.uq = Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SLL
 *	This function implements the Integer Shift Left Logical instruction
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
AXP_EXCEPTIONS AXP_SLL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq << (Rbv & 0x000000000000003fll);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SRL
 *	This function implements the Integer Shift Right Logical instruction
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
AXP_EXCEPTIONS AXP_SRL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq >> (Rbv & 0x000000000000003fll);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SRA
 *	This function implements the Integer Shift Right Arithmetical instruction
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
AXP_EXCEPTIONS AXP_SRA(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	Rbv &= 0x000000000000003fll;
	instr->destv.r.uq = instr->src1v.r.uq >> Rbv;
	if ((Rbv != 0) && ((instr->src1v.r.uq & 0x8000000000000000ll) != 0))
		instr->destv.r.uq |= 0xffffffffffffffffll << (64 - Rbv);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
