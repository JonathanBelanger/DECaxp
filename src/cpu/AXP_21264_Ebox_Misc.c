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
 *	Miscellaneous instructions of the 21264 Alpha AXP CPU.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox_Misc.h"

/*
 * AXP_AMASK
 *	This function implements the Architecture Mask instruction of the Alpha AXP
 *	processor.
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
AXP_EXCEPTIONS AXP_AMASK(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64	Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);

	/*
	 * Return the masked off CPU Features.
	 */
	switch (cpu->majorType)
	{
		case EV3:
		case Simulation:
		case EV4:					// 21064
		case EV45:					// 21064A
		case LCAFamily:				// 21066, 21068, 20166A, 20168A
		case EV5:					// 21164
		case PCA56:					// 21164PC
		case PCA57:
			instr->destv.r.uq = Rbv;
			break;

		case EV56:					// 21164A
		case EV6:					// 21264
		case EV67:					// 21264
		case EV68CB_DC:				// 21264
		case EV68A:					// 21264
		case EV68CX:				// 21264
		case EV69A:					// 21264
		case EV7:					// 21364
		case EV79:					// 21364
			switch (cpu->iCtl.chip_id)
			{
				case 3:
					instr->destv.r.uq = Rbv & ~0x0000000000000001ll;
					break;

				case 4:
					instr->destv.r.uq = Rbv & ~0x0000000000000303ll;
					break;
			}
			break;
	}

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
 * AXP_CALL_PAL
 *	This function implements the CALL Privileged Architecture Logic instruction
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
AXP_EXCEPTIONS AXP_CALL_PAL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_PC	*retPC = (AXP_PC *) &instr->destv.r.uq;

	/*
	 * The destination register was set to the R23 (R39) shadow register or
	 * R27 (does not have a shadow register).
	 *
	 * TODO: This return address will need to be pushed onto the return stack.
	 */
	*retPC = AXP_21264_GetNextVPC(cpu);

	/*
	 * CALL_PAL is just like a branch, but it is not predicted.
	 */
	instr->branchPC = AXP_21264_GetPALFuncVPC(cpu, instr->function);

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
 * AXP_IMPLVER
 *	This function implements the Implementation Version instruction of the
 *	Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_IMPLVER(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Return the Implementation Version value.
	 */
	switch (cpu->majorType)
	{
		case EV3:
		case Simulation:
			instr->destv.r.uq = 0xffffffffffffffffll;
			break;

		case EV4:					// 21064
		case EV45:					// 21064A
		case LCAFamily:				// 21066, 21068, 20166A, 20168A
			instr->destv.r.uq = 0x0000000000000000ll;
			break;

		case EV5:					// 21164
		case EV56:					// 21164A
		case PCA56:					// 21164PC
		case PCA57:
			instr->destv.r.uq = 0x0000000000000001ll;
			break;

		case EV6:					// 21264
		case EV67:					// 21264
		case EV68CB_DC:				// 21264
		case EV68A:					// 21264
		case EV68CX:				// 21264
		case EV69A:					// 21264
			instr->destv.r.uq = 0x0000000000000002ll;
			break;

		case EV7:					// 21364
		case EV79:					// 21364
			instr->destv.r.uq = 0x0000000000000003ll;
			break;
	}

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(NoException);
}
