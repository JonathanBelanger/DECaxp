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

/*
 * TODO:	We're going to need the unpacking code, but a highly watered down
 * 			version.
 */

void fpNormalize(AXP_FP_REGISTER *r)
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

	r->fpv.t.fraction &= AXP_LOW_QUAD;
	if (r->fpv.t.fraction == 0)
	{
		r->fpv.t.sign = r->fpv.t.exponent = 0;
		return;
	}

	/*
	 * Normalize the the register.
	 *
	 * TODO:	fraction comes out of a 52-bit field.  The and below masks out
	 * 			the 64th bit.  We need to check into this.
	 */
	while ((r->fpv.t.fraction & AXP_R_NM) == 0)
	{
		for (ii = 0; ii < 5; ii++)
		{
			if (r->fpv.t.fraction & normmask[ii])
				break;
		}
		r->fpv.t.fraction <<= normtab[ii];
		r->fpv.t.exponent -= normtab[ii];
	}
	return;
}

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
	AXP_FP_REGISTER	*a = &instr->src1v.fp;
	AXP_FP_REGISTER	*b = &instr->src2v.fp;
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

		/*
		 * If the IEEE value indicates Denormal, then the exponent is zero and
		 * he fraction is not.  If the denormals are to be set to zero, then
		 * do so.  Otherwise, we have an invalid operation.
		 */
		if (a->fpc == IEEEDenormal)
		{
			if (cpu->fpcr.dnz == 1)
			{
				a->fpv.t.fraction = 0;
				a->fpc = IEEEZero;
			}
			else
			{
				// TODO: The following code may not work as expected.  Fraction
				//		 is 52-bits and doing this shift may not do what we
				//		 expect.
				a->fpv.t.fraction <<= AXP_R_GUARD;
				fpNormalize(&a);
				retVal = IllegalOperand;
			}
		}
		if (b->fpc == IEEEDenormal)
		{
			if (cpu->fpcr.dnz == 1)
			{
				b->fpv.t.fraction = 0;
				b->fpc = IEEEZero;
			}
			else
			{
				// TODO: The following code may not work as expected.  Fraction
				//		 is 52-bits and doing this shift may not do what we
				//		 expect.
				b->fpv.t.fraction <<= AXP_R_GUARD;
				fpNormalize(&b);
				retVal = IllegalOperand;
			}
		}
	}

	/*
	 * If unpacking indicated that we had IEEE NaN (Not a Number), then
	 * return a quiet value.
	 */
	if (a->fpc == IEEENotANumber)
	{
		instr->destv.fp.fpv.uq = a->fpv.uq | AXP_QUIET_NAN;
		instr->destv.fp.fpc = IEEENotANumber;
		return(retVal);
	}
	else if (b->fpc == IEEENotANumber)
	{
		instr->destv.fp.fpv.uq = b->fpv.uq | AXP_QUIET_NAN;
		instr->destv.fp.fpc = IEEENotANumber;
		return(retVal);
	}

	/*
	 * OK, we have what appear to be good numbers with which to play.
	 */
	else
	{

		/*
		 * If this is a subtraction, then we invert the sign of the second
		 * operand (+ --> -/- --> +).
		 */
		if (subtraction == true)
			b->fpv.t.sign ^= 1;

		/*
		 * If b is infinite, we have some work to do.
		 */
		if (b->fpc == IEEEInfinity)
		{

			/*
			 * If a is infinite and we are effectively doing a subtraction,
			 * then we have an invalid operation.  We'll set the result of the
			 * subtraction to a canonical quiet NaN value and set the return
			 * value of this function to Invalid Operation.
			 */
			if ((a->fpc == IEEEInfinity) &&
				((a->fpv.t.sign ^ b->fpv.t.sign) != 0))
			{
				instr->destv.fp.fpv.uq = AXP_R_CQ_NAN;	// canonical quiet NaN
				instr->destv.fp.fpc = IEEENotANumber;
				retVal = IllegalOperand;
			}
			else
			{
				instr->destv.fp.fpv.uq = b->fpv.uq;
				if (subtraction == true)
					instr->destv.fp.fpv.t.sign ^= 1;
				instr->destv.fp.fpc = b->fpc;
			}
			return(retVal);
		}

		/*
		 * If a is infinite, set it as the result and return to the caller.
		 */
		if (a->fpc == IEEEInfinity)
		{
			instr->destv.fp.fpv.uq = a->fpv.uq;
			instr->destv.fp.fpc = a->fpc;
			return(retVal);
		}

		/*
		 * TODO:	We need to extract the round mode from the instruction's
		 *			function value.
		 */
		roundMode = ((AXP_FP_FUNC) instr->function).rnd;
		if (roundMode == AXP_FP_DYNAMIC)
			roundMode = cpu->fpcr.dyn;

		/*
		 * If a's exponent is zero, then a is set equal to b.
		 */
		if (((a->fpv.t.exponent == 0) && (a->fpc != IEEEZero)) ||
			(a->fpc == IEEEZero))
		{
			if (b->fpc != IEEEZero)
				a = &b;
			else if (a->fpv.t.sign != b->fpv.t.sign)
				a->fpv.t.sign = (roundMode == AXP_FP_MINUS_INF) ? 1 : 0;
		}

		/*
		 * If b's exponent is not zero, then we have some calculating to do.
		 */
		else if (((b->fpv.t.exponent != 0) && (b->fpc != IEEEZero)) ||
				 (b->fpc != IEEEZero))
		{

			/*
			 * If the absolute value of a is less than b, then swap a and b.
			 */
			if ((a->fpv.t.exponent < b->fpv.t.exponent) ||
				((a->fpv.t.exponent == b->fpv.t.exponent) &&
				 (a->fpv.t.fraction < b->fpv.t.fraction)))
			{
				a = &instr->src2v.fp;
				b = &instr->src1v.fp;
			}

			/*
			 * Get the difference between the two exponents.  This is, potentially,
			 * for the denormalization of b.
			 */
			expDiff = a->fpv.t.exponent - b->fpv.t.exponent;

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
					b->fpv.t.fraction = 1;

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
					sticky = (b->fpv.t.fraction << (64 - expDiff)) ? 1: 0;
					b->fpv.t.fraction = (b.fpv.t.fraction >> expDiff) | sticky;
				}
			}

			/*
			 * If the signs don't match, but one is set, then we are performing
			 * a Subtraction.
			 */
			if ((a->fpv.t.sign ^ b->fpv.t.sign) != 0)
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
						b->fpv.t.fraction = 1;

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
						sticky = (b->fpv.t.fraction << (64 - expDiff)) ? 1: 0;
						b->fpv.t.fraction = (b.fpv.t.fraction >> expDiff) | sticky;
					}
				}

				/*
				 * Normalize the result.
				 */
				if (ieee == false)
					fpNormalize(&a);
				else if (a->fpv.t.fraction == 0)
				{
					a->fpv.t.exponent = 0;
					a->fpv.t.sign = (roundMode == AXP_FP_MINUS_INF) ? 1 : 0;
				}
				else
					fpNormalize(&a);
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
						b->fpv.t.fraction = 0;

					/*
					 * A non-zero differnce, means that we have to shift b to
					 * compensate for this difference (so the exponents can
					 * have the same value.
					 */
					else if (expDiff != 0)
						b->fpv.t.fraction >>= expDiff;
				}

				/*
				 * Perform the operation (in this case it is a subtraction).
				 */
				a->fpv.t.fraction += b->fpv.t.fraction;

				/*
				 * If a is less than b, then we need to perform a shift and
				 * carry.  There is also no need to normalize the result,
				 * because it already will be.
				 *
				 * TODO:	This code does not look correct.  How are we
				 * 			supposed to handle these "special" values.
				 */
				if (a->fpv.t.fraction < b->fpv.t.fraction)
				{
					a->fpv.t.fraction =
						0x8000000000000000ll | (a->fpv.t.fraction >> 1);
					a->fpv.t.exponent += 1;
				}
			}
		}

		/*
		 * OK, we now need to pack the sign, exponent, and fraction back into
		 * their appropriate places, with the appropriate lengths in register
		 * format.
		 */
		instr->destv.fp.fpv = a->fpv;
		if (a->fpv.t.exponent == AXP_R_NAN)
		{
			if (a->fpv.t.fraction == 0)
				instr->destv.fp.fpc = IEEENotANumber;
			else
				instr->destv.fp.fpc = IEEEInfinity;
		}
		else if (a->fpv.t.exponent == 0)
		{
			if (a->fpv.t.fraction == 0)
				instr->destv.fp.fpc = IEEEZero;
			else
				instr->destv.fp.fpc = IEEEDenormal;
		}
		else
			instr->destv.fp.fpc = IEEEFinite;
	}

	/*
	 * TODO: Need to perform rounding functionality.
	 */

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}

/* Round and pack
   Much of the treachery of the IEEE standard is buried here
   - Rounding modes (chopped, +infinity, nearest, -infinity)
   - Inexact (set if there are any rounding bits, regardless of rounding)
   - Overflow (result is infinite if rounded, max if not)
   - Underflow (no denorms!)

   Underflow handling is particularly complicated
   - Result is always 0
   - UNF and INE are always set in FPCR
   - If /U is set,
     o If /S is clear, trap
     o If /S is set, UNFD is set, but UNFZ is clear, ignore UNFD and
       trap, because the hardware cannot produce denormals
     o If /S is set, UNFD is set, and UNFZ is set, do not trap
   - If /SUI is set, and INED is clear, trap */
/*
 * TODO:	The below code needs to be adjusted for this application.  We need
 * 			the rounding and unpacking (though the unpacking has not been
 * 			written, yet).
 */
typedef struct
{
    u32	sign;
    i32	exp;
    u64	frac;
} UFP;
#define FPR_V_SIGN      63
#define I_GETFRND(p)	(p)
#define FPCR_GETFRND(p)	(p)
#define TRAP_SWC        0x001                           /* software completion */
#define TRAP_INV        0x002                           /* invalid operand */
#define TRAP_DZE        0x004                           /* divide by zero */
#define TRAP_OVF        0x008                           /* overflow */
#define TRAP_UNF        0x010                           /* underflow */
#define TRAP_INE        0x020                           /* inexact */
#define TRAP_IOV        0x040                           /* integer overflow */
#define TRAP_SUMM_RW 0x07F
#define UF_SRND         0x0000008000000000              /* S normal round */
#define UF_SINF         0x000000FFFFFFFFFF              /* S infinity round */
#define UF_TRND         0x0000000000000400              /* T normal round */
#define UF_TINF 0x00000000000007FF /* T infinity round */
#define S_BIAS 0x7F
#define T_BIAS 0x3FF
#define S_M_EXP 0xFF
#define T_M_EXP 0x7FF
#define  I_FRND_C       0                               /* chopped */
#define  I_FRND_M       1                               /* to minus inf */
#define  I_FRND_N       2                               /* normal */
#define  I_FRND_D       3                               /* dynamic */
#define  I_FRND_P       3                               /* in FPCR: plus inf */
#define FPCR_IOV        0x02000000                      /* integer overflow */
#define FPCR_INE        0x01000000                      /* inexact */
#define FPCR_UNF        0x00800000                      /* underflow */
#define FPCR_OVF        0x00400000                      /* overflow */
#define FPCR_DZE        0x00200000                      /* div by zero */
#define FPCR_INV        0x00100000                      /* invalid operation */
#define FPCR_OVFD       0x00080000                      /* overflow disable */
#define FPCR_DZED       0x00040000                      /* div by zero disable */
#define FPCR_INVD       0x00020000                      /* invalid op disable */
#define FPCR_DNZ        0x00010000                      /* denormal to zero */
#define FPCR_DNOD       0x00008000                      /* denormal disable */
#define FPCR_RAZ        0x00007FFF                      /* zero */
#define FPCR_SUM        0x80000000                      /* summary */
#define FPCR_INED       0x40000000                      /* inexact disable */
#define FPCR_UNFD       0x20000000                      /* underflow disable */
#define FPCR_UNDZ 0x10000000 /* underflow to 0 */
#define QNAN            0x0008000000000000ll              /* quiet NaN flag */
#define CQNAN           0xFFF8000000000000ll              /* canonical quiet NaN */
#define FPZERO          0x0000000000000000ll              /* plus zero (fp) */
#define FMZERO          0x8000000000000000ll              /* minus zero (fp) */
#define FPINF           0x7FF0000000000000ll              /* plus infinity (fp) */
#define FMINF           0xFFF0000000000000ll              /* minus infinity (fp) */
#define FPMAX           0x7FEFFFFFFFFFFFFFll              /* plus MAX (fp) */
#define FMMAX           0xFFEFFFFFFFFFFFFFll              /* minus MAX (fp) */
#define IPMAX           0x7FFFFFFFFFFFFFFFll              /* plus MAX (int) */
#define IMMAX           0x8000000000000000ll              /* minus MAX (int) */
#define M64				FMMAX
#define UF_NM 0x8000000000000000 /* normalized */
#define I_FTRP (I_M_FTRP << I_V_FTRP)
#define FPR_V_EXP 52
#define  I_F_VAXRSV     0x4800                          /* VAX reserved */
#define  I_FTRP_V       0x2000                          /* /V trap */
#define  I_FTRP_U       0x2000                          /* /U trap */
#define  I_FTRP_S       0x8000                          /* /S trap */
#define  I_FTRP_SUI     0xE000                          /* /SUI trap */
#define  I_FTRP_SVI     0xE000                          /* /SVI trap */
#define FPR_GUARD (63 - FPR_V_EXP)
#define FPR_FRAC 0x000FFFFFFFFFFFFFll

u64 ieee_rpack (UFP *r, u32 ir, u32 dp)
{
	static const u64 stdrnd[2] = { UF_SRND, UF_TRND };
	static const u64 infrnd[2] = { UF_SINF, UF_TINF };
	static const i32 expmax[2] = { T_BIAS - S_BIAS + S_M_EXP - 1, T_M_EXP - 1 };
	static const i32 expmin[2] = { T_BIAS - S_BIAS, 0 };
	u64 rndadd, rndbits, res, fpcr;
	u32 rndm;

	if (r->frac == 0)                                       /* result 0? */
		return ((u64) r->sign << FPR_V_SIGN);
	rndm = I_GETFRND (ir);                                  /* inst round mode */
	if (rndm == I_FRND_D)
		rndm = FPCR_GETFRND (fpcr);       /* dynamic? use FPCR */
	rndbits = r->frac & infrnd[dp];                         /* isolate round bits */
	if (rndm == I_FRND_N)
		rndadd = stdrnd[dp];              /* round to nearest? */
	else if (((rndm == I_FRND_P) && !r->sign) ||            /* round to inf and */
			 ((rndm == I_FRND_M) && r->sign))                    /* right sign? */
		rndadd = infrnd[dp];
	else
		rndadd = 0;
	r->frac = (r->frac + rndadd) & M64;                     /* round */
	if ((r->frac & UF_NM) == 0)
	{                           /* carry out? */
		r->frac = (r->frac >> 1) | UF_NM;                   /* renormalize */
		r->exp = r->exp + 1;
    }
	if (rndbits)                                            /* inexact? */
		ieee_trap(TRAP_INE, Q_SUI (ir), FPCR_INED, ir);    /* set inexact */
	if (r->exp > expmax[dp])
	{                              /* ovflo? */
		ieee_trap (TRAP_OVF, 1, FPCR_OVFD, ir);             /* set overflow trap */
		ieee_trap (TRAP_INE, Q_SUI (ir), FPCR_INED, ir);    /* set inexact */
		if (rndadd)                                         /* did we round? */
			return (r->sign? FMINF: FPINF);                 /* return infinity */
		return (r->sign? FMMAX: FPMAX);                     /* no, return max */
    }
	if (r->exp <= expmin[dp])
	{                             /* underflow? */
		ieee_trap (TRAP_UNF, ir & I_FTRP_U,                 /* set underflow trap */
				(fpcr & FPCR_UNDZ)? FPCR_UNFD: 0, ir);          /* (dsbl only if UNFZ set) */
		ieee_trap (TRAP_INE, Q_SUI (ir), FPCR_INED, ir);    /* set inexact */
		return(0);                                           /* underflow to +0 */
    }
	res = (((u64) r->sign) << FPR_V_SIGN) |            /* form result */
			(((u64) r->exp) << FPR_V_EXP) |
			((r->frac >> FPR_GUARD) & FPR_FRAC);
	if ((rndm == I_FRND_N) && (rndbits == stdrnd[dp]))      /* nearest and halfway? */
		res = res & ~1;                                     /* clear lo bit */
	return(res);
}
