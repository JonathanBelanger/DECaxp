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
 * Revision History:
 *
 *	V01.000		24-June-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		07-Jul-2017	Jonathan D. Belanger
 *	Renamed and left just the IEEE instructions implemented within.
 *
 *	V01.002		11-Jul-2017	Jonathan D. Belanger
 *	Removed the code that was trying to use MPFR.
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
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the math,
	 * and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);
	src2v = AXP_FP_CvtFPRToFloat(instr->src2v.fp);

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
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
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);
	src2v = *((double *) &instr->src2v.fp.uq);

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
    }

    raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
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
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    AXP_FP_ENCODING src1Enc, src2Enc;
    double *src1v = (double *) &instr->src1v.fp.uq;
    double *src2v = (double *) &instr->src2v.fp.uq;
    int raised = 0;

    src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr, true);
    src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr, true);

    if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
    {
	if ((fpFunc->trp & (AXP_FP_TRP_U | AXP_FP_TRP_S)) != 0)
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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
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
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    AXP_FP_ENCODING src1Enc, src2Enc;
    double *src1v = (double *) &instr->src1v.fp.uq;
    double *src2v = (double *) &instr->src2v.fp.uq;
    int raised = 0;

    src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr, true);
    src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr, true);

    if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
    {
	if ((fpFunc->trp & (AXP_FP_TRP_U | AXP_FP_TRP_S)) != 0)
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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
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
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    AXP_FP_ENCODING src1Enc, src2Enc;
    double *src1v = (double *) &instr->src1v.fp.uq;
    double *src2v = (double *) &instr->src2v.fp.uq;
    int raised = 0;

    src1Enc = AXP_FP_ENCODE(&instr->src1v.fp.fpr, true);
    src2Enc = AXP_FP_ENCODE(&instr->src2v.fp.fpr, true);

    if ((src1Enc == NotANumber) || (src2Enc == NotANumber))
    {
	if ((fpFunc->trp & (AXP_FP_TRP_U | AXP_FP_TRP_S)) != 0)
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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
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
    if ((AXP_FP_ENCODE(&instr->src1v.fp.fpr,true) == NotANumber)
	|| (AXP_FP_ENCODE(&instr->src1v.fp.fpr,true) == NotANumber))
	instr->destv.fp.uq = AXP_T_TWO;
    else
	instr->destv.fp.uq = AXP_FPR_ZERO;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_CVTTQ
 *	This function implements the IEEE T Format Floating-Point ConVerT to
 *	Integer Quadword instruction of the Alpha AXP processor.
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
 * 							Integer Overflow
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_CVTTQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v;
    u64 destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, NULL) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);

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
	destv = src1v;

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
    }

    raised &= (FE_INEXACT | FE_OVERFLOW | FE_INVALID);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = destv;

    /*
     * Set the return code, based on any raised exceptions
     */
    if (raised & FE_INVALID)
	retVal = IllegalOperand;
    else if (raised & (FE_INEXACT | FE_OVERFLOW))
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, true);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_CVTQS
 *	This function implements the Integer Quadword ConVerT to IEEE S
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
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_CVTQS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    u64 src1v;
    float destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

    /*
     * Convert from 64-bit register format to 32-bit float, perform the math,
     * and then convert the result back into 64-bit register format.
     */
    src1v = instr->src1v.fp.uq;

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
    destv = src1v;

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
    raised &= FE_INEXACT;

    /*
     * Convert the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp = AXP_FP_CvtFloatToFPR(destv);
    else
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, false);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_CVTQT
 *	This function implements the Integer Quadword ConVerT to IEEE T Format
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
 * 	ArithmeticTraps:	An arithmetic trap has occurred:
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_CVTQT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    u64 src1v;
    double destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

    /*
     * Convert from 64-bit register format to 64-bit integer, perform the math,
     * and then convert the result back into 64-bit register format.
     */
    src1v = (u64) instr->src1v.fp.uq;

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
    destv = src1v;

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
    raised &= FE_INEXACT;

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);
    else
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, true);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_CVTST
 *	This function implements the IEEE S Format Floating-Point ConVerT to
 *	IEEE T Format Floating-Point instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTST(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v;
    double destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, NULL) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);

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
	destv = src1v;

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
    }

    raised &= FE_INVALID;

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);
    else
	retVal = IllegalOperand;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, true);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_CVTTS
 *	This function implements the IEEE T Format Floating-Point ConVerT to
 *	IEEE S Format Floating-Point instruction of the Alpha AXP processor.
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
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_CVTTS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v;
    float destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, NULL) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);

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
	destv = src1v;

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
    }

    raised &= (FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

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
    AXP_FP_SetFPCR(cpu, instr, raised, true);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_DIVS
 *	This function implements the IEEE S Format Floating-Point DIVide
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
 * 							Division by Zero
 * 							Overflow
 * 							Underflow
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_DIVS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the math,
	 * and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);
	src2v = AXP_FP_CvtFPRToFloat(instr->src2v.fp);

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
	destv = src1v / src2v;

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
    }

    /*
     * We only care about this set of exceptions.
     */
    raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID
	| FE_DIVBYZERO);

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
    else if (raised & (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO))
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, false);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_DIVT
 *	This function implements the IEEE T Format Floating-Point DIVide
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
 * 							Divide by Zero
 */
AXP_EXCEPTIONS AXP_DIVT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);
	src2v = *((double *) &instr->src2v.fp.uq);

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
	destv = src1v / src2v;

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
    }

    raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID
	| FE_DIVBYZERO);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

    /*
     * Set the return code, based on any raised exceptions
     */
    if (raised & FE_INVALID)
	retVal = IllegalOperand;
    else if (raised & (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO))
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, false);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_FTOIS
 *	This function implements the IEEE S Format Floating-Point Move to Integer
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
AXP_EXCEPTIONS AXP_FTOIS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Execute the instruction.
     */
    instr->destv.r.uq = ((
	instr->src1v.fp.fpr32.sign == 1 ? 0xffffffff80000000ll : 0)
	| ((instr->src1v.fp.uq & 0x4000000000000000ll) >> 24)
	| ((instr->src1v.fp.uq & 0x07FFFFFFE0000000ll) >> 29));

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FTOIT
 *	This function implements the IEEE T Format Floating-Point Move to Integer
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
AXP_EXCEPTIONS AXP_FTOIT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Execute the instruction.
     */
    instr->destv.r.uq = instr->src1v.fp.uq;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_ITOFS
 *	This function implements the Integer Move to IEEE S Format Floating-Point
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
AXP_EXCEPTIONS AXP_ITOFS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    float src1v = *((float *) &instr->src1v.r.ul);

    /*
     * Execute the instruction.
     */
    instr->destv.fp = AXP_FP_CvtFloatToFPR(src1v);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_ITOFT
 *	This function implements the Integer Move to IEEE T Format Floating-Point
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
AXP_EXCEPTIONS AXP_ITOFT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Execute the instruction.
     */
    instr->destv.fp.uq = instr->src1v.r.uq;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_MULS
 *	This function implements the IEEE S Format Floating-Point MULtiply
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
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_MULS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the math,
	 * and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);
	src2v = AXP_FP_CvtFPRToFloat(instr->src2v.fp);

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
	destv = src1v * src2v;

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_MULT
 *	This function implements the IEEE T Format Floating-Point MULtiply
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
AXP_EXCEPTIONS AXP_MULT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);
	src2v = *((double *) &instr->src2v.fp.uq);

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
	destv = src1v * src2v;

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
    }

    raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_SQRTS
 *	This function implements the IEEE S Format Floating-Point SQuare RooT
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
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_SQRTS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, NULL) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the math,
	 * and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);

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
	destv = sqrt(src1v);

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
    }

    /*
     * We only care about this set of exceptions.
     */
    raised &= (FE_INEXACT | FE_INVALID);

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
    else if (raised & FE_INEXACT)
	retVal = ArithmeticTraps;

    /*
     * Set the FPCR and ExcSum registers.
     */
    AXP_FP_SetFPCR(cpu, instr, raised, false);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_SQRTT
 *	This function implements the IEEE T Format Floating-Point SQuare RooT
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
 * 							Inexct Result
 */
AXP_EXCEPTIONS AXP_SQRTT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, NULL) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);

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
	destv = sqrt(src1v);

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
    }

    raised &= (FE_INEXACT | FE_INVALID);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_SUBS
 *	This function implements the IEEE S Format Floating-Point SUBtract
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
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_SUBS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    float src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 32-bit float, perform the math,
	 * and then convert the result back into 64-bit register format.
	 */
	src1v = AXP_FP_CvtFPRToFloat(instr->src1v.fp);
	src2v = AXP_FP_CvtFPRToFloat(instr->src2v.fp);

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
	destv = src1v - src2v;

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_SUBT
 *	This function implements the IEEE T Format Floating-Point SUBtract
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
 * 							Inexact Result
 */
AXP_EXCEPTIONS AXP_SUBT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    AXP_FP_FUNC *fpFunc = (AXP_FP_FUNC *) &instr->function;
    double src1v, src2v, destv;
    int oldRndMode = 0;
    int oldExcMode = 0;
    int raised = 0;

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
    if (AXP_FP_CheckForIEEEInvalid(&instr->src1v.fp, &instr->src2v.fp) == true)
	raised = FE_INVALID;

    if (raised == 0)
    {

	/*
	 * Convert from 64-bit register format to 64-bit double, perform the
	 * math, and then convert the result back into 64-bit register format.
	 */
	src1v = *((double *) &instr->src1v.fp.uq);
	src2v = *((double *) &instr->src2v.fp.uq);

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
	destv = src1v - src2v;

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
    }

    raised &= (FE_INEXACT | FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

    /*
     * Copy the results into the destination register value.
     */
    if (raised == 0)
	instr->destv.fp.uq = *((u64 *) &destv);

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
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}
