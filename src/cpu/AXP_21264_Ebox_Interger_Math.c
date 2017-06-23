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
 *	Integer Arithmetic functionality of the Ebox.
 *
 * Revision History:
 *
 *	V01.000		23-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
 #include "AXP_21264_Ebox_Integer_Math.h"

/*
 * AXP_ADDL
 *	This function implements the Interger Arithmetic Longword Add without
 *	overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = AXP_SEXT_LONG(AXP_LONG_MASK(instr->src1v + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ADDL_V
 *	This function implements the Interger Arithmetic Longword Add with overflow
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
AXP_EXCEPTIONS AXP_ADDL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v + Rbv;
	if (instr->destv != AXP_LONG_MASK(instr->destv))
		retVal = IntegerOverflow;
	else
		instr->destv = AXP_SEXT_LONG(instr->destv);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_ADDQ
 *	This function implements the Interger Arithmetic Quadword Add without
 *	overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ADDQ_V
 *	This function implements the Interger Arithmetic Quadword Add with overflow
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
AXP_EXCEPTIONS AXP_ADDQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v + Rbv;
	if (((~instr->src1v ^ Rbv) & (instr->src1v & instr->destv)) & 0x8000000000000000ll)
		retVal = IntegerOverflow;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_S4ADDL
 *	This function implements the Interger Arithmetic Scaled by 4 Longword Add
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
AXP_EXCEPTIONS AXP_S4ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v * 4) + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8ADDL
 *	This function implements the Interger Arithmetic Scaled by 8 Longword Add
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
AXP_EXCEPTIONS AXP_S*ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v * 8) + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S4ADDQ
 *	This function implements the Interger Arithmetic Scaled by 4 Quadword Add
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
AXP_EXCEPTIONS AXP_S4ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = (instr->src1v * 4) + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8ADDQ
 *	This function implements the Interger Arithmetic Scaled by 8 Quadword Add
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
AXP_EXCEPTIONS AXP_S4ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = (instr->src1v * 8) + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPEQ
 *	This function implements the Interger Arithmetic Compare Signed Quadword
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
AXP_EXCEPTIONS AXP_CMPEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = (i64) instr->src1v == Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPLE
 *	This function implements the Interger Arithmetic Compare Signed Quadword
 *	Less Than or Equal instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CMPLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = (i64) instr->src1v <= Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPLT
 *	This function implements the Interger Arithmetic Compare Signed Quadword
 *	Less Than instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CMPLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = (i64) instr->src1v < Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPULE
 *	This function implements the Interger Arithmetic Compare Unsigned Quadword
 *	Less Than or Equal instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CMPULE(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v;

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v <= Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CTLZ
 *	This function implements the Interger Arithmetic Count Leading Zero
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
AXP_EXCEPTIONS AXP_CTLZ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v;

	/*
	 * Implement the instruction
	 */
	instr->destv = 0;
	while (mostSigBit != 0)
	{
		if (Rbv & mostSigBit != 0)
			break;
		instr->destv++;
		mostSigBit >>= 1;
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CTTZ
 *	This function implements the Interger Arithmetic Count Trailing Zero
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
AXP_EXCEPTIONS AXP_CTTZ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v;
	u64 mostSigBit = 0x0000000000000001ll;

	/*
	 * Implement the instruction
	 */
	instr->destv = 0;
	while (mostSigBit != 0)
	{
		if (Rbv & mostSigBit != 0)
			break;
		instr->destv++;
		mostSigBit <<= 1;
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULL
 *	This function implements the Interger Arithmetic Longword Multiply without
 *	overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MULL(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = AXP_SEXT_LONG(AXP_LONG_MASK(instr->src1v * Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULL_V
 *	This function implements the Interger Arithmetic Longword Multiply with
 *	overflow of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MULL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v * Rbv;
	if (instr->destv != AXP_LONG_MASK(instr->destv))
		retVal = IntegerOverflow;
	else
		instr->destv = AXP_SEXT_LONG(instr->destv);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_MULQ
 *	This function implements the Interger Arithmetic Quadword Multiply without
 *	overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MULQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v * Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULQ_V
 *	This function implements the Interger Arithmetic Quadword Multiply with
 *	overflow of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MULQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v);

	/*
	 * Implement the instruction
	 */
	instr->destv = instr->src1v * Rbv;
	if (((~instr->src1v ^ Rbv) & (instr->src1v & instr->destv)) & 0x8000000000000000ll)
		retVal = IntegerOverflow;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
