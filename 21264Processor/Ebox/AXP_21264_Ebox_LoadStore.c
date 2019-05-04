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
 *	Load and Store functionality of the Ebox.
 *
 * Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		25-Jun-2017	Jonathan D. Belanger
 *	I changed the way I used registers and retrieved memory.  I was doing it
 *	with bits and masks and should have been using structures, letting the
 *	compiler deal with the details.
 *
 *	V01.002		28-Jun-2017	Jonathan D. Belanger
 *	Added code to call the Mbox function to load and store data values into
 *	memory (Dcache).
 *
 *	V01.003		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
 */

#include "CommonUtilities/AXP_Configure.h"
#include "21264Processor/Ebox/AXP_21264_Ebox_LoadStore.h"

/*
 * IMPLEMENTATION NOTES:
 *
 * 		1)	If R31 is a destination register, then the code that selects the
 * 			instruction for execution from the IQ, will determine this and just
 * 			move the instruction state to WaitingRetirement.  The exception to
 * 			this are the LDL and LDQ instructions, where these instructions
 * 			become PREFETCH and PREFETCH_EN, respectively.
 * 		2)	When these functions are called, the instruction state is set to
 * 			Executing prior to the call.
 * 		3)	Registers have a layout.  Once a value is loaded into a register,
 * 			the 64-bit value, signed or unsigned, is utilized.  When a value
 * 			needs to be written from a register, then the proper size is
 * 			selected (always unsigned byte, word, longword, or quadword).
 */

/*
 * AXP_LDA
 *	This function implements the Load Address instruction of the Alpha AXP
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
AXP_EXCEPTIONS AXP_LDA(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction.
     */
    instr->destv.r.uq = instr->src1v.r.uq + instr->displacement;

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
 * AXP_LDAH
 *	This function implements the Load Address High instruction of the Alpha AXP
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
AXP_EXCEPTIONS AXP_LDAH(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Implement the instruction.
     */
    instr->destv.r.uq = instr->src1v.r.uq
  + (instr->displacement * AXP_LDAH_MULT);

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
 * AXP_LDBU
 *	This function implements the Load Zero-Extend Byte from Memory to Register
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
AXP_EXCEPTIONS AXP_LDBU(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_BYTE(va);

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDWU
 *	This function implements the Load Zero-Extend Word from Memory to Register
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
AXP_EXCEPTIONS AXP_LDWU(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_WORD(va);
    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDL
 *	This function implements the Load/Prefetch Sign-Extend Longword from Memory
 *	to Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is R31, then this instruction becomes the
 *	PREFETCH instruction.
 *
 *	A prefetch is a hint to the processor that a cache block might be used in
 *	the future and should be brought into the cache now.
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
AXP_EXCEPTIONS AXP_LDL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_LONG(va);

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDQ
 *	This function implements the Load/Prefetch Quadword from Memory to
 *	Register/no-where instruction of the Alpha AXP processor.
 *
 *	If the destination register is R31, then this instruction becomes the
 *	PREFETCH_EN instruction.
 *
 *	A prefetch, evict next, is a hint to the processor that a cache block
 *	should be brought into the cache now and marked for preferential eviction
 *	on future cache fills.  Such a prefetch is particularly useful with an
 *	associative cache, to prefetch data that is not repeatedly referenced --
 *	data that has a short temporal lifetime in the cache.
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
AXP_EXCEPTIONS AXP_LDQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = instr->src1v.r.uq + instr->displacement;

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDQ_U
 *	This function implements the Unaligned Load Quadword from Memory to
 *	Register/no-where instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_LDQ_U(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = (instr->src1v.r.uq + instr->displacement) & ~0x7;

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDL_L
 *	This function implements the Load Longword Memory Data into Integer
 *	Register Locked instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_LDL_L(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_LONG(va);

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_LDQ_L
 *	This function implements the Load Quadword Memory Data into Integer
 *	Register Locked instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_LDQ_L(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = instr->src1v.r.uq + instr->displacement;

    AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STL_C
 *	This function implements the Store Longword Integer Register into Memory
 *	Conditional instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_STL_C(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_LONG(va);

    AXP_21264_Mbox_WriteMem(
  cpu,
  instr,
  instr->slot,
  vaPrime,
  instr->src1v.r.ul);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STQ_C
 *	This function implements the Store Quadword Integer Register into Memory
 *	Conditional instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_STQ_C(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = instr->src1v.r.uq + instr->displacement;

    AXP_21264_Mbox_WriteMem(cpu, instr, instr->slot, va, instr->src1v.r.uq);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STB
 *	This function implements the Store Byte Integer Register into Memory
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
AXP_EXCEPTIONS AXP_STB(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_BYTE(va);

    AXP_21264_Mbox_WriteMem(
  cpu,
  instr,
  instr->slot,
  vaPrime,
  instr->src1v.r.ub);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STW
 *	This function implements the Store Word Integer Register into Memory
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
AXP_EXCEPTIONS AXP_STW(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_WORD(va);

    /*
     * Queue up the request to store this value into memory (Dcache).
     */
    AXP_21264_Mbox_WriteMem(
  cpu,
  instr,
  instr->slot,
  vaPrime,
  instr->src1v.r.uw);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STL
 *	This function implements the Store Longword Integer Register into Memory
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
AXP_EXCEPTIONS AXP_STL(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va, vaPrime;

    /*
     * Implement the instruction.
     */
    vaPrime = va = instr->src1v.r.uq + instr->displacement;

    /*
     * If we are executing in big-endian mode, then we need to do some address
     * adjustment.
     */
    if (cpu->vaCtl.b_endian == 1)
  vaPrime = AXP_BIG_ENDIAN_LONG(va);
    AXP_21264_Mbox_WriteMem(
  cpu,
  instr,
  instr->slot,
  vaPrime,
  instr->src1v.r.ul);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STQ
 *	This function implements the Store Quadword Integer Register into Memory
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
AXP_EXCEPTIONS AXP_STQ(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = instr->src1v.r.uq + instr->displacement;

    AXP_21264_Mbox_WriteMem(cpu, instr, instr->slot, va, instr->src1v.r.uq);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}

/*
 * AXP_STQ_U
 *	This function implements the Store Unaligned Quadword Integer Register into
 *	Memory instruction of the Alpha AXP processor.
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
AXP_EXCEPTIONS AXP_STQ_U(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
    AXP_EXCEPTIONS retVal = NoException;
    u64 va;

    /*
     * Implement the instruction.
     */
    va = (instr->src1v.r.uq + instr->displacement) & ~0x7;

    AXP_21264_Mbox_WriteMem(cpu, instr, instr->slot, va, instr->src1v.r.uq);

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (retVal);
}
