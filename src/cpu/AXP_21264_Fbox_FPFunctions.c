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
 * AXP_FP_CvtFPRToFloat
 *	Hey, guess what, the GNU C compiler generates code that is IEEE compliant.
 *	The only thing We are going to do here is to convert the floating-point
 *	register format to a 32-bit float and then use the math run-time library
 *	functions to actually do that work.  This only needs to be called for
 *	IEEE S formatted registers (we need to strip out the high order
 *	fraction and reduce the exponent from 11 to 8 bits.
 *
 *	NOTE: 	This function does not concern itself with potential overflows or
 *			underflows.  This function is called for an IEEE S Float only, so
 *			the 64-bit register containing the value to be converted is assumed
 *			to always fit into an equivalent 32-bit float.
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
	AXP_S_MEMORY	*sFloat = (AXP_S_MEMORY *) &retVal;

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
 *	to a floating-point register format.  This only needs to be called for
 *	IEEE S formatted registers (we need to put back the high order fraction,
 *	clearing out the low order portion, and expend the exponent from 8 to 11
 *	bits.
 *
 *	NOTE: 	This function does not concern itself with potential overflows or
 *			underflows.  This function is called for an IEEE S Float only, so
 *			the 32-bit float containing the value to be converted will always
 *			fit into an equivalent 64-bit IEEE S float (in an IEEE T float
 *			equivalent of the IEEE S float).
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
	AXP_S_MEMORY	*sFloat = (AXP_S_MEMORY *) &real32;

	/*
	 * Convert away.
	 */
	retVal.fpr32.sign = sFloat->sign;
	if (sFloat->exponent == AXP_S_NAN)
		retVal.fpr32.exponent = AXP_T_NAN;
	else if (sFloat->exponent == 0)
		retVal.fpr32.exponent = 0;
	else
		retVal.fpr32.exponent = sFloat->exponent + AXP_T_BIAS - AXP_S_BIAS;
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
 * AXP_FP_SetExceptionMode
 * 	This function is called to disable/enable the exception mode.  It
 * 	determines this based on the FPCR in the cpu structure.  If this is called
 * 	with the cpu equal to NULL, then use the exceptionMode parameter to enable
 * 	the exception mode.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	resetExceptionMode:
 * 		A value of the previous exception mode, which was returned on a
 * 		previous call to this function.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A value representing the previous value for the exception mode.
 */
int AXP_FP_SetExceptionMode(AXP_21264_CPU *cpu, int resetExceptionMode)
{
	int retVal = fegetexcept();
	int exceptionBits = 0;

	if (cpu != NULL)
	{
		if (cpu->fpcr.dzed == 1)
			exceptionBits |= FE_DIVBYZERO;
		if (cpu->fpcr.ined == 1)
			exceptionBits |= FE_INEXACT;
		if (cpu->fpcr.invd == 1)
			exceptionBits |= FE_INVALID;
		if (cpu->fpcr.ovfd == 1)
			exceptionBits |= FE_OVERFLOW;
		if (cpu->fpcr.unfd == 1)
			exceptionBits |= FE_UNDERFLOW;
		if (exceptionBits != 0)
			fedisableexcept(exceptionBits);
		else
			retVal = 0;
	}
	else if (resetExceptionMode != 0)
		feenableexcept(resetExceptionMode);
	return(retVal);
}

/*
 * AXP_FP_SetFPCR
 * 	This function is called to conditionally set the excSum field, and always
 * 	the insFPCR fields in the instruction parameter.  The function field is
 * 	used to determine which qualifier was supplied with the call.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 * 	raised:
 * 		A value from the fetestexcept, used to set the FPCR and conditionally,
 * 		the excSum.
 * 	integerOverflow:
 * 		An indication that the overflow condition is for an integer overflow
 * 		and not a floating point overflow.
 *
 * Output Parameters:
 * 	instr:
 * 		The fpcr and, conditionally, the excSum will be updated, based on the
 * 		exception.
 *
 * Return Value:
 * 	None.
 */
void AXP_FP_SetFPCR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr, int raised, bool integerOverflow)
{
	AXP_FP_FUNC *func = (AXP_FP_FUNC *) &instr->function;
	bool		excSet = false;

	/*
	 * We always set the FPCR
	 */
	if (raised & FE_DIVBYZERO)
	{
		instr->insFpcr.dze = 1;;
		excSet = true;
	}
	if (raised & FE_INVALID)
	{
		instr->insFpcr.inv = 1;
		excSet = true;
	}
	if (raised & FE_OVERFLOW)
	{
		if (integerOverflow)
			instr->insFpcr.iov = 1;
		else
			instr->insFpcr.ovf = 1;
		excSet = true;
	}
    if ((raised & FE_INEXACT) && ((func->trp & AXP_FP_TRP_I) == 0))
	{
    	instr->insFpcr.ine = 1;
		excSet = true;
	}
	if ((raised & FE_UNDERFLOW) && (func->trp & AXP_FP_TRP_U))
	{
		instr->insFpcr.unf = 1;
		excSet = true;
	}

	/*
	 * If we set any IEEE exception bits, which are for the fpcr register, then
	 * go and set these bits.
	 */
	if (excSet)
		instr->insFpcr.sum = 1;

	/*
	 * Go set the excSum register bit fields, as well.
	 */
	AXP_FP_SetExcSum(instr, raised, integerOverflow);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_FP_SetExcSum
 * 	This function is called to conditionally set the excSum bits in the
 * 	instruction parameter.  The function field is used to determine which
 * 	qualifier was supplied with the call, so that we can set, or not, the
 * 	correct bits.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 * 	raised:
 * 		A value from the fetestexcept, used to set the excSum.
 * 	integerOverflow:
 * 		An indication that the overflow condition is for an integer overflow
 * 		and not a floating point overflow.
 *
 * Output Parameters:
 * 	instr:
 * 		The excSum will be updated, based on the exception.
 *
 * Return Value:
 * 	None.
 */
void AXP_FP_SetExcSum(AXP_INSTRUCTION *instr, int raised, bool integerOverflow)
{
	AXP_FP_FUNC *func = (AXP_FP_FUNC *) &instr->function;
	u32	axpExceptions = 0;

	/*
	 * We always set the the following exceptions.
	 */
	if (raised & FE_DIVBYZERO)
		axpExceptions |= AXP_EXC_DIV_BY_ZERO;
	if (raised & FE_INVALID)
		axpExceptions |= AXP_EXC_INV_OPER;
	if (raised & FE_OVERFLOW)
		axpExceptions |=
			(integerOverflow ? AXP_EXC_INT_OVERFLOW : AXP_EXC_FP_OVERFLOW);

	/*
	 * If '/I' is present, then set excSum
	 */
	if ((raised & FE_INEXACT) && (func->trp & AXP_FP_TRP_I))
		axpExceptions |= AXP_EXC_INEXACT_RES;

	/*
	 * If '/U', which is the same as '/V' is present, then set excSum
	 */
	if ((raised & FE_UNDERFLOW) && (func->trp & AXP_FP_TRP_U))
		axpExceptions |= AXP_EXC_UNDERFLOW;

	/*
	 * If we set any AXP exception bits, which are for the excSum register,
	 * then go and set these bits.
	 */
	if (axpExceptions != 0)
	{

		/*
		 * If '/S' is present, then set the software completion bit.
		 */
		if (func->trp & AXP_FP_TRP_S)
			axpExceptions |= AXP_EXC_SW_COMPL;
		AXP_SetException(instr, axpExceptions);
	}

	/*
	 * Return back to the caller.
	 */
	return;
}
