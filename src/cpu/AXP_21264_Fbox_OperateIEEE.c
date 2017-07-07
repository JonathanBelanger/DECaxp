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
 *	Renamed and left just the IEEE instructions implemented within.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox_OperateIEEE.h"

/*
 * AXP_ADDS
 *	This function implements the IEEE S Format Floating-Point ADD instruction
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
AXP_EXCEPTIONS AXP_ADDS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	float			src1v, src2v, destv;
	int				oldRndMode = 0;
	int				oldExcMode = 0;
	int				raised = 0;
	AXP_FP_ENCODING	src1Enc;
	AXP_FP_ENCODING	src2Enc;

	/*
	 * I was relying upon the calculation below to trigger invalid operation
	 * when the operands are one of the following:
	 *
	 *		A signaling NaN
	 *		Unlike-signed infinities, such as (+infinity + –infinity)
	 *
	 * Unfortunately, it does not apparently do this.  So, I'm going to have to
	 * do it myself.
	 */
	src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr32, true);
	src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr32, true);
	if ((src1Enc == Infinity) && (src2Enc == Infinity))
	{
		if (instr->src1v.fp.fpr32.sign != instr->src2v.fp.fpr32.sign)
			raised = FE_INVALID;
	}
	else if ((src1Enc == NotANumber) && (instr->src1v.fp.fprQ32.quiet == 0))
		raised = FE_INVALID;
	else if ((src2Enc == NotANumber) && (instr->src2v.fp.fprQ32.quiet == 0))
		raised = FE_INVALID;

	if (raised == 0)
	{

		/*
		 * Convert from 64-bit register format to 32-bit float, perform the math,
		 * and then convert the result back into 64-bit register format.
		 */
		src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);
		src2v = AXP_FP_CvtFPRToFloat(instr->src2v.fp);

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
		 * Disable any exception modes indicating in the FPCR.
		 */
		oldExcMode = AXP_FP_SetExceptionMode(cpu, oldExcMode);

		/*
		 * Execute the instruction.
		 */
		destv = src1v + src2v;

		/*
		 * Test to see what exceptions were raised.
		 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
		 * Enable any exception modes that were set prior to getting into this
		 * function.
		 */
		oldExcMode = AXP_FP_SetExceptionMode(NULL, oldExcMode);

		/*
		 * Reset the rounding mode
		 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.
	}

	/*
	 * We only care about this set of exceptions.
	 */
	raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

	/*
	* Copy the results into the destination register value.
	 */
	if (raised == 0)
		instr->destv.fp = AXP_FP_CvtFloatToFPR(destv);

	/*
	 * Set the return code, based on any raised exceptions
	 */
	if (raised & FE_INVALID)
		retVal = IllegalOperand;
	else if (raised & (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW))
		retVal = ArithmeticTraps;

	/*
	 * Set the FPCR and ExcSum registers.
	 */
	AXP_FP_SetFPCR(cpu, instr, raised, false);

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
 * AXP_ADDT
 *	This function implements the IEEE T Format Floating-Point ADD instruction
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
AXP_EXCEPTIONS AXP_ADDT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode = 0;
	int				oldExcMode = 0;
	int				raised = 0;
	AXP_FP_ENCODING	src1Enc;
	AXP_FP_ENCODING	src2Enc;

	/*
	 * I was relying upon the calculation below to trigger invalid operation
	 * when the operands are one of the following:
	 *
	 *		A signaling NaN
	 *		Unlike-signed infinities, such as (+infinity + –infinity)
	 *
	 * Unfortunately, it does not apparently do this.  So, I'm going to have to
	 * do it myself.
	 */
	src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr, true);
	src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr, true);
	if ((src1Enc == Infinity) && (src2Enc == Infinity))
	{
		if (instr->src1v.fp.fpr.sign != instr->src2v.fp.fpr.sign)
			raised = FE_INVALID;
	}
	else if ((src1Enc == NotANumber) && (instr->src1v.fp.fprQ.quiet == 0))
		raised = FE_INVALID;
	else if ((src2Enc == NotANumber) && (instr->src2v.fp.fprQ.quiet == 0))
		raised = FE_INVALID;

	if (raised == 0)
	{

		/*
		 * Convert from 64-bit register format to 32-bit float, perform the math,
		 * and then convert the result back into 64-bit register format.
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
		 * Disable any exception modes indicating in the FPCR.
		*/
		oldExcMode = AXP_FP_SetExceptionMode(cpu, oldExcMode);

		/*
		 * Execute the instruction.
		 */
		destv = src1v + src2v;

		/*
		 * Test to see what exceptions were raised.
 		 */
		raised = fetestexcept(FE_ALL_EXCEPT);

		/*
		 * Enable any exception modes that were set prior to getting into this
		 * function.
		 */
		oldExcMode = AXP_FP_SetExceptionMode(NULL, oldExcMode);

		/*
	 	 * Reset the rounding mode
	 	 */
		oldRndMode = AXP_FP_SetRoundingMode(NULL, NULL, oldRndMode);

		// TODO: We need to have a mutex ending at this point.
	}

	raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

	/*
	 * Copy the results into the destination register value.
	 */
	if (raised == 0)
		instr->destv.fp.uq = (u64) destv;

	/*
	 * Set the return code, based on any raised exceptions
	 */
	if (raised & FE_INVALID)
		retVal = IllegalOperand;
	else if (raised & (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW))
		retVal = ArithmeticTraps;

	/*
	 * Set the FPCR and ExcSum registers.
	 */
	AXP_FP_SetFPCR(cpu, instr, raised, false);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
