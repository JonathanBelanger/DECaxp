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
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox_Operate.h"

/*
 * AXP_CPYS
 * 	This function implements the Floating-Point Operate Copy Sign instruction of
 * 	the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CPYS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.fpr = instr->src2v.fp.fpr;
	instr->destv.fp.fpr.sign = instr->src1v.fp.fpr.sign;

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
 * AXP_CPYSE
 * 	This function implements the Floating-Point Operate Copy Sign and Exponent
 * 	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CPYSE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.fpr.sign = instr->src1v.fp.fpr.sign;
	instr->destv.fp.fpr.exponent = instr->src1v.fp.fpr.exponent;
	instr->destv.fp.fpr.fraction = instr->src2v.fp.fpr.fraction;

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
 * AXP_CPYSN
 * 	This function implements the Floating-Point Operate Copy Sign Negate
 * 	instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CPYSN(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.fpr = instr->src2v.fp.fpr;
	instr->destv.fp.fpr.sign = ~instr->src1v.fp.fpr.sign;

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
 * AXP_CVTLQ
 * 	This function implements the Floating-Point Operate Convert Longword to
 * 	Quadword instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTLQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.qCvt.sign = instr->src1v.fp.l.sign;
	instr->destv.fp.qCvt.integerHigh = instr->src1v.fp.l.integerHigh;
	instr->destv.fp.qCvt.integerLow = instr->src1v.fp.l.integerLow;

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
 * AXP_CVTQL
 * 	This function implements the Floating-Point Operate Convert Quadword to
 * 	Longword without overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTQL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.l.sign = instr->src1v.fp.qVCvt.sign;
	instr->destv.fp.l.integerHigh = instr->src1v.fp.qVCvt.integerLowHigh;
	instr->destv.fp.l.zero_2 = 0;
	instr->destv.fp.l.integerLow = instr->src1v.fp.qVCvt.integerLowLow;
	instr->destv.fp.l.zero_1 = 0;

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
 * AXP_CVTQL_V
 * 	This function implements the Floating-Point Operate Convert Quadword to
 * 	Longword with overflow instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_CVTQL_V(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.l.sign = instr->src1v.fp.qVCvt.sign;
	instr->destv.fp.l.integerHigh = instr->src1v.fp.qVCvt.integerLowHigh;
	instr->destv.fp.l.zero_2 = 0;
	instr->destv.fp.l.integerLow = instr->src1v.fp.qVCvt.integerLowLow;
	instr->destv.fp.l.zero_1 = 0;

	if (AXP_R_Q2L_OVERFLOW(instr->src1v.fp.uq))
		retVal = ArithmeticTraps;	// IntegerOverflow;
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
 * AXP_CVTQL_SV
 * 	This function implements the Floating-Point Operate Convert Quadword to
 * 	Longword with overflow and software completion instruction of the Alpha AXP
 * 	processor.
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
AXP_EXCEPTIONS AXP_CVTLQ_SV(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * Implement the instruction.
	 */
	instr->destv.fp.l.sign = instr->src1v.fp.qVCvt.sign;
	instr->destv.fp.l.integerHigh = instr->src1v.fp.qVCvt.integerLowHigh;
	instr->destv.fp.l.zero_2 = 0;
	instr->destv.fp.l.integerLow = instr->src1v.fp.qVCvt.integerLowLow;
	instr->destv.fp.l.zero_1 = 0;

	/*
	 * TODO:	We need to understand how software-completion differs from just
	 *			reporting the exception.
	 */
	if (AXP_R_Q2L_OVERFLOW(instr->src1v.fp.uq))
		retVal = ArithmeticTraps;	// IntegerOverflow;
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
 * AXP_FCMOVEQ
 *	This function implements the Floating-Point Conditional Move if Equal
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
AXP_EXCEPTIONS AXP_FCMOVEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.fp.fpr.exponent == 0) &&
		(instr->src1v.fp.fpr.fraction == 0))
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_FCMOVGE
 *	This function implements the Floating-Point Conditional Move if Greater
 *	Than or Equal instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FCMOVGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if (instr->src1v.fp.uq <= AXP_R_SIGN)
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_FCMOVGT
 *	This function implements the Floating-Point Conditional Move if Greater
 *	Than instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FCMOVGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.fp.fpr.sign == 0) && (instr->src1v.fp.uq != 0))
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_FCMOVLE
 *	This function implements the Floating-Point Conditional Move if Less Than
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
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_FCMOVLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq == 0))
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_FCMOVLT
 *	This function implements the Floating-Point Conditional Move if Less Than
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
AXP_EXCEPTIONS AXP_FCMOVLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq != 0))
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_FCMOVNE
 *	This function implements the Floating-Point Conditional Move if Not Equal
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
AXP_EXCEPTIONS AXP_FCMOVNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	if ((instr->src1v.fp.uq & ~AXP_R_SIGN) != 0)
		instr->destv.fp.uq = instr->src2v.fp.uq;

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
 * AXP_MF_FPCR
 *	This function implements the Floating-Point Move From Floating-Point
 *	Control Register instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MF_FPCR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 */
	instr->destv.fp.uq = *((u64 *) &cpu->fpcr);

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
 * AXP_MT_FPCR
 *	This function implements the Floating-Point Move To Floating-Point
 *	Control Register instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_MT_FPCR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Implement the instruction
	 *
	 * NOTE: The value will actually be moved into the FPCR at the time of
	 * 		 instruction retirement.
	 */
	instr->destv.fp.uq = instr->src1v.fp.uq;

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
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_ADDF(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * The way this emulator is implemented, there is only one function to
	 * handle both add and subtract, as well as any of the floating types
	 * (VAX F, VAX G, IEEE S, and IEEE T).
	 */
	retVal = AXP_FPAddSub(cpu, instr, AXP_F_DT);

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
 * 	An exception indicator.
 */
AXP_EXCEPTIONS AXP_ADDG(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * The way this emulator is implemented, there is only one function to
	 * handle both add and subtract, as well as any of the floating types
	 * (VAX F, VAX G, IEEE S, and IEEE T).
	 */
	retVal = AXP_FPAddSub(cpu, instr, AXP_G_DT);

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
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * The way this emulator is implemented, there is only one function to
	 * handle both add and subtract, as well as any of the floating types
	 * (VAX F, VAX G, IEEE S, and IEEE T).
	 */
	retVal = AXP_FPAddSub(cpu, instr, AXP_S_DT);

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
 * AXP_ADDF
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
	AXP_EXCEPTIONS retVal = NoException;

	/*
	 * The way this emulator is implemented, there is only one function to
	 * handle both add and subtract, as well as any of the floating types
	 * (VAX F, VAX G, IEEE S, and IEEE T).
	 */
	retVal = AXP_FPAddSub(cpu, instr, AXP_T_DT);

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
