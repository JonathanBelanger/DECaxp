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
 *
 *	V01.001		25-Jun-2017	Jonathan D. Belanger
 *	Registers are no longer just 64-bit values, but have been described as a
 *	structure to aid in coding (no more masks and shifts).
 *
 * TODO:	We need to look at handling Arithmetic Traps.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox_IntegerMath.h"

/*
 * AXP_multiply64
 * 	This function is called to multiply 2 64 bit values to produce a 128-bit
 * 	result.  The low-order 64-bits are returned on the call and the high
 * 	64-bits are return in the third parameter.
 *
 * Input Parameters:
 * 	a:
 * 		A 64-bit value containing the first of the two 64-bit values to
 * 		multiply together.
 * 	b:
 * 		A 64-bit value containing the second of the two 64-bit values to
 * 		multiply together.
 *
 * Output Parameters:
 * 	c:
 * 		A pointer to a 64-bit location to receive the high-order 64-bit
 * 		results.
 *
 * Return Value:
 * 	The low-oder 64-bits of the results.
 */
u64 AXP_multiply64(u64 a, u64 b, u64 *c)
{
	u64	lowResult, mid1Result, mid2Result, highResult;
	u64	aHighValue, aLowValue;
	u64	bHighValue, bLowValue;

	/*
	 * First off, separate the high and low 32-bits from the input parameters.
	 */
	aHighValue = AXP_LONG_MASK(a >> 32);
	aLowValue = AXP_LONG_MASK(a);
	bHighValue = AXP_LONG_MASK(b >> 32);
	bLowValue = AXP_LONG_MASK(b);

	/*
	 * Now, let's multiply the various parts of the input values.
	 */
	highResult = aHighValue * bHighValue;
	mid1Result = aHighValue * bLowValue;
	mid2Result = aLowValue * bHighValue;
	lowResult = aLowValue * bLowValue;

	/*
	 * Now, let's put them all together.
	 */
	highResult = highResult +
				 AXP_LONG_MASK(mid1Result >> 32) +
				 AXP_LONG_MASK(mid2Result >> 32);
	mid1Result = AXP_QUAD_MASK(mid1Result << 32);
	mid2Result = AXP_QUAD_MASK(mid2Result << 32);

	/*
	 * Now let's move some overflow values from low to high.
	 */
	lowResult = AXP_QUAD_MASK(lowResult + mid1Result);
	if (lowResult < mid1Result)
		highResult++;
	lowResult = AXP_QUAD_MASK(lowResult + mid2Result);
	if (lowResult < mid2Result)
		highResult++;

	/*
	 * Return the high 32-bits back in the output parameter.
	 */
	*c = AXP_QUAD_MASK(highResult);

	/*
	 * Return the low 32-bits back to the caller as a return value.
	 */
	return(lowResult);
}

/*
 * AXP_ADDL
 *	This function implements the Integer Arithmetic Longword Add without
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
AXP_EXCEPTIONS AXP_ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = AXP_SEXT_LONG(AXP_LONG_MASK(instr->src1v.r.uq + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ADDL_V
 *	This function implements the Integer Arithmetic Longword Add with overflow
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
AXP_EXCEPTIONS AXP_ADDL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq + Rbv;
	if (instr->destv.r.uq != instr->destv.r.ul)
		retVal = ArithmeticTraps; // IntegerOverflow;
	else
		instr->destv.r.sq = AXP_SEXT_LONG(instr->destv.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_ADDQ
 *	This function implements the Integer Arithmetic Quadword Add without
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
AXP_EXCEPTIONS AXP_ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_ADDQ_V
 *	This function implements the Integer Arithmetic Quadword Add with overflow
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
AXP_EXCEPTIONS AXP_ADDQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq + Rbv;
	if (((~instr->src1v.r.uq ^ Rbv) &
		 (instr->src1v.r.uq & instr->destv.r.uq)) &
		0x8000000000000000ll)
		retVal = ArithmeticTraps;	// IntegerOverflow;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_S4ADDL
 *	This function implements the Integer Arithmetic Scaled by 4 Longword Add
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
AXP_EXCEPTIONS AXP_S4ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.sq =
		AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v.r.uq * 4) + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8ADDL
 *	This function implements the Integer Arithmetic Scaled by 8 Longword Add
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
AXP_EXCEPTIONS AXP_S8ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.sq =
		AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v.r.uq * 8) + Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S4ADDQ
 *	This function implements the Integer Arithmetic Scaled by 4 Quadword Add
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
AXP_EXCEPTIONS AXP_S4ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (instr->src1v.r.uq * 4) + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8ADDQ
 *	This function implements the Integer Arithmetic Scaled by 8 Quadword Add
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
AXP_EXCEPTIONS AXP_S8ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (instr->src1v.r.uq * 8) + Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPEQ
 *	This function implements the Integer Arithmetic Compare Signed Quadword
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
AXP_EXCEPTIONS AXP_CMPEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v.r.sq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (i64) instr->src1v.r.sq == Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPLE
 *	This function implements the Integer Arithmetic Compare Signed Quadword
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
AXP_EXCEPTIONS AXP_CMPLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v.r.sq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (i64) instr->src1v.r.sq <= Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPLT
 *	This function implements the Integer Arithmetic Compare Signed Quadword
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
AXP_EXCEPTIONS AXP_CMPLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	i64 Rbv = (i64) (instr->useLiteral ? instr->literal : instr->src2v.r.sq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (i64) instr->src1v.r.sq < Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CMPULE
 *	This function implements the Integer Arithmetic Compare Unsigned Quadword
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
AXP_EXCEPTIONS AXP_CMPULE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v.r.uq;

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq <= Rbv ? 1 : 0;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CTLZ
 *	This function implements the Integer Arithmetic Count Leading Zero
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
AXP_EXCEPTIONS AXP_CTLZ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v.r.uq;
	u64 mostSigBit = 0x8000000000000000ll;

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = 0;
	while (mostSigBit != 0)
	{
		if ((Rbv & mostSigBit) != 0)
			break;
		instr->destv.r.uq++;
		mostSigBit >>= 1;
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_CTTZ
 *	This function implements the Integer Arithmetic Count Trailing Zero
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
AXP_EXCEPTIONS AXP_CTTZ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = instr->useLiteral ? instr->literal : instr->src2v.r.uq;
	u64 mostSigBit = 0x0000000000000001ll;

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = 0;
	while (mostSigBit != 0)
	{
		if ((Rbv & mostSigBit) != 0)
			break;
		instr->destv.r.uq++;
		mostSigBit <<= 1;
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULL
 *	This function implements the Integer Arithmetic Longword Multiply without
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
AXP_EXCEPTIONS AXP_MULL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.sq = AXP_SEXT_LONG(AXP_LONG_MASK(instr->src1v.r.uq * Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULL_V
 *	This function implements the Integer Arithmetic Longword Multiply with
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
AXP_EXCEPTIONS AXP_MULL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq * Rbv;
	if (instr->destv.r.uq != instr->destv.r.ul)
		retVal = ArithmeticTraps;	// IntegerOverflow;
	else
		instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_MULQ
 *	This function implements the Integer Arithmetic Quadword Multiply without
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
AXP_EXCEPTIONS AXP_MULQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq * Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_MULQ_V
 *	This function implements the Integer Arithmetic Quadword Multiply with
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
AXP_EXCEPTIONS AXP_MULQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);
	u64				highResult;

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = AXP_multiply64(instr->src1v.r.uq, Rbv, &highResult);
	if (instr->src1v.r.sq < 0)
		highResult -= instr->src2v.r.sq;
	if (instr->src2v.r.sq < 0)
		highResult -= instr->src1v.r.uq;
	if ((instr->destv.r.sq < 0) ?
		(highResult != 0xffffffffffffffff) :
		(highResult != 0))
		retVal = ArithmeticTraps;	// IntegerOverflow;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_UMULH
 *	This function implements the Integer Arithmetic Unsigned Quadword Multiply
 *	High instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_UMULH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	(void) AXP_multiply64(instr->src1v.r.uq, Rbv, &instr->destv.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SUBL
 *	This function implements the Integer Arithmetic Longword Subtract without
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
AXP_EXCEPTIONS AXP_SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = AXP_SEXT_LONG(AXP_LONG_MASK(instr->src1v.r.uq - Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SUBL_V
 *	This function implements the Integer Arithmetic Longword Subtract with
 *	overflow instruction f the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_SUBL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq - Rbv;
	if (instr->destv.r.uq != instr->destv.r.ul)
		retVal = ArithmeticTraps; // IntegerOverflow;
	else
		instr->destv.r.sq = AXP_SEXT_LONG(instr->destv.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SUBLQ
 *	This function implements the Integer Arithmetic Quadword Subtract without
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
AXP_EXCEPTIONS AXP_SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq - Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_SUBQ_V
 *	This function implements the Integer Arithmetic Quadword Subtract with
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
AXP_EXCEPTIONS AXP_SUBQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = instr->src1v.r.uq - Rbv;
	if (((~instr->src1v.r.uq ^ Rbv) &
		 (instr->src1v.r.uq & instr->destv.r.uq)) &
		0x8000000000000000ll)
		retVal = ArithmeticTraps;	// IntegerOverflow;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_S4SUBL
 *	This function implements the Integer Arithmetic Scaled by 4 Longword
 *	Subtract instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_S4SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.sq = AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v.r.uq * 4) - Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8SUBL
 *	This function implements the Integer Arithmetic Scaled by 8 Longword
 *	Subtract instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_S8SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.sq = AXP_SEXT_LONG(AXP_LONG_MASK((instr->src1v.r.uq * 8) - Rbv));

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S4SUBQ
 *	This function implements the Integer Arithmetic Scaled by 4 Quadword
 *	Subtract instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_S4SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (instr->src1v.r.uq * 4) - Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}

/*
 * AXP_S8SUBQ
 *	This function implements the Integer Arithmetic Scaled by 8 Quadword
 *	Subtract instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_S8SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	instr->destv.r.uq = (instr->src1v.r.uq * 8) - Rbv;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
