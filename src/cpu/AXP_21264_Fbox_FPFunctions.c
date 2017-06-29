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
 *	functionality of the Fbox Floating Point Operations of the Digital Alpha
 *	AXP Processor.
 *
 *	Revision History:
 *
 *	V01.000		29-June-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox_FPFunctions.h"

typedef struct
{
	u8		ieeeType;
	u32		sign;
	u32		exponent;
	u64		fraction;
} AXP_FP_COMP;

/*
 * AXP_FP_AddSub
 *	This function implements the VAX F/G and IEEE S/T Format Floating-Point
 *	ADD/SUB instructions of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FP_AddSub(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_COMP		av, bv;
	AXP_FP_COMP		*a = &av, *b = &bv;
	i32				expDiff;
	u32				sticky, roundMode;
	bool			subtraction = false;
	bool			ieee = false;

	/*
	 * First we need to unpack the sign, exponent and fraction.  We do this
	 * based on the specific instruction, which indicates the type of floating-
	 * point value we are dealing.  We also can determine if this is an Add or
	 * Subtract call.
	 *
	 * TODO:	This can be cleaned up quite a bit and simplified.  Same goes
	 *			at the end of this function.
	 */
	if (instr->opcode == FLTV)
	{
		switch (instr->function)
		{
			case AXP_FUNC_SUBF:
				subtraction = true;
				/* Let this fall through */
			case AXP_FUNC_ADDF:
				AXP_F_UNPACK(instr->src1v, av, retval);
				if (retVal == NoExceptions)
					AXP_F_UNPACK(instr->src2v, bv, retVal);
				break;

			case AXP_FUNC_SUBG:
				subtraction = true;
				/* Let this fall through */
			case AXP_FUNC_ADDG:
				AXP_G_UNPACK(instr->src1v, av, retVal);
				if (retVal == NoExceptions)
					AXP_G_UNPACK(instr->src2v, bv, retVal);
				break;
		}
	}
	else
	{
		ieee = true;
		switch (instr->function)
		{
			case AXP_FUNC_SUBS:
				subtraction = true;
				/* Let this fall through */
			case AXP_FUNC_ADDS:
				AXP_F_UNPACK(instr->src1v, av, retVal);
				AXP_F_UNPACK(instr->src2v, bv, retVal);
				break;

			case AXP_FUNC_SUBT:
				subtraction = true;
				/* Let this fall through */
			case AXP_FUNC_ADDT:
				AXP_G_UNPACK(instr->src1v, av, retVal);
				AXP_G_UNPACK(instr->src2v, bv, retVal);
				break;
		}
	}

	/*
	 * If unpacking indicated that we had IEEE NaN (Not a Number), then
	 * return a quiet value.
	 */
	if (a->ieeeType == AXP_IEEE_NAN)
		instr->destv.fp.uq = instr->src1v.fo.uq | QNAN;
	else if (b->ieeeType == AXP_IEEE_NAN)
		instr->destv.fp.uq = instr->src2v.fo.uw | QNAN;

	/*
	 * OK, we have what appear to be good numbers with which to play.
	 */
	else
	{

		/*
		 * If this is a subtraction, then we invery the sign of the second
		 * operand (+ --> -/- --> +).
		 */
		if (subtraction == true)
			b->sign ^= 1;

		/*
		 * If b is infinite, we have some work to do.
		 */
		if (b->ieeeType == AXP_IEEE_INF)
		{

			/*
			 * If a is infinite and we are effectively doing a subtraction,
			 * then we have an invalid operation.  We'll set the result of the
			 * subtraction to a canonical quiet NaN value and set the return
			 * value of this function to Invalid Operation.
			 */
			if ((a->ieeeType == AXP_IEEE_INF) &&
				((a->sign ^ b->sign) != 0))
			{
				instr->destv.fp.uq = 0xfff8000000000000ll;	// canonical quiet NaN
				retVal = InvalidOperation;
			}
			else
			{
				instr->destv.fp.uq = instr->src2v.fp,uq;
				if (subtraction == true)
					instr->destv.fp.t.sign ^= 1;
			}
			return(retVal);
		}

		/*
		 * If a is infinite, set it as the result and return to the caller.
		 */
		if (a->ieeeType == AXP_IEEE_INF)
		{
			instr->destv.fp.uq = instr->src1v.fp.uq;
			return(retVal);
		}

		/*
		 * TODO:	We need to extract the round mode from the instruction's
		 *			function value.
		 */
		if (roundMode == AXP_IEEE_DYNAMIC)
			roundMode = cpu->fpcr.dyn;

		/*
		 * If a's exponent is zero, then a is set equal to b.
		 */
		if (((a->exponent == 0) && (a->ieeeType != AXP_IEEE_ZERO)) ||
			(a->ieeeType == AXP_IEEE_ZERO))
		{
			if (b->ieeeType != AXP_IEEE_ZERO)
				a = &bv;
			else if (a->sign != b->sign)
				a->sign = (roundMode == AXP_IEEE_MINUS) ? 1 : 0;
		}

		/*
		 * If b's exponent is not zero, then we have some calculating to do.
		 */
		else if (((b->exponent != 0) && (b->ieeeType != AXP_IEEE_ZERO)) ||
				 (b->ieeeType != AXP_IEEE_ZERO))
		{

			/*
			 * If the absolute value of a is less than b, then swap a and b.
			 */
			if ((a->exponent < b->exponent) || 
				((a->exponent == b->exponent) &&
				 (a->fraction < b->fraction)))
			{
				a = &bv;
				b = &av;
			}

			/*
			 * Get the difference between the two exponents.  This is, potentially,
			 * for the denormalization of b.
			 */
			expDiff = a->exponent - b->exponent;

			/*
			 * For some reason, the following code is outside the "Subtraction"
			 * section for IEEE, but inside for VAX.  Not exactly sure why that
			 * is, or even if it makes a difference.  For now, I'll do either
			 * depending upon whether I'm dealing with IEEE or VAX floating
			 * point value.
			 */
			if (ieee == true)
			{

				/*
				 * If the difference is greater than (63), then we need to
				 * retain the sticky bit.
				 */
				if (expDiff > 63)
					b->fraction = 1;

				/*
				 * If the exponents for a and b are different, then we need to
				 * denormalize b by shifting it the same amount.
				 */
				else if (expDiff != 0) 
				{

					/*
					 * We need to retain the lost bits, and the shift b's
					 * fraction, or'ing back in these bits.
					 */
					sticky = (b->fraction << (64 - expDiff)) ? 1: 0;
					b->fraction = (b.fraction >> expDiff) | sticky;
				}
			}

			/*
			 * If the signs don't match, but one is set, then we are performing
			 * a Subtraction.
			 */
			if ((a->sign ^ b->sign) != 0)
			{

				/*
				 * We did the following code above for IEEE.  Only do it here
				 * if we are not IEEE (or are VAX).
				 */
				if (ieee == false)
				{

					/*
					 * If the difference is greater than (63), then we need to
					 * retain the sticky bit.
					 */
					if (expDiff > 63)
						b->fraction = 1;

					/*
					 * If the exponents for a and b are different, then we need to
					 * denormalize b by shifting it the same amount.
					 */
					else if (expDiff != 0) 
					{

						/*
						 * We need to retain the lost bits, and the shift b's
						 * fraction, or'ing back in these bits.
						 */
						sticky = (b->fraction << (64 - expDiff)) ? 1: 0;
						b->fraction = (b.fraction >> expDiff) | sticky;
					}
				}

				/*
				 * Perform the operation (in this case it is a subtraction).
				 * NOTE: We explicitly put the result in av, to be used later.
				 */
				av.sign = a->sign;
				av.exponent = a->exponent;
				av.fraction = (a->fraction - b->fraction);

				/*
				 * Normalize the result.
				 */
				if (ieee == false)
					vax_norm(av);
				else if (av.fraction == 0)
				{
					av.exponent = 0;
					av.sign = (roundMode == AXP_IEEE_MINUS) ? 1 : 0;
				}
				else
					ieee_norm(av);
			}

			/*
			 * OK, the signs match, so we are performing a Add.
			 */
			else
			{

				/*
				 * We did the following code above for IEEE (but different).
				 * Only do it here if we are not IEEE (or are VAX).
				 */
				if (ieee == false)
				{

					/*
					 * A difference of exponents greater than 63 means that b's
					 * fraction goes to zero.
					 */
					if (expDiff > 63)
						b->fraction = 0;

					/*
					 * A non-zero differnce, means that we have to shift b to
					 * compensate for this difference (so the exponents can
					 * have the same value.
					 */
					else if (expDiff != 0)
						b->fraction = b->fraction >> expDiff;
				}

				/*
				 * Perform the operation (in this case it is a subtraction).
				 */
				av.sign = a->sign;
				av.exponent = a->exponent;
				av.fraction = a->fraction + b->fraction

				/*
				 * If a is less than b, then we need to perform a shift and
				 * carry.  There is also no need to normalize the result,
				 * because it already will be.
				 */
				if (av.fraction < b->fraction)
				{
					av.fraction = 0x8000000000000000ll | (av.fraction >> 1);
					av.exponent = av.exponent + 1;
				}
			}
		}

		/*
		 * OK, we now need to pack the sign, exponent, and fraction back into
		 * their appropriate places, with the appropriate lengths in register
		 * format.
		 */
		if (instr->opcode == FLTV)
		{
			switch (instr->function)
			{
				case AXP_FUNC_SUBF:
				case AXP_FUNC_ADDF:
					AXP_F_PACK(instr->destv, av, retVal);
					break;

				case AXP_FUNC_SUBG:
				case AXP_FUNC_ADDG:
					AXP_G_PACK(instr->destv, av, retVal);
					break;
			}
		}
		else
		{
			switch (instr->function)
			{
				case AXP_FUNC_SUBS:
				case AXP_FUNC_ADDS:
					AXP_S_PACK(instr->destvv, av, retVal);
					break;

				case AXP_FUNC_SUBT:
				case AXP_FUNC_ADDT:
					AXP_T_PACK(instr->destv, av, retVal);
					break;
			}
		}
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
