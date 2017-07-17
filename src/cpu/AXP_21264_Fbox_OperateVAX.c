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
 *	functionality of the Fbox Operate Instructions.
 *
 *	Revision History:
 *
 *	V01.000		24-June-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		07-Jul-2017	Jonathan D. Belanger
 *	Renamed and left just the VAX instructions implemented within.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox_OperateVAX.h"

/*
 * AXP_ADDF
 *	This function implements the VAX F Format Floating-Point ADD instruction of
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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_ADDF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Cast the register values into doubles (no conversion required).
	 	 */
		src1v = (double) instr->src1v.fp.uq;
		src2v = (double) instr->src2v.fp.uq;

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v + src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Recast the result into the destination register.  Since this is
			 * a 32-bit value, don't forget to clear the bits that are supposed
			 * to be zero.
			 */
			instr->destv.fp.uq = (u64) destv;
			instr->destv.fp.fpr32.zero = 0;

			/*
			 * Before we can simply return back to the caller, we need to
			 * determine if an overflow condition may have occurred.
			 */
			if ((instr->destv.fp.fpr.exponent - AXP_T_BIAS) > AXP_F_BIAS)
				raised = FE_OVERFLOW;
			else
				switch (AXP_FP_ENCODE(&instr->destv.fp.fpr, false))
				{

					/*
					 * These 2 cases are the same as Denormal for IEEE.
					 * Basically, these are values that cannot be represented
					 * in VAX Float.
					 */
					case DirtyZero:
					case Reserved:
						raised = FE_UNDERFLOW;
						break;

					/*
					 * These are just fine.  Nothing more to do here.
					 */
					case Finite:
					case Zero:
						break;

					/*
					 * These are not returned when IEEE is set to false above.
					 * So, there's nothing we can do here.  This is done to
					 * keep the compiler happy (it does not like switch
					 * statements with an enumeration and not all the values
					 * are present).
					 */
					case Denormal:
					case Infinity:
					case NotANumber:
						break;
				}
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_ADDG
 *	This function implements the VAX G Format Floating-Point ADD instruction of
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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_ADDG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	long double		src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Need to convert from 64-bit to 128-bit.  We need to do this because,
	 	 * the VAX G exponent can go up to 1024, but the IEEE T exponent can
	 	 * only go up to 1023.  We need to expand the exponent to a 15-bit
	 	 * representation from an 11-bit one, to accommodate this difference.
	 	 */
		AXP_FP_CvtG2X(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr, &src1v, &src2v);

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v + src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Convert the result back into a VAX G format and see if we got
			 * an overflow or underflow.
			 */
			raised = AXP_FP_CvtX2G(&destv, NULL, &instr->destv.fp.fpr, NULL);
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CMPGEQ
 *	This function implements the VAX G Format Floating-Point Compare Equal
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 */
AXP_EXCEPTIONS AXP_CMPGEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if (instr->src1v.fp.uq == instr->src2v.fp.uq)
		instr->destv.fp.uq = AXP_G_HALF;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CMPGLE
 *	This function implements the VAX G Format Floating-Point Compare Less Than
 *	or Equal instruction of the Alpha AXP processor.
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 */
AXP_EXCEPTIONS AXP_CMPGLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if ((instr->src1v.fp.uq == instr->src2v.fp.uq) ||
			 (instr->src1v.fp.fpr.sign > instr->src2v.fp.fpr.sign) ||
			 (((instr->src1v.fp.uq == instr->src2v.fp.uq) ^
			   instr->src1v.fp.fpr.sign) != 0))
		instr->destv.fp.uq = AXP_G_HALF;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CMPGLT
 *	This function implements the VAX G Format Floating-Point Compare Less Than
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 */
AXP_EXCEPTIONS AXP_CMPGLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if ((instr->src1v.fp.fpr.sign > instr->src2v.fp.fpr.sign) ||
			 (((instr->src1v.fp.uq == instr->src2v.fp.uq) ^
			   instr->src1v.fp.fpr.sign) != 0))
		instr->destv.fp.uq = AXP_G_HALF;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CVTGQ
 *	This function implements the convert from VAX G Format Floating-Point to
 *	Integer instruction of the Alpha AXP processor.
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 * 							Integer Overflow
 */
AXP_EXCEPTIONS AXP_CVTGQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	i32				unbiasedExp = instr->src1v.fp.fpr.exponent - AXP_G_BIAS;
	u64				convertedValue = 0;
	u32				sign = instr->src1v.fp.fpr.sign;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, NULL))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
		 * If the unbiased exponent is less then zero, then its too small to be
		 * converted to an integer, just set the fraction to zero and make sure
		 * the sign is "+".
		 */
		if (unbiasedExp < 0)
		{
			convertedValue = 0;
			sign = 0;
		}

		/*
		 * If the unbiased exponent is in range, then  perform the conversion.
		 */
		else if (unbiasedExp <= AXP_R_NMBIT)
		{

			/*
			 * Use the unbiased exponent, offset by the normalized bit location
			 * to shift the fraction to its integer representation.
			 */
			convertedValue =
				instr->src1v.fp.fpr.fraction >> (AXP_R_NMBIT - unbiasedExp);

			/*
			 * If rounding mode is not chopped, then add 1 to the converted
			 * value.
			 */
			if (fpFunc->rnd != AXP_FP_CHOPPED)
				convertedValue++;

			/*
			 * Justify the converted value, after rounding.
			 */
			convertedValue = convertedValue >> 1;

			/*
			 * If the converted value is too large to store in an integer, then
			 * we have an Overflow condition.
			 */
			if ((convertedValue > (sign == 1 ? AXP_Q_NEGMAX : AXP_Q_POSMAX)) &&
				((fpFunc->trp & AXP_FP_TRP_V) != 0))
			{
				retVal = ArithmeticTraps;
				raised = FE_OVERFLOW;
			}
		}
		else
		{

			/*
			 * If the unbiased exponent is out of range, then the converted
			 * value is set to zero.  Otherwise, shift the fraction the other
			 * way.  No matter what, we have an overflow condition.
			 */
			if (unbiasedExp > (AXP_R_NMBIT + 64))
				convertedValue = 0;
			else
				convertedValue =
					instr->src1v.fp.fpr.fraction <<
						(unbiasedExp - AXP_R_NMBIT - 1);
			if ((fpFunc->trp & AXP_FP_TRP_V) != 0)
			{
				retVal = ArithmeticTraps;
				raised = FE_OVERFLOW;
			}
		}

		/*
		 * Store the converted value into the destination register.
		 */
		instr->destv.fp.uq =
			((sign == 1) ? ((~convertedValue) + 1) : convertedValue);
	}

	/*
	 * Set the exception bits.
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, true);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CVTQF
 *	This function implements the convert from Quadword integer to VAX F Format
 *	Floating-Point instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTQF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Initialize various fields in the destination register.
	 */
	instr->destv.fp.fpr32.sign = instr->src2v.fp.q.sign;
	instr->destv.fp.fpr32.zero = 0;

	/*
	 * If the value to convert is already zero, then just set it and forget it.
	 * Otherwise, if it is signed, complement the value, else copy it into the
	 * destination register.
	 */
	if (instr->src1v.fp.uq == 0)
		instr->destv.fp.uq = AXP_FPR_ZERO;
	else if (instr->src2v.fp.q.sign == 1)
		instr->destv.fp.fpr32.fraction = ~instr->src1v.fp.q.integer;
	else
		instr->destv.fp.fpr32.fraction = instr->src1v.fp.q.integer;

	/*
	 * Normalize the destination register.
	 */
	AXP_FP_fpNormalize(&instr->destv.fp.fpr);

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
 * AXP_CVTQG
 *	This function implements the convert from Quadword integer to VAX G Format
 *	Floating-Point instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTQG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Initialize various fields in the destination register.
	 */
	instr->destv.fp.fpr.sign = instr->src2v.fp.q.sign;

	/*
	 * If the value to convert is already zero, then just set it and forget it.
	 * Otherwise, if it is signed, complement the value, else copy it into the
	 * destination register.
	 */
	if (instr->src1v.fp.uq == 0)
		instr->destv.fp.uq = AXP_FPR_ZERO;
	else if (instr->src2v.fp.q.sign == 1)
		instr->destv.fp.fpr.fraction = ~instr->src1v.fp.q.integer;
	else
		instr->destv.fp.fpr.fraction = instr->src1v.fp.q.integer;

	/*
	 * Normalize the destination register.
	 */
	AXP_FP_fpNormalize(&instr->destv.fp.fpr);

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
 * AXP_CVTDG
 *	This function implements the convert from VAX D Format Floating-Point to
 *	VAX G Float instruction of the Alpha AXP processor.
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_CVTDG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS		retVal = NoException;
	AXP_FP_FUNC			*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING 	encoding;
	u64					fraction = instr->src1v.fp.fdr.fraction;
	i32					exponent = instr->src1v.fp.fdr.exponent;
	int					raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	encoding = AXP_FP_ENCODE(&instr->src1v.fp.fdr, false);
	if ((encoding == Reserved) || (encoding == DirtyZero))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if (instr->src1v.fp.uq == 0)
		instr->destv.fp.uq = AXP_FPR_ZERO;
	else
	{
		u32 sign = instr->src1v.fp.fdr.sign;

		if (fpFunc->rnd != AXP_FP_CHOPPED)
		{

			fraction += AXP_G_RND;

			/*
			 * TODO: This code seems wrong to me.
			 */
			if ((fraction & AXP_R_NM) == 0)
			{
				fraction = (fraction >> 1) | AXP_R_NM;
				exponent++;
			}
		}
		if (exponent > AXP_G_EXP_MASK)		// the mask also is the max
		{
			retVal = ArithmeticTraps;
			raised = FE_OVERFLOW;
			exponent = AXP_G_EXP_MASK;
		}
		else if (exponent < 0)
		{
			if ((fpFunc->trp & AXP_FP_TRP_U) != 0)
			{
				retVal = ArithmeticTraps;
				raised = FE_UNDERFLOW;
			}
			sign = fraction = exponent = 0;
		}
		instr->destv.fp.fpr.sign = sign;
		instr->destv.fp.fpr.exponent = exponent;
		instr->destv.fp.fpr.fraction = (fraction >> 3) & AXP_R_FRAC;
	}

	/*
	 * Set the exception bits.
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, true);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CVTGD
 *	This function implements the convert from VAX G Format Floating-Point to
 *	VAX D Float instruction of the Alpha AXP processor.
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_CVTGD(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS		retVal = NoException;
	AXP_FP_FUNC			*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING 	encoding;
	u64					fraction = instr->src1v.fp.fpr.fraction;
	i32					exponent = instr->src1v.fp.fpr.exponent;
	int					raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	encoding = AXP_FP_ENCODE(&instr->src1v.fp.fpr, false);
	if ((encoding == Reserved) || (encoding == DirtyZero))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if (instr->src1v.fp.uq == 0)
		instr->destv.fp.uq = AXP_FPR_ZERO;
	else
	{
		u32 sign = instr->src1v.fp.fpr.sign;

		if (fpFunc->rnd != AXP_FP_CHOPPED)
		{

			fraction += AXP_D_RND;

			/*
			 * TODO: This code seems wrong to me.
			 */
			if ((fraction & AXP_R_NM) == 0)
			{
				fraction = (fraction >> 1) | AXP_R_NM;
				exponent++;
			}
		}
		if (exponent > AXP_D_EXP_MASK)		// the mask also is the max
		{
			retVal = ArithmeticTraps;
			raised = FE_OVERFLOW;
			exponent = AXP_D_EXP_MASK;
		}
		else if (exponent < 0)
		{
			if ((fpFunc->trp & AXP_FP_TRP_U) != 0)
			{
				retVal = ArithmeticTraps;
				raised = FE_UNDERFLOW;
			}
			sign = fraction = exponent = 0;
		}
		instr->destv.fp.fdr.sign = sign;
		instr->destv.fp.fdr.exponent = exponent;
		instr->destv.fp.fdr.fraction = (fraction << 3) & AXP_R_FRAC;
	}

	/*
	 * Set the exception bits.
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, true);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_CVTGF
 *	This function implements the convert from VAX G Format Floating-Point to
 *	VAX F Float instruction of the Alpha AXP processor.
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
 * 	IllegalOperand:		An illegal operand trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_CVTGF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS		retVal = NoException;
	AXP_FP_FUNC			*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING 	encoding;
	u64					fraction = instr->src1v.fp.fdr.fraction;
	i32					exponent = instr->src1v.fp.fdr.exponent;
	int					raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	encoding = AXP_FP_ENCODE(&instr->src1v.fp.fpr, false);
	if ((encoding == Reserved) || (encoding == DirtyZero))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if (instr->src1v.fp.uq == 0)
		instr->destv.fp.uq = AXP_FPR_ZERO;
	else
	{
		u32 sign = instr->src1v.fp.fpr.sign;

		if (fpFunc->rnd != AXP_FP_CHOPPED)
		{

			fraction += AXP_F_RND;

			/*
			 * TODO: This code seems wrong to me.
			 */
			if ((fraction & AXP_R_NM) == 0)
			{
				fraction = (fraction >> 1) | AXP_R_NM;
				exponent++;
			}
		}
		if (exponent > AXP_F_EXP_MASK)		// the mask also is the max
		{
			retVal = ArithmeticTraps;
			raised = FE_OVERFLOW;
			exponent = AXP_F_EXP_MASK;
		}
		else if (exponent < 0)
		{
			if ((fpFunc->trp & AXP_FP_TRP_U) != 0)
			{
				retVal = ArithmeticTraps;
				raised = FE_UNDERFLOW;
			}
			sign = fraction = exponent = 0;
		}
		instr->destv.fp.fpr32.sign = sign;
		instr->destv.fp.fpr32.exponent = exponent;
		instr->destv.fp.fpr32.fraction = fraction & AXP_R_FRAC;
		instr->destv.fp.fpr32.zero = 0;
	}

	/*
	 * Set the exception bits.
	 */
	if (raised != 0)
		AXP_FP_SetExcSum(instr, raised, true);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_DIVF
 *	This function implements the VAX F Format Floating-Point DIVide instruction
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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Invalid Operation
 *							Division By Zero
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_DIVF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}

	/*
	 * If we are trying to divide by zero, there is nothing else really to do.
	 * Just set the reason and move on.
	 */
	else if (instr->src2v.fp.uq != 0)
	{

		/*
	 	 * Cast the register values into doubles (no conversion required).
	 	 */
		src1v = (double) instr->src1v.fp.uq;
		src2v = (double) instr->src2v.fp.uq;

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v / src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Recast the result into the destination register.  Since this is
			 * a 32-bit value, don't forget to clear the bits that are supposed
			 * to be zero.
			 */
			instr->destv.fp.uq = (u64) destv;
			instr->destv.fp.fpr32.zero = 0;

			/*
			 * Before we can simply return back to the caller, we need to
			 * determine if an overflow condition may have occurred.
			 */
			if ((instr->destv.fp.fpr.exponent - AXP_T_BIAS) > AXP_F_BIAS)
				raised = FE_OVERFLOW;
			else
				switch (AXP_FP_ENCODE(&instr->destv.fp.fpr, false))
				{

					/*
					 * These 2 cases are the same as Denormal for IEEE.
					 * Basically, these are values that cannot be represented
					 * in VAX Float.
					 */
					case DirtyZero:
					case Reserved:
						raised = FE_UNDERFLOW;
						break;

					/*
					 * These are just fine.  Nothing more to do here.
					 */
					case Finite:
					case Zero:
						break;

					/*
					 * These are not returned when IEEE is set to false above.
					 * So, there's nothing we can do here.  This is done to
					 * keep the compiler happy (it does not like switch
					 * statements with an enumeration and not all the values
					 * are present).
					 */
					case Denormal:
					case Infinity:
					case NotANumber:
						break;
				}
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}
	else
	{
		raised = FE_DIVBYZERO;
		retVal = ArithmeticTraps;
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID | FE_DIVBYZERO);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_DIVG
 *	This function implements the VAX G Format Floating-Point DIVide instruction
 *	by the Alpha AXP processor.
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
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_DIVG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	long double		src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else if (instr->src2v.fp.uq != 0)
	{

		/*
	 	 * Need to convert from 64-bit to 128-bit.  We need to do this because,
	 	 * the VAX G exponent can go up to 1024, but the IEEE T exponent can
	 	 * only go up to 1023.  We need to expand the exponent to a 15-bit
	 	 * representation from an 11-bit one, to accommodate this difference.
	 	 */
		AXP_FP_CvtG2X(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr, &src1v, &src2v);

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v / src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Convert the result back into a VAX G format and see if we got
			 * an overflow or underflow.
			 */
			raised = AXP_FP_CvtX2G(&destv, NULL, &instr->destv.fp.fpr, NULL);
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}
	else
	{
		raised = FE_DIVBYZERO;
		retVal = ArithmeticTraps;
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID | FE_DIVBYZERO);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_ITOFF
 *	This function implements the Integer Move to VAX F Format Floating-Point
 *	Register instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_ITOFF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_S_MEMORY	*src1v = (AXP_S_MEMORY *) &instr->src1v.r.ul;
	u64 			exp	= src1v->exponent;

	/*
	 * Map the exponent from its 8-bit value to its 11-bit value.
	 */
	if (exp != 0)
		exp += (AXP_G_BIAS - AXP_F_BIAS);

	/*
	 * Move all the right parts into the right places.
	 */
	instr->destv.fp.sCvt.sign = src1v->sign;
	instr->destv.fp.sCvt.exponent = exp;
	instr->destv.fp.sCvt.fraction = src1v->fraction;
	instr->destv.fp.sCvt.zero = 0;

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
 * AXP_MULF
 *	This function implements the VAX F Format Floating-Point MULtiply
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
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_MULF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}

	/*
	 * OK, no Illegal Operand, so let's go and perform the calculation.
	 */
	else
	{

		/*
	 	 * Cast the register values into doubles (no conversion required).
	 	 */
		src1v = (double) instr->src1v.fp.uq;
		src2v = (double) instr->src2v.fp.uq;

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v * src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Recast the result into the destination register.  Since this is
			 * a 32-bit value, don't forget to clear the bits that are supposed
			 * to be zero.
			 */
			instr->destv.fp.uq = (u64) destv;
			instr->destv.fp.fpr32.zero = 0;

			/*
			 * Before we can simply return back to the caller, we need to
			 * determine if an overflow condition may have occurred.
			 */
			if ((instr->destv.fp.fpr.exponent - AXP_T_BIAS) > AXP_F_BIAS)
				raised = FE_OVERFLOW;
			else
				switch (AXP_FP_ENCODE(&instr->destv.fp.fpr, false))
				{

					/*
					 * These 2 cases are the same as Denormal for IEEE.
					 * Basically, these are values that cannot be represented
					 * in VAX Float.
					 */
					case DirtyZero:
					case Reserved:
						raised = FE_UNDERFLOW;
						break;

					/*
					 * These are just fine.  Nothing more to do here.
					 */
					case Finite:
					case Zero:
						break;

					/*
					 * These are not returned when IEEE is set to false above.
					 * So, there's nothing we can do here.  This is done to
					 * keep the compiler happy (it does not like switch
					 * statements with an enumeration and not all the values
					 * are present).
					 */
					case Denormal:
					case Infinity:
					case NotANumber:
						break;
				}
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_MULG
 *	This function implements the VAX G Format Floating-Point MULtiply
 *	instruction by the Alpha AXP processor.
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
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_MULG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	long double		src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Need to convert from 64-bit to 128-bit.  We need to do this because,
	 	 * the VAX G exponent can go up to 1024, but the IEEE T exponent can
	 	 * only go up to 1023.  We need to expand the exponent to a 15-bit
	 	 * representation from an 11-bit one, to accommodate this difference.
	 	 */
		AXP_FP_CvtG2X(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr, &src1v, &src2v);

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v * src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Convert the result back into a VAX G format and see if we got
			 * an overflow or underflow.
			 */
			raised = AXP_FP_CvtX2G(&destv, NULL, &instr->destv.fp.fpr, NULL);
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SQRTF
 *	This function implements the VAX F Format Floating-Point SQuare RooT
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
 * 							Invalid Operation
 */
AXP_EXCEPTIONS AXP_SQRTF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, NULL))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}

	/*
	 * OK, no Illegal Operand, so let's go and perform the calculation.
	 */
	else
	{

		/*
	 	 * Cast the register values into doubles (no conversion required).
	 	 */
		src1v = (double) instr->src1v.fp.uq;

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = sqrt(src1v);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		/*
		 * Recast the result into the destination register.  Since this is
		 * a 32-bit value, don't forget to clear the bits that are supposed
		 * to be zero.
		 */
		instr->destv.fp.uq = (u64) destv;
		instr->destv.fp.fpr32.zero = 0;
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SQRTG
 *	This function implements the VAX G Format Floating-Point SQuare RooT
 *	instruction by the Alpha AXP processor.
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
 * 							Invalid Operation
 */
AXP_EXCEPTIONS AXP_SQRTG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	long double		src1v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, NULL))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Need to convert from 64-bit to 128-bit.  We need to do this because,
	 	 * the VAX G exponent can go up to 1024, but the IEEE T exponent can
	 	 * only go up to 1023.  We need to expand the exponent to a 15-bit
	 	 * representation from an 11-bit one, to accommodate this difference.
	 	 */
		AXP_FP_CvtG2X(&instr->src1v.fp.fpr, NULL, &src1v, NULL);

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = sqrt(src1v);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		/*
		 * Convert the result back into a VAX G format and see if we got
		 * an overflow or underflow.
		 */
		raised = AXP_FP_CvtX2G(&destv, NULL, &instr->destv.fp.fpr, NULL);
		if (raised != 0)
			retVal = ArithmeticTraps;
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SUBF
 *	This function implements the VAX F Format Floating-Point SUBtract
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
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_SUBF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Cast the register values into doubles (no conversion required).
	 	 */
		src1v = (double) instr->src1v.fp.uq;
		src2v = (double) instr->src2v.fp.uq;

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v - src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Recast the result into the destination register.  Since this is
			 * a 32-bit value, don't forget to clear the bits that are supposed
			 * to be zero.
			 */
			instr->destv.fp.uq = (u64) destv;
			instr->destv.fp.fpr32.zero = 0;

			/*
			 * Before we can simply return back to the caller, we need to
			 * determine if an overflow condition may have occurred.
			 */
			if ((instr->destv.fp.fpr.exponent - AXP_T_BIAS) > AXP_F_BIAS)
				raised = FE_OVERFLOW;
			else
				switch (AXP_FP_ENCODE(&instr->destv.fp.fpr, false))
				{

					/*
					 * These 2 cases are the same as Denormal for IEEE.
					 * Basically, these are values that cannot be represented
					 * in VAX Float.
					 */
					case DirtyZero:
					case Reserved:
						raised = FE_UNDERFLOW;
						break;

					/*
					 * These are just fine.  Nothing more to do here.
					 */
					case Finite:
					case Zero:
						break;

					/*
					 * These are not returned when IEEE is set to false above.
					 * So, there's nothing we can do here.  This is done to
					 * keep the compiler happy (it does not like switch
					 * statements with an enumeration and not all the values
					 * are present).
					 */
					case Denormal:
					case Infinity:
					case NotANumber:
						break;
				}
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/*
 * AXP_SUBG
 *	This function implements the VAX G Format Floating-Point SUBtract
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
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_SUBG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	long double		src1v, src2v, destv;
	int				oldRndMode =0;
	int				raised = 0;

	/*
	 * Before we go too far, let's check the contents of the source registers.
	 *
	 * If either encoding turned up to be a dirty-zero or a reserved operand,
	 * then we need to return an Invalid Operation.
	 */
	if (AXP_FP_CheckForVAXInvalid(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr))
	{
		retVal = IllegalOperand;
		raised = FE_INVALID;
	}
	else
	{

		/*
	 	 * Need to convert from 64-bit to 128-bit.  We need to do this because,
	 	 * the VAX G exponent can go up to 1024, but the IEEE T exponent can
	 	 * only go up to 1023.  We need to expand the exponent to a 15-bit
	 	 * representation from an 11-bit one, to accommodate this difference.
	 	 */
		AXP_FP_CvtG2X(&instr->src1v.fp.fpr, &instr->src2v.fp.fpr, &src1v, &src2v);

		// TODO: We need to have a mutex starting at this point.

		/*
	 	 * Set the rounding mode, based on the function code and/or the FPCR.
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(cpu, fpFunc, oldRndMode);

		/*
	 	 * Clear the current set of exceptions.
	 	 */
		feclearexcept(FE_ALL_EXCEPT);

		/*
	 	 * Execute the instruction.
	 	 */
		destv = src1v - src2v;

		/*
	 	 * Test to see what exceptions were raised.
	 	 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.

		if (raised == 0)
		{

			/*
			 * Convert the result back into a VAX G format and see if we got
			 * an overflow or underflow.
			 */
			raised = AXP_FP_CvtX2G(&destv, NULL, &instr->destv.fp.fpr, NULL);
			if (raised != 0)
				retVal = ArithmeticTraps;
		}
	}

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	raised &= (FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);
	AXP_FP_SetExcSum(instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
