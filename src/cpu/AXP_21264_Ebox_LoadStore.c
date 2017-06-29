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
 *	Load and Store functionality of the Ebox.
 *
 *	Revision History:
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
 */

#include "AXP_Configure.h"
#include "AXP_21264_Ebox_LoadStore.h"

/*
 * Prototypes for Local Functions.
 */
void AXP_LDBU_COMPL(AXP_INSTRUCTION *);
void AXP_LDWU_COMPL(AXP_INSTRUCTION *);
void AXP_LDL_COMPL(AXP_INSTRUCTION *);

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
	return(NoException);
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
	instr->destv.r.uq = instr->src1v.r.uq + (instr->displacement * AXP_LDAH_MULT);

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

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime, sizeof(u8));
	instr->loadCompletion = AXP_LDBU_COMPL;

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
 * AXP_LDBU_COMPL
 *	This function completes the Load Zero-Extend Byte from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to complete
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	None.
 */
void AXP_LDBU_COMPL(AXP_INSTRUCTION *instr)
{

	/*
	 * The last thing we need to do is zero extend the destination register.
	 */
	instr->destv.r.uq = AXP_ZEXT_BYTE(instr->destv.r.uq);

	/*
	 * Return back to the caller.
	 */
	return;
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

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime, sizeof(u16));
	instr->loadCompletion = AXP_LDWU_COMPL;

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
 * AXP_LDWU_COMPL
 *	This function completes the Load Zero-Extend Word from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to complete
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	None.
 */
void AXP_LDWU_COMPL(AXP_INSTRUCTION *instr)
{

	/*
	 * The last thing we need to do is zero extend the destination register.
	 */
	instr->destv.r.uq = AXP_ZEXT_WORD(instr->destv.r.uq);

	/*
	 * Return back to the caller.
	 */
	return;
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

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime, sizeof(u32));
	instr->loadCompletion = AXP_LDL_COMPL;

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
 * AXP_LDL_COMPL
 *	This function completes the Load Sign-Extend Long from Memory to Register
 *	instruction of the Alpha AXP processor.
 *
 * Input Parameters:
 * 	instr:
 * 		A pointer to a structure containing the information needed to complete
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	None.
 */
void AXP_LDL_COMPL(AXP_INSTRUCTION *instr)
{

	/*
	 * The last thing we need to do is zero extend the destination register.
	 */
	instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);

	/*
	 * Return back to the caller.
	 */
	return;
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

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va, sizeof(u64));
	// No completion function required for quadword loads.

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

	// TODO: Need to make sure the completion code ZEXT the value.
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va, sizeof(u64));
	// No completion function required for quadword loads.

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
 * TODO:	The 21264 does not contain a dedicated lock register, nor are any
 * TODO:	system components required to do so.
 * TODO:
 * TODO:	When a load-lock instruction executed, data is accessed from the
 * TODO:	Dcache (or Bcache).  If there is a cache miss, data is access from
 * TODO:	memory with a RdBlk command.  Its associated cache line is filled
 * TODO:	into the Dcache in the clean state, if it's not already there.
 * TODO:
 * TODO:	When a store-conditional instruction executes, it is allowed to
 * TODO:	succeed if it's associated cache line is still present in the
 * TODO:	Dcache and can be made writable; otherwise it fails.
 * TODO:
 * TODO:	This algorithm is successful because another agent in the system
 * TODO:	writing to the cache line between the load-lock and
 * TODO:	store-conditional cache line would make the cache line invalid.
 * TODO:
 * TODO:	The following code does not take any of this into account and will
 * TODO:	need to be corrected.
 */

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

	instr->lockFlagPending = true;
	instr->lockPhysAddrPending = va;	// TODO: Need to convert to a physical address
	instr->lockVirtAddrPending = va;

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, vaPrime, sizeof(u32));
	instr->loadCompletion = AXP_LDL_COMPL;

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

	instr->lockFlagPending = true;
	instr->lockPhysAddrPending = va;	// TODO: Need to convert to a physical address
	instr->lockVirtAddrPending = va;

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a read fault (Fault on Read)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_ReadMem(cpu, instr, instr->slot, va, sizeof(u64));
	// No completion function required for quadword loads.

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

	if (cpu->lockFlag == true)
	{
		AXP_21264_Mbox_WriteMem(
			cpu,
			instr,
			instr->slot,
			vaPrime,
			instr->src1v.r.ul,
			sizeof(instr->src1v.r.ul));
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)
		instr->destv.r.uq = 1;
	}
	else
		instr->destv.r.uq = 0;
	instr->clearLockPending = true;

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

	if (cpu->lockFlag == true)
	{
		AXP_21264_Mbox_WriteMem(
			cpu,
			instr,
			instr->slot,
			va,
			instr->src1v.r.uq,
			sizeof(instr->src1v.r.uq));
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)
		instr->destv.r.uq = 1;
	}
	else
		instr->destv.r.uq = 0;
	instr->clearLockPending = true;

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
			instr->src1v.r.ub,
			sizeof(instr->src1v.r.ub));
		// TODO: Check to see if we had an access fault (Access Violation)
		// TODO: Check to see if we had an alignment fault (Alignment)
		// TODO: Check to see if we had a write fault (Fault on Write)
		// TODO: Check to see if we had a translation fault (Translation Not Valid)

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
		instr->src1v.r.uw,
		sizeof(instr->src1v.r.uw));
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a write fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

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
		instr->src1v.r.ul,
		sizeof(instr->src1v.r.ul));
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a write fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

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

	AXP_21264_Mbox_WriteMem(
		cpu,
		instr,
		instr->slot,
		va,
		instr->src1v.r.uq,
		sizeof(instr->src1v.r.uq));
	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had an alignment fault (Alignment)
	// TODO: Check to see if we had a write fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)

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

	// TODO: Check to see if we had an access fault (Access Violation)
	// TODO: Check to see if we had a write fault (Fault on Write)
	// TODO: Check to see if we had a translation fault (Translation Not Valid)
	AXP_21264_Mbox_WriteMem(
		cpu,
		instr,
		instr->slot,
		va,
		instr->src1v.r.uq,
		sizeof(instr->src1v.r.uq));

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller with any exception that may have occurred.
	 */
	return(retVal);
}
