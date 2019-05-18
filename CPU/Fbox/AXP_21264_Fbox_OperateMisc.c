/*
 * Copyright (C) Jonathan D. Belanger 2017-2019.
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
 *  functionality of the Fbox Operate Instructions.
 *
 * Revision History:
 *
 *  V01.000 24-Jun-2017 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 07-Jul-2017 Jonathan D. Belanger
 *  Renamed and left just the non-IEEE and non-VAX instructions implemented
 *  within.
 *
 *  V01.002 18-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CPU/Fbox/AXP_21264_Fbox_OperateMisc.h"

/*
 * AXP_CPYS
 *   This function implements the Floating-Point Operate Copy Sign instruction of
 *   the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_CPYS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction.
     */
    instr->destv.fp.fpr = instr->src2v.fp.fpr;
    instr->destv.fp.fpr.sign = instr->src1v.fp.fpr.sign;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_CPYSE
 *   This function implements the Floating-Point Operate Copy Sign and Exponent
 *   instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
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
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_CPYSN
 *   This function implements the Floating-Point Operate Copy Sign Negate
 *   instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_CPYSN(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction.
     */
    instr->destv.fp.fpr = instr->src2v.fp.fpr;
    instr->destv.fp.fpr.sign = ~instr->src1v.fp.fpr.sign;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_CVTLQ
 *   This function implements the Floating-Point Operate Convert Longword to
 *   Quadword instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
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
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_CVTQL
 *   This function implements the Floating-Point Operate Convert Quadword to
 *   Longword with overflow (for /V) instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 *  ArithmeticTraps:    An arithmetic trap has occurred:
 *                          Integer Overflow.
 */
AXP_EXCEPTIONS AXP_CVTQL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_FP_FUNC *func = (AXP_FP_FUNC *) &instr->function;
    AXP_EXCEPTIONS retVal = NoException;

    /*
     * Implement the instruction.
     */
    instr->destv.fp.l.sign = instr->src1v.fp.qVCvt.sign;
    instr->destv.fp.l.integerHigh = instr->src1v.fp.qVCvt.integerLowHigh;
    instr->destv.fp.l.zero_2 = 0;
    instr->destv.fp.l.integerLow = instr->src1v.fp.qVCvt.integerLowLow;
    instr->destv.fp.l.zero_1 = 0;

    if (func->trp == AXP_FP_TRP_V)
    {
        if (AXP_R_Q2L_OVERFLOW(instr->src1v.fp.uq))
        {
            int raised = FE_INEXACT | FE_OVERFLOW;

            retVal = ArithmeticTraps;
            AXP_FP_SetFPCR(cpu, instr, raised, true);
        }
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_FCMOVEQ
 *  This function implements the Floating-Point Conditional Move if Equal
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVEQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if ((instr->src1v.fp.fpr.exponent == 0) &&
        (instr->src1v.fp.fpr.fraction == 0))
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FCMOVGE
 *  This function implements the Floating-Point Conditional Move if Greater
 *  Than or Equal instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVGE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if (instr->src1v.fp.uq <= AXP_R_SIGN)
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FCMOVGT
 *  This function implements the Floating-Point Conditional Move if Greater
 *  Than instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVGT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if ((instr->src1v.fp.fpr.sign == 0) && (instr->src1v.fp.uq != 0))
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FCMOVLE
 *  This function implements the Floating-Point Conditional Move if Less Than
 *  or Equal instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVLE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq == 0))
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FCMOVLT
 *  This function implements the Floating-Point Conditional Move if Less Than
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVLT(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if ((instr->src1v.fp.fpr.sign == 1) || (instr->src1v.fp.uq != 0))
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FCMOVNE
 *  This function implements the Floating-Point Conditional Move if Not Equal
 *  instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FCMOVNE(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     */
    if ((instr->src1v.fp.uq & ~AXP_R_SIGN) != 0)
    {
        instr->destv.fp.uq = instr->src2v.fp.uq;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_MF_FPCR
 *  This function implements the Floating-Point Move From Floating-Point
 *  Control Register instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MF_FPCR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    union
    {
        u64 tmp1;
        AXP_FBOX_FPCR tmp2;
    } src = { .tmp2 = cpu->fpcr };

    /*
     * Implement the instruction
     */
    instr->destv.fp.uq = src.tmp1;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_MT_FPCR
 *  This function implements the Floating-Point Move To Floating-Point
 *  Control Register instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the structure containing the information needed to emulate
 *      a single CPU.
 *  instr:
 *      A pointer to a structure containing the information needed to execute
 *      this instruction.
 *
 * Output Parameters:
 *  instr:
 *      The contents of this structure are updated, as needed.
 *
 * Return Value:
 *  NoException:        Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MT_FPCR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction
     *
     * NOTE: The value will actually be moved into the FPCR at the time of
     *        instruction retirement.
     */
    instr->destv.fp.uq = instr->src1v.fp.uq;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}
