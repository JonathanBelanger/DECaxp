/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *	Floating-Point Control functionality of the Fbox.
 *
 * Revision History:
 *
 *	V01.000		24-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		25-Jun-2017	Jonathan D. Belanger
 *	Floating point registers are not just 64-bit values, but now have a
 *	structure to them for the various floating-point types.
 *
 *	V01.002		25-Feb-2018	Jonathan D. Belanger
 *	The branch instructions that add a displacement need to use the PC for the
 *	instruction itself, rather than the most recent one on the VPC stack.  This
 *	is because there may have been a number of instructions that are queued or
 *	even executed (but not yet retired) that added a PC to the VPC stack.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox_Control.h"
#include "AXP_21264_Ibox_PCHandling.h"

/*
 * AXP_FBEQ
 *	This function implements the Floating-Point Control Branch if Register
 *	Equal to Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is equal to 0.0,
     * then adjust the PC by the displacement.
     */
    if ((instr->src1v.fp.fpr.exponent == 0)
	&& (instr->src1v.fp.fpr.fraction == 0))
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FBGE
 *	This function implements the Floating-Point Control Branch if Register
 *	Greater than or Equal to Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is greater than or
     * equal to 0.0, then adjust the PC by the displacement.
     */
    if (instr->src1v.fp.uq <= AXP_R_SIGN)
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FBGT
 *	This function implements the Floating-Point Control Branch if Register
 *	Greater Than Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is greater than 0.0,
     * then adjust the PC by the displacement.
     */
    if ((instr->src1v.fp.fpr.sign == 0) && (instr->src1v.fp.uq != 0))
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FBLE
 *	This function implements the Floating-Point Control Branch if Register Less
 *	Than or Equal to Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is less than or equal
     * to 0.0, then adjust the PC by the displacement.
     */
    if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq == 0))
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FBLT
 *	This function implements the Floating-Point Control Branch if Register Less
 *	Than Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is less than 0.0,
     * then adjust the PC by the displacement.
     */
    if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq != 0))
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FBNE
 *	This function implements the Floating-Point Control Branch if Register Not
 *	Equal to Zero instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_FBNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC pc;

    /*
     * Implement the instruction.
     *
     * First we need the PC for the instruction immediately after the PC for
     * this branch instruction.
     */
    pc = instr->pc;
    pc.pc++;

    /*
     * If the conditions have been met, the value of src1 is not equal to 0.0,
     * then adjust the PC by the displacement.
     */
    if ((instr->src1v.fp.uq & ~AXP_R_SIGN) != 0)
	instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    else
    {
	instr->branchPC.pal = 0;
	instr->branchPC.res = 0;
	instr->branchPC.pc = 0;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}
