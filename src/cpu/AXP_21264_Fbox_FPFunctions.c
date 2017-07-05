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
	u32		fraction : 23;
	u32		exponent : 8;
	u32		sign;
} AXP_IEEE_S_FLOAT;

/*
 * AXP_FP_CvtFPRToFloat
 *	Hey, guess what, the GNU C compiler generates code that is IEEE compliant.
 *	The only thing We are going to do here is to convert the floating-point
 *	register format to a 32-bit float and then use the math run-time library
 *	functions to actually do that work.  This only needs to be called for VAX F
 *	and IEEE S formatted registers (we need to strip out the high order
 *	fraction and reduce the exponent from 11 to 8 bits.
 *
 * Input Parameter:
 * 	fpr:
 * 		The 64-bit floating point register value to be converted.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The converted float.
 */
float AXP_FP_CvtFPRToFloat(AXP_FP_REGISTER fpr)
{
	float				retVal = 0.0;
	AXP_IEEE_S_FLOAT	*sFloat = (AXP_IEEE_S_FLOAT *) &retVal;

	/*
	 * Convert away.
	 */
	sFloat->sign = fpr.fpr.sign;
	sFloat->exponent =
		((fpr.fpr.exponent & 0x400) >> 3) |
		((fpr.fpr.exponent & 0x07f));
	sFloat->fraction = fpr.fpr32.fraction;

	/*
	 * Return the converted value back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_FP_CvtFloatToFPR
 *	Hey, guess what, the GNU C compiler generates code that is IEEE compliant.
 *	The only thing We are going to do here is to convert the 32-bit float back
 *	to a floating-point register format.  This only needs to be called for VAX
 *	F and IEEE S formatted registers (we need to put back the high order
 *	fraction, clearing out the low order portion, and expend the exponent from
 *	8 to 11 bits.
 *
 * Input Parameter:
 * 	real32:
 * 		The float value to be converted.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The converted 64-bit floating point register value.
 */
AXP_FP_REGISTER AXP_FP_CvtFloatToFPR(float real32)
{
	AXP_FP_REGISTER		retVal;
	AXP_IEEE_S_FLOAT	*sFloat = (AXP_IEEE_S_FLOAT *) &real32;

	/*
	 * Convert away.
	 */
	retVal.fpr32.sign =sFloat->sign;
	retVal.fpr32.exponent =
		((sFloat->exponent & 0x80) << 3) |
		((sFloat->exponent & 0x7f));
	if (sFloat->exponent & 0x80)
	{
		if ((sFloat->exponent & 0x7f) == 0x7f)
			sFloat->exponent |= 0x38;
	}
	else
	{
		if ((sFloat->exponent & 0x7f) != 0x00)
			sFloat->exponent |= 0x38;
	}
	retVal.fpr32.fraction = sFloat->fraction;
	retVal.fpr32.zero = 0;

	/*
	 * Return the converted value back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_FP_SetRoundingMode
 * 	This function is called to set the rounding mode.  It determines this based
 * 	one the function field from the instruction of the FPCR in the cpu
 * 	structure.  If this is called with the cpu equal to NULL, then use the
 * 	roundingMode parameter to reset the rounding mode.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	function:
 * 		A pointer to a structure containing the information needed determine
 * 		the rounding mode.
 * 	resetRoundingMode:
 * 		A value of the previous rounding mode, which was returned on a previous
 * 		call to this function.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A value representing the previous value for the rounding mode.
 */
int AXP_FP_SetRoundingMode(
	AXP_21264_CPU *cpu,
	AXP_FP_FUNC *func,
	int resetRoundingMode)
{
	int		savedRoundingMode = fegetround();
	int		newRoundingMode;

	/*
	 * perform the math,
	 * and then convert the result back into 64-bit register format
	 */
	if (cpu != NULL)
	{
		switch (func->rnd)
		{
			case  AXP_FP_CHOPPED:
				newRoundingMode = FE_TOWARDZERO;
				break;

			case  AXP_FP_MINUS_INF:
				newRoundingMode = FE_DOWNWARD;
				break;

			case  AXP_FP_NORMAL:
				newRoundingMode = FE_TONEAREST;
				break;

			case  AXP_FP_DYNAMIC:
				switch (cpu->fpcr.dyn)
				{
					case  AXP_FP_CHOPPED:
						newRoundingMode = FE_TOWARDZERO;
						break;

					case  AXP_FP_MINUS_INF:
						newRoundingMode = FE_DOWNWARD;
						break;

					case  AXP_FP_NORMAL:
						newRoundingMode = FE_TONEAREST;
						break;

					case  AXP_FP_PLUS_INF:
						newRoundingMode = FE_UPWARD;
						break;
				}
				break;
		}
	}
	else
		newRoundingMode = resetRoundingMode;

	/*
	 * Set the rounding mode.
	 */
	if (fesetround(newRoundingMode) != 0)
	{
		fprintf(stderr, "Internal error: "
				"unexpected return value when setting "
				"rounding mode to %d at %s, line %d.\n",
				newRoundingMode, __FILE__, __LINE__);
	}

	/*
	 * Returned the previous rounding mode back to the caller.  They'll call
	 * back to reset the rounding mode with this value and a NULL cpu
	 * parameter.
	 */
	return(savedRoundingMode);
}

/*
 * fpNormalize
 * 	This function is called to normalize a floating point value.  Both VAX and
 * 	IEEE floating point values are stored in memory and registers with a
 * 	"hidden" bit.  This hidden bit is always a 1 and represents a value that is
 * 	1.xxxx-Eyyyy where xxxx is the fraction and yyyy is the exponent.  Since the
 * 	'1' is always there and always 1, then there is no reason to store it.
 * 	This allows for greater precision in the fraction.  This code shifts the
 * 	fraction and adjusts the exponent to this "normalized" form.
 *
 * Input Parameter:
 * 	r:
 * 		A pointer to the structure containing the fraction and exponent to be
 * 		normalized.
 *
 * Output Parameter:
 * 	r:
 * 		A pointer to the structure to receive the normalized result.
 *
 * Return Value:
 * 	None.
 */
void AXP_FPNormalize(AXP_FP_UNPACKED *r)
{
	i32			ii;
	static u64	normmask[5] =
	{
		0xc000000000000000ll,
		0xf000000000000000ll,
		0xff00000000000000ll,
		0xffff000000000000ll,
		0xffffffff00000000ll
	};
	static i32	normtab[6] = { 1, 2, 4, 8, 16, 32};

	r->fraction &= AXP_LOW_QUAD;
	if (r->fraction == 0)
	{
		r->sign = r->exponent = 0;
		return;
	}

	/*
	 * Normalize the the register.
	 */
	while ((r->fraction & AXP_R_NM) == 0)
	{
		for (ii = 0; ii < 5; ii++)
		{
			if (r->fraction & normmask[ii])
				break;
		}
		r->fraction <<= normtab[ii];
		r->exponent -= normtab[ii];
	}
	return;
}

/*
 * AXP_FPUnpack
 * 	This function is called to unpack a floating point value from register
 * 	format into a structure used to perform the operation that has been
 * 	requested.  The code also interprets the various bit patterns to determine
 * 	what is encoded in the FP value (Zero, NaN, Dirty Zero, Finite, ...).
 * 	There is an equivalent 'pack' function to store the floating point result
 * 	back into the register.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Alpha AXP 21264 CPU structure.  We utilize the
 * 		Floating-Point Control Register (FPRC) to determine how to handle
 * 		denormalized values (to zero or not).
 * 	s:
 * 		A source FP register value to be "unpacked".
 * 	ieee:
 * 		An indicator of whether we are unpacking an IEEE float or not (VAX
 * 		float).
 *
 * Output Parameters:
 * 	r:
 * 		A structure to receive the various components of the floating point
 * 		value (sign, exponent, fraction and encoding).
 *
 * Return Value:
 * 	NoException:	The value in the FP register was formed properly, or
 * 					quietly adjusted.
 * 	IllegalOperand:	The value in the FP register had 'reserved' or invalid
 * 					format.
 */
AXP_EXCEPTIONS AXP_FPUnpack(
		AXP_21264_CPU *cpu,
		AXP_FP_UNPACKED *r,
		AXP_FP_REGISTER s,
		bool ieee)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * Unpack the various fields into the unpacked structure.  We'll re-pack
	 * the results later.
	 */
	r->sign = s.fpr.sign;
	r->exponent = s.fpr.exponent;
	r->fraction = s.fpr.fraction;
	r->encoding = AXP_FP_ENCODE(r, ieee);

	/*
	 * Now we do some checking and correcting.
	 */
	if (ieee == false)
	{
		if (r->exponent == 0)
		{
			if (s.uq != 0)
				retVal = IllegalOperand;
			r->fraction = r->sign = 0;
		}
	}
	else
	{
		if (r->exponent == 0)
		{
			if (r->fraction != 0)
			{
				if (cpu->fpcr.dnz == 1)
				{
					r->fraction = 0;
					r->encoding = Zero;
				}
				else
				{
					r->fraction <<= AXP_R_GUARD;
					AXP_FPNormalize(r);
					r->encoding = Denormal;
					retVal = IllegalOperand;
				}
			}
		}
		else if (r->encoding == NotANumber)
		{
			if (r->fraction & AXP_R_CQ_NAN)
				retVal = IllegalOperand;
		}
	}

	/*
	 * If we are still in good shape, insert the hidden bit.
	 */
	if (retVal == NoException)
		r->fraction = (r->fraction | AXP_R_HB) << AXP_R_GUARD;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_VAXPack
 * 	This function is called to take the results of the executed calculation
 * 	and store them into the destination value, in register format, rounding as
 * 	necessary.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer containing the decoded instruction information.
 * 	r:
 * 		A pointer to the execution results.
 * 	dt:
 * 		A value indicating the data type being packed.
 *
 * Output Parameters:
 * 	instr:
 * 		The destination value within the structure is set with the results.
 *
 * Return Value:
 * 	NoException:
 * 		Normal successful completion.
 * 	ArithmeticTraps:
 * 		We got either an Overflow or Underflow.
 */
AXP_EXCEPTIONS AXP_VAXPack(
		AXP_INSTRUCTION *instr,
		AXP_FP_UNPACKED *r,
		u32 dt)
{
	AXP_EXCEPTIONS		retVal = NoException;
	AXP_FP_FUNC			fpFunc;
	u32					roundMode;
	static const u64	roundBit[] = {AXP_F_RND, AXP_G_RND};
	static const i32	expMax[] =
		{
			AXP_G_BIAS - AXP_F_BIAS + AXP_F_EXP_MASK,
			AXP_G_EXP_MASK
		};
	static const i32	expMin[] = { AXP_G_BIAS - AXP_F_BIAS, 0};

	fpFunc = *((AXP_FP_FUNC *) &instr->function);
	roundMode = fpFunc.rnd;

	/*
	 * If the fraction not zero, then we may need to perform some rounding.
	 */
	if (r->fraction != 0)
	{

		/*
		 * If the instruction rounding mode is not Chopped, then we need to do
		 * some explicit rounding.
		 */
		if (roundMode != AXP_FP_CHOPPED)
		{
			r->fraction += roundBit[dt];
			if ((r->fraction & AXP_R_NM) == 0)
			{
				r->fraction = (r->fraction >> 1) | AXP_R_NM;
				r->exponent++;
			}
		}

		/*
		 * If the exponent is larger than the maximum.  Set it to the maximum
		 * and return an exception.
		 */
		if (r->exponent > expMax[dt])
		{
			r->exponent = expMax[dt];
			retVal = ArithmeticTraps;
		}

		/*
		 * If the exponent is smaller than the minimum.  Set it to the minimum
		 * and, if the instruction indicates return an exception.
		 */
		if (r->exponent < expMin[dt])
		{
			r->exponent = 0;
			r->fraction = 0;
			if (fpFunc.trp == AXP_FP_TRP_V)
				retVal = ArithmeticTraps;
		}
	}
	else
		r->exponent = 0;

	/*
	 * Move the various components into the destination value for the
	 * instruction (register format).
	 */
	instr->destv.fp.fpr.sign = r->sign;
	instr->destv.fp.fpr.exponent = r->exponent;
	instr->destv.fp.fpr.fraction = (r->fraction >> AXP_R_GUARD) & AXP_R_FRAC;

	/*
	 * If this is for a VAX F float, zero out the low part of the fraction.
	 */
	if (dt == AXP_F_DT)
		instr->destv.fp.fCvt.zero = 0;
	return(retVal);
}

/*
 * AXP_IEEEPack
 * 	This function is called to take the results of the executed calculation
 * 	and store them into the destination value, in register format, rounding as
 * 	necessary.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer containing the decoded instruction information.
 * 	r:
 * 		A pointer to the execution results.
 * 	dt:
 * 		A value indicating the data type being packed.
 *
 * Output Parameters:
 * 	instr:
 * 		The destination value within the structure is set with the results.
 *
 * Return Value:
 * 	NoException:
 * 		Normal successful completion.
 * 	ArithmeticTraps:
 * 		We got an Overflow, Underflow, or InExact result.
 */

AXP_EXCEPTIONS AXP_IEEEPack(
		AXP_21264_CPU *cpu,
		AXP_INSTRUCTION *instr,
		AXP_FP_UNPACKED *r,
		u32 dt)
{
	AXP_EXCEPTIONS		retVal = NoException;
	AXP_FP_FUNC			fpFunc;
	static const u64	stdRound[] = {AXP_S_RND, AXP_T_RND};
	static const u64	infRound[] = {AXP_S_INF, AXP_T_INF};
	static const i32	expMax[] =
		{
			AXP_T_BIAS - AXP_S_BIAS + AXP_S_EXP_MASK - 1,
			AXP_T_EXP_MASK - 1
		};
	static const i32	expMin[] = { AXP_T_BIAS - AXP_S_BIAS, 0};
	u64					roundAdd, roundBits;
	u32					roundMode;

	fpFunc = *((AXP_FP_FUNC *) &instr->function);
	roundMode = fpFunc.rnd;
	if (r->fraction != 0)
	{
		if (roundMode == AXP_FP_DYNAMIC)
			roundMode = cpu->fpcr.dyn;
		roundBits = r->fraction & infRound[dt];
		if (roundMode == AXP_FP_NORMAL)
			roundAdd = stdRound[dt];
		else if (((roundMode == AXP_FP_PLUS_INF) && (r->sign == 0)) ||
				 ((roundMode == AXP_FP_MINUS_INF) && (r->sign == 1)))
			roundAdd = infRound[dt];
		else
			roundAdd = 0;
		r->fraction += roundAdd;
		if ((r->fraction & AXP_R_NM) == 0)
		{
			r->fraction = (r->fraction & 1) | AXP_R_NM;
			r->exponent++;
		}
		if (roundBits != 0)
			retVal = ArithmeticTraps;
		if (r->exponent > expMax[dt])
		{
			retVal = ArithmeticTraps;
			if (roundAdd != 0)
			{
				instr->destv.fp.uq = (r->sign == 1) ? AXP_R_MINF : AXP_R_PINF;
				if (dt == AXP_S_DT)
					instr->destv.fp.sCvt.zero = 0;
				return(retVal);
			}
			instr->destv.fp.uq = (r->sign == 1) ? AXP_R_MMAX : AXP_R_PMAX;
			if (dt == AXP_S_DT)
				instr->destv.fp.sCvt.zero = 0;
			return(retVal);
		}
		if (r->exponent <= expMin[dt])
		{
			retVal = ArithmeticTraps;
			r->sign = 0;
			r->exponent = 0;
			r->fraction = 0;
		}
	}
	else
		r->exponent = 0;

	/*
	 * Move the various components into the destination value for the
	 * instruction (register format).
	 */
	instr->destv.fp.fpr.sign = r->sign;
	instr->destv.fp.fpr.exponent = r->exponent;
	instr->destv.fp.fpr.fraction = (r->fraction >> AXP_R_GUARD) & AXP_R_FRAC;
	if ((roundMode == AXP_FP_NORMAL) && (roundBits == stdRound[dt]))
		instr->destv.fp.uq &= ~1;

	/*
	 * If this is for a IEEE S float, zero out the low part of the fraction.
	 */
	if (dt == AXP_S_DT)
		instr->destv.fp.sCvt.zero = 0;
	return(retVal);
}
/*
 * AXP_FPAddSub
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
AXP_EXCEPTIONS AXP_FPAddSub(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr, u32 dt)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_UNPACKED	a;
	AXP_FP_UNPACKED b;
	i32				expDiff;
	u32				roundMode;
	bool			subtraction = false;
	bool			ieee = false;

	/*
	 * First we need to determine whether we are processing a add or
	 * subtraction, and IEEE Floating or VAX Floating.
	 */
	if (instr->opcode == FLTV)
	{
		if ((instr->function == AXP_FUNC_SUBF) ||
			(instr->function == AXP_FUNC_SUBG))
				subtraction = true;
	}
	else
	{
		ieee = true;
		if ((instr->function == AXP_FUNC_SUBS) ||
			(instr->function == AXP_FUNC_SUBT))
				subtraction = true;
	}

	/*
	 * Unpack the input registers for processing.
	 */
	retVal = AXP_FPUnpack(cpu, &a, instr->src1v.fp, ieee);
	if (retVal == NoException)
		retVal = AXP_FPUnpack(cpu, &b, instr->src2v.fp, ieee);

	/*
	 * We only proceed if some exception was not detected.
	 */
	if (retVal == NoException)
	{

		/*
		 * If unpacking indicated that we had IEEE NaN (Not a Number), then
	 	 * return a quiet value.
	 	 */
		if (a.encoding == NotANumber)
		{
			instr->destv.fp.uq = instr->src1v.fp.uq | AXP_R_QNAN;
			return(retVal);
		}
		else if (b.encoding == NotANumber)
		{
			instr->destv.fp.uq = instr->src2v.fp.uq | AXP_R_QNAN;
			return(retVal);
		}

		/*
	 	 * If this is a subtraction, then we invert the sign of the second
	 	 * operand (+ --> -/- --> +).
	 	 */
		if (subtraction == true)
			b.sign ^= 1;

		/*
		 * We need to handle some IEEE specifics here.
		 */
		if (ieee == true)
		{

			/*
		 	 * If 'b' is infinite, we have some work to do.
		 	 */
			if (b.encoding == Infinity)
			{

				/*
			 	 * If 'a' is infinite and we are effectively doing a
			 	 * subtraction, then we have an invalid operation.  We'll set
			 	 * the result of the subtraction to a canonical quiet NaN value
			 	 * and set the return value of this function to Invalid
			 	 * Operation.
			 	 */
				if ((a.encoding == Infinity) && ((a.sign ^ b.sign) != 0))
				{
					instr->destv.fp.uq = AXP_R_CQ_NAN;	// canonical quiet NaN
					retVal = IllegalOperand;
				}
				else
				{
					instr->destv.fp.uq = instr->src2v.fp.uq;
					if (subtraction == true)
						instr->destv.fp.fpr.sign ^= 1;
				}
				return(retVal);
			}

			/*
		 	 * If 'a' is infinite, set it as the result and return to the
		 	 * caller.
		 	 */
			if (a.encoding == Infinity)
			{
				instr->destv.fp.uq = instr->src1v.fp.uq;
				return(retVal);
			}

			/*
		 	 * Extract the rounding mode indicated in the instruction function.
		 	 * If it indicates 'dynamic', then get the rounding mode out of the
		 	 * FPCR.
		 	 */
			roundMode = ((AXP_FP_FUNC *) &instr->function)->rnd;
			if (roundMode == AXP_FP_DYNAMIC)
				roundMode = cpu->fpcr.dyn;
		}

		/*
		 * If 'a' is zero (for VAX), then set 'a' equal to 'b'.
		 */
		if ((a.encoding == Zero) || (a.exponent == 0))
		{
			if ((ieee == false) ||
				((ieee == true) && (b.encoding != Zero)))
				a = b;
			else if ((ieee == true) && (a.sign != b.sign))
				a.sign = (roundMode == AXP_FP_MINUS_INF) ? 1 : 0;
		}
		else if (((ieee == false) && (b.exponent != 0)) ||
				 ((ieee == true) && (b.encoding != Zero)))
		{
			if ((a.exponent < b.exponent) ||
				((a.exponent == b.exponent) &&
				 (a.fraction < b.fraction)))
			{
				AXP_FP_UNPACKED t;

				t = a;
				a = b;
				b = t;
			}
			expDiff = a.exponent - b.exponent;
			if ((ieee == true) || ((a.sign ^ b.sign) != 0))
			{
				if (expDiff > 63)
					b.fraction = 1;
				else if (expDiff != 0)
				{
					u32		sticky;

					sticky = (b.fraction << (64 - expDiff)) != 0 ? 1 : 0;
					b.fraction = (b.fraction >> expDiff) | sticky;
				}
			}
			else if ((ieee = false) && ((a.sign ^ b.sign) == 0))
			{
				if (expDiff > 63)
					b.fraction = 0;
				else if (expDiff != 0)
					b.fraction >>= expDiff;
			}
			if ((a.sign ^ b.sign) != 0)
			{
				a.fraction -= b.fraction;
				if ((ieee == true) && (a.fraction == 0))
				{
					a.exponent = 0;
					a.sign = (roundMode == AXP_FP_MINUS_INF) ? 1 : 0;
				}
				else
					AXP_FPNormalize(&a);
			}
			else
			{
				a.fraction += b.fraction;
				if (a.fraction < b.fraction)
				{
					a.fraction =
						AXP_R_NM | (a.fraction >> 1) | (a.fraction & 1);
					a.exponent++;
				}
			}
		}

		/*
		 * OK, we are finally done here.  Go and repack the fraction back into
		 * register format.
		 */
		if (ieee == true)
			retVal = AXP_IEEEPack(cpu, instr, &a, dt);
		else
			retVal = AXP_VAXPack(instr, &a, dt);
	}

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
