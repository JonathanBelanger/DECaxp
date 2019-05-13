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
 *  This source file contains the functions needed to implement the
 *  Integer Control functionality of the Ebox.
 *
 * Revision History:
 *
 *  V01.000 22-Jun-2017 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 23-Jun-2017 Jonathan D. Belanger
 *  On branches, we need to save the branched to PC until the instruction is
 *  retired.  The original code was updating it immediately.  This would cause
 *  huge problems, especially around exception and interrupt handling.
 *
 *  V01.002 25-Jun-2017 Jonathan D. Belanger
 *  Change registers to be 64-bit structures to aid in coding.  Needed to
 *  update all the register references.
 *
 *  V01.003 25-Feb-2018 Jonathan D. Belanger
 *  The branch instructions that add a displacement need to use the PC for the
 *  instruction itself, rather than the most recent one on the VPC stack.  This
 *  is because there may have been a number of instructions that are queued or
 *  even executed (but not yet retired) that added a PC to the VPC stack.
 *
 *  V01.004 12-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CPU/Ebox/AXP_21264_Ebox_Control.h"
#include "CPU/Ibox/AXP_21264_Ibox_PCHandling.h"

/*
 * AXP_BEQ
 *  This function implements the Control Branch if Register Equal to Zero
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 is equal to zero,
     * then adjust the PC by the displacement.
     */
    if (instr->src1v.r.sq == 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BGE
 *  This function implements the Control Branch if Register Greater than or
 *  Equal to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * equal to zero, then adjust the PC by the displacement.
     */
    if (instr->src1v.r.sq >= 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BGT
 *  This function implements the Control Branch if Register Greater Than Zero
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 is greater than zero,
     * then adjust the PC by the displacement.
     */
    if (instr->src1v.r.sq > 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BLBC
 *  This function implements the Control Branch if Low Bit Is Clear instruction
 *  of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BLBC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 has its low-order
     * bit clear, then adjust the PC by the displacement.
     */
    if ((instr->src1v.r.uq & 0x01) == 0x00)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BLBS
 *  This function implements the Control Branch if Low Bit Is Set instruction
 *  of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BLBS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 has its low-order
     * bit set, then adjust the PC by the displacement.
     */
    if ((instr->src1v.r.uq & 0x01) == 0x01)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BLE
 *  This function implements the Control Branch if Register Less Than or Equal
 *  to Zero instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 is less than or
     * equal to zero, then adjust the PC by the displacement.
     */
    if (instr->src1v.r.sq <= 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BLT
 *  This function implements the Control Branch if Register Less Than Zero
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 is less than zero,
     * then adjust the PC by the displacement.
     */
    if (instr->src1v.r.sq < 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BNE
 *  This function implements the Control Branch if Register Not Equal to Zero
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * If the conditions have been met, the value of src1 is not equal to zero,
     * then adjust the PC by the displacement.
     */
    if (instr->src1v.r.uq != 0)
    {
        instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);
    }
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
 * AXP_BR
 *  This function implements the Unconditional Branch  instruction of the Alpha
 *  AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * We store the PC calculated above into the destination register value.
     */
    instr->destv.r.uq = AXP_GET_PC(pc);

    /*
     * Now we add a displacement to the PC calculated above.
     */
    instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_BSR
 *  This function implements the Unconditional Branch  instruction of the Alpha
 *  AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_BSR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
    AXP_PUSH(pc);

    /*
     * We store the PC calculated above into the destination register value.
     */
    instr->destv.r.uq = AXP_GET_PC(pc);

    /*
     * Now we add a displacement to the PC calculated above.
     */
    instr->branchPC = AXP_21264_DisplaceVPC(cpu, pc, instr->displacement);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_JMP
 *  This function implements the Jump instructions of the Alpha AXP processor.
 *  This instruction is unique, in that the displacement field is used to
 *  contain the type of Jump is being performed.  The following table indicates
 *  who the displacement field will be interpreted:
 *
 *      disp<15:14>     Meaning         Predicted Target    Prediction Stack  Action
 *      -----------     -------------   ------------------- ------------------------------
 *      00              JMP             PC + (4*disp<13:0   --
 *      01              JSR             PC + (4*disp<13:0>) Push return PC onto stack
 *      10              RET             Prediction stack    Pop new (return) PC from stack
 *      11              JSR_COROUTINE   Prediction stack    Pop, push return
 *
 *  For the last 2, RET(10) and JSR_COROUTINE(11), the encoding for disp<13:0>
 *  have the following meaning:
 *
 *      Encoding    Meaning
 *      --------    ------------------------------
 *      0x0000      Indicates non-procedure return
 *      0x0001      Indicates procedure return
 *                  All other encodings are reserved.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *   instr:
 *       A pointer to a structure containing the information needed to execute
 *       this instruction.
 *
 * Output Parameters:
 *   instr:
 *       The contents of this structure are updated, as needed.
 *
 * Return Value:
 *   An exception indicator.
 */
AXP_EXCEPTIONS
AXP_JMP(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
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
     * We store the PC calculated above into the destination register value.
     */
    instr->destv.r.uq = AXP_GET_PC(pc);

    /*
     * Use the hint bits to determine what we are being asked to do.
     */
    switch (AXP_JMP_TYPE(instr->displacement))
    {
        case AXP_HW_JMP:
            instr->branchPC = AXP_21264_DisplaceVPC(cpu,
                                                    pc,
                                                    instr->displacement);
            break;

        case AXP_HW_JSR:
            AXP_PUSH(pc);
            instr->branchPC = AXP_21264_DisplaceVPC(cpu,
                                                    pc,
                                                    instr->displacement);
            break;

        case AXP_HW_RET:
            AXP_POP(pc);
            instr->branchPC = AXP_21264_MakeVPC(cpu,
                                                AXP_GET_PC(pc),
                                                AXP_NORMAL_MODE);
            break;

        case AXP_HW_COROUTINE:
            AXP_SWAP(pc);
            instr->branchPC = AXP_21264_MakeVPC(cpu,
                                                AXP_GET_PC(pc),
                                                AXP_NORMAL_MODE);
            break;
    }

    /*
     * Now we use the PC indicated in src1 as the new PC.
     */
    instr->branchPC = AXP_21264_MakeVPC(cpu,
                                        instr->src1v.r.uq,
                                        AXP_NORMAL_MODE);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}
