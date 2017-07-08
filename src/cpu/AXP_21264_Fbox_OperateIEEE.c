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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_ADDS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	float			src1v, src2v, destv;
	int				oldRndMode = 0;
	int				oldExcMode = 0;
	int				raised = 0;

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
	if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src1v.fp) == true)
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
 * 	NoException:		Normal successful completion.
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Invalid Operation
 * 							Overflow
 * 							Underflow
 */
AXP_EXCEPTIONS AXP_ADDT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	double			src1v, src2v, destv;
	int				oldRndMode = 0;
	int				oldExcMode = 0;
	int				raised = 0;

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
	if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src1v.fp) == true)
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

/*
 * AXP_CMPTEQ
 *	This function implements the IEEE T Format Floating-Point Compare Equal
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
AXP_EXCEPTIONS AXP_CMPTEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING	src1Enc, src2Enc;
	double			*src1v = (double *) &instr->src1v.fp.uq;
	double			*src2v = (double *) &instr->src2v.fp.uq;
	int				raised = 0;

	src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr,true);
	src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr,true);

	if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
	{
		if (fpFunc->trp == (AXP_FP_TRP_U | AXP_FP_TRP_S))
			raised = FE_INVALID;
		instr->destv.fp.uq = AXP_FPR_ZERO;
	}
	else if (*src1v == *src2v)
		instr->destv.fp.uq = AXP_T_TWO;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
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
 * AXP_CMPTLE
 *	This function implements the IEEE T Format Floating-Point Compare Less Than
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
AXP_EXCEPTIONS AXP_CMPTLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING	src1Enc, src2Enc;
	double			*src1v = (double *) &instr->src1v.fp.uq;
	double			*src2v = (double *) &instr->src2v.fp.uq;
	int				raised = 0;

	src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr,true);
	src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr,true);

	if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
	{
		if (fpFunc->trp == (AXP_FP_TRP_U | AXP_FP_TRP_S))
			raised = FE_INVALID;
		instr->destv.fp.uq = AXP_FPR_ZERO;
	}
	else if (*src1v <= *src2v)
		instr->destv.fp.uq = AXP_T_TWO;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
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
 * AXP_CMPTLT
 *	This function implements the IEEE T Format Floating-Point Compare Less Than
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
AXP_EXCEPTIONS AXP_CMPTLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS	retVal = NoException;
	AXP_FP_FUNC		*fpFunc = (AXP_FP_FUNC *) &instr->function;
	AXP_FP_ENCODING	src1Enc, src2Enc;
	double			*src1v = (double *) &instr->src1v.fp.uq;
	double			*src2v = (double *) &instr->src2v.fp.uq;
	int				raised = 0;

	src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr,true);
	src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr,true);

	if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
	{
		if (fpFunc->trp == (AXP_FP_TRP_U | AXP_FP_TRP_S))
			raised = FE_INVALID;
		instr->destv.fp.uq = AXP_FPR_ZERO;
	}
	else if (*src1v < *src2v)
		instr->destv.fp.uq = AXP_T_TWO;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Set the exception bits (I probably don't have to do this, but I will
	 * anyway, just so that unexpected results get returned for this
	 * instruction).
	 */
	if (raised != 0)
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
 * AXP_CMPTUN
 *	This function implements the IEEE T Format Floating-Point Compare Unordered
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
AXP_EXCEPTIONS AXP_CMPTUN(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Unordered just means that either of the floating-point values is equal
	 * to NaN (Not a Number).
	 */
	if ((AXP_FP_ENCODE(&instr->src1v.fp.fpr,true) == NotANumber) ||
		(AXP_FP_ENCODE(&instr->src1v.fp.fpr,true) == NotANumber))
		instr->destv.fp.uq = AXP_T_TWO;
	else
		instr->destv.fp.uq = AXP_FPR_ZERO;

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
