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
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		20-Jul-2017	Jonathan D. Belanger
 *	Implemented and coded the Miscellaneous Alpha AXP instructions.  Some
 *	instructions are just placeholders until the Data Cache functionality
 *	has been implemented.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox_Misc.h"

/*
 * DESIGN CONSIDERATIONS
 *
 *	In order to support the various Barrier instructions, they are implemented
 *	as a real instruction but does not do anything to registers or other items.
 *	They are used to indicate to which point in the instruction queue the Ebox
 *	and Fbox should search until the barrier instruction has been completed.
 *	In all cases a barrier instruction will be cloned and placed in both the
 *	IQ and FQ, indicating the both queues should not process records past the
 *	barrier or any other instructions in its queue until both instructions have
 *	completed.  The barrier instructions are:
 *
 *		Instruction		Name						Description
 *		-----------		--------------------		---------------------------
 *		EXCB			Exception Barrier			All Arithmetic Exceptions
 *		MB				Memory Barrier				Both Memory Writes & Stores
 *		TRAPB			Trap Barrier				All Arithmetic Traps
 *		WMB				Write Memory Barrier		Memory Writes only
 *
 *	NOTE: 	There is only one implementation of these instrctions (not one for
 *			Ebox and one for the Fbox.  Both will call the same instructions
 *			defined here.
 */

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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_AMASK(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    u64 Rbv = (instr->useLiteral ? instr->literal : instr->src2v.r.uq);
    u64 *aMask = (u64 *) &cpu->amask;

    /*
     * Return the masked off CPU Features.
     */
    if (cpu->majorType < EV56)
	instr->destv.r.uq = Rbv;
    else
	instr->destv.r.uq = Rbv & ~*aMask;

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_CALL_PAL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_PC *retPC = (AXP_PC *) &instr->destv.r.uq;

    /*
     * The destination register was set to the R23 (R39) shadow register or
     * R27 (does not have a shadow register).
     */
    *retPC = AXP_21264_GetNextVPC(cpu);
    AXP_PUSH(*retPC);

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
    return (NoException);
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_IMPLVER(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Return the Implementation Version value.
     */
    instr->destv.r.uq = cpu->implVer;

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_ECB
 *	This function implements the Evict Data Cache Block instruction of the
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_ECB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_DcacheEvict(cpu, instr->src1v.r.uq, instr->pc);

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_EXCB
 *	This function implements the Exception Barrier instruction of the Alpha AXP
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_EXCB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * There is nothing we have to do except get retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FETCH
 *	This function implements the Prefetch Data instruction of the Alpha AXP
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FETCH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * TODO:	We need to implement the prefetch data instruction
     *			once the data cache code has been implemented.
     *			Not sure if this needs to be handled by the Mbox or
     *			Cbox.
     * NOTE:	This function fetches an aligned 512-byte block.  Much larger
     *			than the usual 64 byte block fetched as the result of the Load
     *			and Store instructions.  We need to think this through.  For
     *			now, this is implemented as a NO-OP.
     */

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_FETCH_M
 *	This function implements the Prefetch Data with Modify Intent instruction
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_FETCH_M(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * TODO:	We need to implement the prefetch data instruction
     *			once the data cache code has been implemented.
     *			Not sure if this needs to be handled by the Mbox or
     *			Cbox.
     * NOTE:	This function fetches an aligned 512-byte block.  Much larger
     *			than the usual 64 byte block fetched as the result of the Load
     *			and Store instructions.  We need to think this through.  For
     *			now, this is implemented as a NO-OP.
     */

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_MB
 *	This function implements the Memory Barrier instruction of the Alpha AXP
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_MB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * TODO:	The Cbox needs to be told that an MB is pending.  The Cbox will
     *			perform some processing to make sure that instruction execution
     *			can begin with the instruction after the MB.  It does this by
     *			indicating to the Ibox that the MB instruction can be retired.
     *			Since the MB can be executed speculatively, then it is possible
     *			to have to abort it, in the case of an exception or
     *			mispredicted branch.
     */

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * The PREFETCHx instructions are implemented in the equivalent LDx
 * instructions.  The PREFETCHx instructions just have R31/F31 specified on
 * the instruction as the destination register.
 */

/*
 * AXP_PRCC
 *	This function implements the Read Processor Cycle Counter instruction of
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_RPCC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * The definition of this instruction has the following requirement:
     *
     *	RPCC does not read the Processor Cycle Counter (PCC) any earlier than
     *	the generation of a result by the nearest preceding instruction that
     *	modifies register Rb. If R31 is used as the Rb operand, the PCC need
     *	not wait for any preceding computation.
     *
     * In this emulator, because we support register renaming, the RPCC
     * instruction will not be executed until the instruction modifying the
     * register indicated in Rb, then, by definition, the nearest preceding
     * instruction that modifies this register has already been retired.  We
     * should be good to go.
     */

    /*
     * Return the Cycle Counter Counter and Offset values as 1 64-bit value.
     */
    instr->destv.r.uq = *((u64 *) &cpu->cc);

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_TRAPB
 *	This function implements the Trap Barrier instruction of the Alpha AXP
 *	processor.  This instruction is specific to Arithmetic Traps (Integer and
 *	Floating Point).  The EXCB instruction handles all Exceptions and is thus
 *	a superset of TRAPB.
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_TRAPB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * There is nothing we have to do except get retired.
     *
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_WH64
 *	This function implements the Write Hint - 64 Bytes instruction of the
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_WH64(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * TODO:	This instruction is used to provide a "hint" that the address
     *			specified in Rbv will not be read again, but will be
     *			overwritten shortly.  A cache resource/location may be
     *			allocated, but the contents of the memory location where this
     *			cached address resides, may not be read.  Any error that occurs
     *			(access violation, translation not valid, and so forth) will
     *			cause this instruction to behave like a NO-OP.
     */

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_WH64EN
 *	This function implements the Write Hint - 64 Bytes and Evict Next
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_WH64EN(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;

    /*
     * CPU implementations prior EV6x implement this instruction as a NOOP.
     * CPU implementations equal to EV6x implement this instruction as a WH64.
     * CPU implementations after EV6x fully implement this instruction.
     */
    if ((cpu->majorType == EV6) || /* 21264 */
    (cpu->majorType == EV67) || /* 21264 */
    (cpu->majorType == EV68A) || /* 21264 */
    (cpu->majorType == EV68CX) || /* 21264 */
    (cpu->majorType == EV69A)) /* 21264 */
    {
	retVal = AXP_WH64(cpu, instr);
    }
    else if ((cpu->majorType == EV7) || /* 21364 */
    (cpu->majorType == EV79)) /* 21364 */
    {

	/*
	 * TODO:	This instruction is used to provide a "hint" that the
	 * 			address specified in Rbv will not be read again, but will
	 * 			be overwritten shortly.  A cache resource/location may be
	 *			allocated, but the contents of the memory location where
	 *			this cached address resides, may not be read.  Any error
	 *			that occurs (access violation, translation not valid, and
	 *			so forth) will cause this instruction to behave like a
	 *			NOOP.
	 *
	 *			The difference between this instruction and WH64 is that
	 *			this one indicates that eviction policy for the indicated
	 *			64 byte location is different than the other.
	 */

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;
    }

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_WMB
 *	This function implements the Write Memory Barrier instruction of the Alpha
 *	AXP processor.
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
 * 	NoException		Normal successful completion.
 */
AXP_EXCEPTIONS AXP_WMB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * TODO:	Unlike the MB, the WMB needs to be sent to the Mbox.  The Mbox
     *			will hold onto the WMB until all prior store instructions
     *			become writable.  When this happens, the Mbox will indicate
     *			that the WMB can be retired.
     */

    /*
     * Indicate that the instruction is ready to be retired.
     */
    instr->state = WaitingRetirement;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}
