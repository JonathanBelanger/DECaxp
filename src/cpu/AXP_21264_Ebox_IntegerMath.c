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
 * 	NoException:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32		Rav = instr->src1v.r.ul;
	u32		Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32		Rcv;

	/*
	 * Implement the instruction (we ignore the overflow)
	 */
	__builtin_add_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_ADDL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u32		Rav = instr->src1v.r.ul;
	u32		Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32		Rcv;

	/*
	 * Implement the instruction.
	 */
	if (__builtin_add_overflow(Rav, Rbv, &Rcv))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	__builtin_add_overflow(Rav, Rbv, &instr->destv.r.uq);

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_ADDQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64				Rav = instr->src1v.r.uq;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (__builtin_add_overflow(Rav, Rbv, &instr->destv.r.uq ))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S4ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	Rav <<= 2;
	__builtin_add_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.sq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S8ADDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	Rav <<= 3;
	__builtin_add_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.sq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S4ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	Rav <<= 2;
	__builtin_add_overflow(Rav, Rbv, &instr->destv.r.sq);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S8ADDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	Rav <<= 3;
	__builtin_add_overflow(Rav, Rbv, &instr->destv.r.sq);

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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MULL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	__builtin_mul_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.sq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MULL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	if (__builtin_mul_overflow(Rav, Rbv, &Rcv))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}
	instr->destv.r.sq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MULQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);
	u64 Rcv;

	/*
	 * Implement the instruction
	 */
	__builtin_mul_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.sq = Rcv;

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_MULQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (__builtin_mul_overflow(Rav, Rbv, &instr->destv.r.sq))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_UMULH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64				Rav = instr->src1v.r.uq;
	u64				Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);
	u128			Rcv;

	/*
	 * Implement the instruction
	 */
	__builtin_mul_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.uq = (u64) ((Rcv >> 64) & AXP_LOW_QUAD);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	__builtin_sub_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_SUBL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	if (__builtin_sub_overflow(Rav, Rbv, &Rcv))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SUBQ
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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	__builtin_sub_overflow(Rav, Rbv, &instr->destv.r.uq);

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_SUBQ_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	if (__builtin_sub_overflow(Rav, Rbv, &instr->destv.r.uq))
	{
		retVal = ArithmeticTraps;
		AXP_SetException(instr, AXP_EXC_INT_OVERFLOW);
	}

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S4SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	Rav <<= 2;
	__builtin_sub_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S8SUBL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u32 Rav = instr->src1v.r.ul;
	u32 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.ul);
	u32 Rcv;

	/*
	 * Implement the instruction
	 */
	Rav <<= 3;
	__builtin_sub_overflow(Rav, Rbv, &Rcv);
	instr->destv.r.uq = AXP_SEXT_LONG(Rcv);

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
 * 	NoException:		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_S4SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	Rav <<= 2;
	__builtin_sub_overflow(Rav, Rbv, &instr->destv.r.uq);

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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Integer Overflow.
 */
AXP_EXCEPTIONS AXP_S8SUBQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 Rav = instr->src1v.r.uq;
	u64 Rbv = AXP_LONG_MASK(instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Implement the instruction
	 */
	Rav <<= 3;
	__builtin_sub_overflow(Rav, Rbv, &instr->destv.r.uq);

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
