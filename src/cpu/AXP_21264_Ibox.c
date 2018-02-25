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
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Cleaned up some compiler errors.  Ran some tests, and corrected some coding
 *	mistakes.  The prediction code correctly predicts a branch instruction
 *	between 95.0% and 99.1% of the time.
 *
 *	V01.002		20-May-2017	Jonathan D. Belanger
 *	Did a bit of reorganization.  This module will contain all the
 *	functionality that is described as being part of the Ibox.  I move the
 *	prediction code here.
 *
 *	V01.003		22-May-2017	Jonathan D. Belanger
 *	Moved the main() function out of this module and into its own.
 *
 *	V01.004		24-May-2017	Jonathan D. Belanger
 *	Added some instruction cache (icache) code and definitions into this code.
 *	The iCache structure should be part of the CPU structure and the iCache
 *	code should not be adding a cache entry on a miss.  There should be a
 *	separate fill cache function.  Also, the iCache look-up code should return
 *	the requested instructions on a hit.
 *
 *	V01.005		01-Jun-2017	Jonathan D. Belanger
 *	Added a function to add an Icache line/block.  There are a couple to do
 *	items for ASM and ASN in this function.
 *
 *	V01.006		11-Jun-2017	Jonathan D. Belanger
 *	Started working on the instruction decode and register renaming code.
 *
 *	V01.007		12-Jun-2017	Jonathan D. Belanger
 *	Forgot to include the generation of a unique ID (8-bits long) to associate
 *	with each decoded instruction.
 *	BTW: We need a better way to decode an instruction.  The decoded
 *	instruction does not require the architectural register number, so we
 *	should not save them.  Also, renaming can be done while we are decoding the
 *	instruction.  We are also going to have to special case some instructions,
 *	such as PAL instructions, CMOVE, and there are exceptions for instruction
 *	types (Ra is a destination register for a Load, but a source register for a
 *	store.
 *
 *	V01.008		15-Jun-2017	Jonathan D. Belanger
 *	Finished defining structure array to be able to normalize how a register is
 *	used in an instruction.  Generally speaking, register 'c' (Rc or Fc) is a
 *	destination register.  In load operations, register 'a' (Ra or Fa) is the
 *	destination register.  Register 'b' (Rb or Fb) is never used as a
 *	designation (through I have implemented provisions for this).
 *	Unfortunately, sometimes a register is not used, or 'b' may be source 1 and
 *	other times it is source 2.  Sometimes not register is utilized (e.g. MB).
 *	The array structures assist in these differences so that in the end we end
 *	up with a destination (not not), a source 1 (or not), and/or a source 2
 *	(or not) set of registers.  If we have a destination register, then
 *	register renaming needs to allocate a new register.  IF we have source
 *	register, then renaming needs to associate the current register mapping to
 *	this register.
 *
 *	V01.009		16-Jun-2017	Jonathan D. Belanger
 *	Finished the implementation of normalizing registers, as indicated above,
 *	and then mapping them from architectural to physical registers, generating
 *	a new mapping for the destination register.
 *
 *	V01.010		23-Jun-2017	Jonathan D. Belanger
 *	The way I'm handling the Program Counter (PC/VPC) is not correct.  When
 *	instructions are decoded, queued, and executed, their results are not
 *	realized until the instruction is retired.  The Integer Control
 *	instructions currently determine the next PC and set it immediately. not
 *	are retirement.  This is because I tried to do to much in the code that
 *	handles the PC/VPC.  The next PC should only be updated as a part of normal
 *	instruction queuing or instruction retirement.  We should only have one
 *	way to "set" the PC, but other functions to calculate one, based on various
 *	criteria.
 *
 *	V01.011		18-Jul-2017	Jonathan D. Belanger
 *	Updated the instruction decoding code to replace instruction indicated
 *	registers for PALshadow registers when we are in, or going to be in,
 *	PALmode.  Also, do not include floating point registers, as there are no
 *	PALshadow registers for these.
 *
 *	V01.012		15-Jan-2018	Jonathan D. Belanger
 *	Move the instruction decoding functions to their own module.  This was
 *	getting too large for eclipse to handle without crashing.
 */
#include "AXP_Configure.h"
#include "AXP_Dumps.h"
#include "AXP_Trace.h"
#include "AXP_21264_Ibox.h"
#include "AXP_21264_Ibox_Initialize.h"
#include "AXP_21264_Ibox_InstructionDecoding.h"
#include "AXP_21264_Ibox_PCHandling.h"
#include "AXP_21264_Mbox.h"

/*
 * A local structure used to calculate the PC for a CALL_PAL function.
 */
struct palBaseBits21264
{
	u64	res : 15;
	u64	highPC : 49;
};
struct palBaseBits21164
{
	u64	res : 14;
	u64	highPC : 50;
};

typedef union
{
	 struct palBaseBits21164	bits21164;
	 struct palBaseBits21264	bits21264;
	 u64						palBaseAddr;

}AXP_IBOX_PALBASE_BITS;

struct palPCBits21264
{
	u64	palMode : 1;
	u64 mbz_1 : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 mbz_2 : 1;
	u64 highPC : 49;
};
struct palPCBits21164
{
	u64	palMode : 1;
	u64 mbz : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 highPC : 50;
};

typedef union
{
	struct palPCBits21164	bits21164;
	struct palPCBits21264	bits21264;
	AXP_PC					vpc;
} AXP_IBOX_PAL_PC;

struct palFuncBits
{
	u32 func_5_0 : 6;
	u32 res_1 : 1;
	u32 func_7 : 1;
	u32 res_2 : 24;
};

typedef union
{
	struct palFuncBits	bits;
	u32					func;
} AXP_IBOX_PAL_FUNC_BITS;

/*
 * Converts instruction state to a string.
 */
static char *_ins_state_[] =
{
	"Retired",
	"Queued",
	"Executing",
	"Waiting to be Retired"
};

/*
 * Functions that manage the instruction queue free entries.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextIQEntry(AXP_21264_CPU *);
static AXP_QUEUE_ENTRY *AXP_GetNextFQEntry(AXP_21264_CPU *);

/*
 * AXP_GetNextIQEntry
 * 	This function is called to get the next available entry for the IQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A pointer to the next available pre-allocated queue entry for the IQ.
 *
 * NOTE:	This function assumes that there is always at least one free entry.
 * 			Since the number of entries pre-allocated is equal to the maximum
 * 			number of entries that can be in the IQ, this is not necessarily a
 * 			bad assumption.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextIQEntry(AXP_21264_CPU *cpu)
{
	AXP_QUEUE_ENTRY *retVal;

	retVal = &cpu->iqEntries[cpu->iqEFlStart];
	cpu->iqEFlStart = (cpu->iqEFlStart + 1) % AXP_IQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ReturnIQEntry
 * 	This function is called to return an entry back to the IQ queue for a
 * 	future instruction.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	entry:
 *		A pointer to the entry being returned to the free-list.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_ReturnIQEntry(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{

	/*
	 * Enter the index of the IQ entry onto the end of the free-list.
	 */
	cpu->iqEFreelist[cpu->iqEFlEnd] = entry->index;

	/*
	 * Increment the counter, in a round-robin fashion, for the entry just
	 * after end of the free-list.
	 */
	cpu->iqEFlEnd = (cpu->iqEFlEnd + 1) % AXP_IQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_GetNextFQEntry
 * 	This function is called to get the next available entry for the FQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A pointer to the next available pre-allocated queue entry for the FQ.
 *
 * NOTE:	This function assumes that there is always at least one free entry.
 * 			Since the number of entries pre-allocated is equal to the maximum
 * 			number of entries that can be in the FQ, this is not necessarily a
 * 			bad assumption.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextFQEntry(AXP_21264_CPU *cpu)
{
	AXP_QUEUE_ENTRY *retVal;

	retVal = &cpu->fqEntries[cpu->fqEFlStart];
	cpu->fqEFlStart = (cpu->fqEFlStart + 1) % AXP_FQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ReturnFQEntry
 * 	This function is called to return an entry back to the FQ queue for a
 * 	future instruction.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	entry:
 *		A pointer to the entry being returned to the free-list.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_ReturnFQEntry(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{

	/*
	 * Enter the index of the IQ entry onto the end of the free-list.
	 */
	cpu->fqEFreelist[cpu->fqEFlEnd] = entry->index;

	/*
	 * Increment the counter, in a round-robin fashion, for the entry just
	 * after end of the free-list.
	 */
	cpu->fqEFlEnd = (cpu->fqEFlEnd + 1) % AXP_FQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_Event
 *	This function is called from a number of places.  It receives information
 *	about an event (interrupt) that just occurred.  We need to queue this event
 *	up for the Ibox to process.  The callers to this function include not only
 *	the Ibox, itself, but also the Mbox.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *	fault:
 *		A value indicating the fault that occurred.
 *	pc:
 *		A value indicating the PC for the instruction being executed.
 *	va:
 *		A value indicating the Virtual Address where the fault occurred.
 *	opcode:
 *		A value for the opcode for with the instruction associated with the
 *		fault
 *	reg:
 *		A value indicating the architectural register associated with the
 *		fault.
 *	write:
 *		A boolean to indicate if it was a write operation associated with the
 *		fault.
 *	self:
 *		A boolean too  indicate that the Ibox is actually calling this
 *		function.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 * 	None.
 */
void AXP_21264_Ibox_Event(
			AXP_21264_CPU *cpu,
			u32 fault,
			AXP_PC pc,
			u64 va,
			u8 opcode,
			u8 reg,
			bool write,
			bool self)
{
	u8	mmStatOpcode = opcode;

	/*
	 * We, the Ibox, did not call this function, then we need to lock down the
	 * the Ibox mutex.
	 */
	if (self == false)
		pthread_mutex_lock(&cpu->iBoxMutex);

	/*
	 * If there is already an exception pending, swallow this current one.
	 */
	if (cpu->excPend == false)
	{

		/*
		 * We always need to lock down the IPR mutex.
		 */
		pthread_mutex_lock(&cpu->iBoxIPRMutex);

		/*
		 * HW_LD (0x1b = 27 -> 3) and HW_ST (0x1f = 31 -> 7), subtract 0x18(24)
		 * from both.
		 */
		if ((opcode == HW_LD) || (opcode == HW_ST))
			mmStatOpcode -= 0x18;
		cpu->excAddr.exc_pc = pc;

		/*
		 * Clear out the fault IPRs.
		 */
		cpu->va = 0;
		*((u64 *) &cpu->excSum) = 0;
		*((u64 *) &cpu->mmStat) = 0;

		/*
		 * Based on the fault, set the appropriate IPRs.
		 */
		switch (fault)
		{
			case AXP_DTBM_DOUBLE_3:
			case AXP_DTBM_DOUBLE_4:
			case AXP_ITB_MISS:
			case AXP_DTBM_SINGLE:
				cpu->mmStat.opcodes = mmStatOpcode;
				cpu->mmStat.wr = (write ? 1 : 0);
				cpu->va = va;
				cpu->excSum.reg = reg;
				break;

			case AXP_DFAULT:
			case AXP_UNALIGNED:
				cpu->excSum.reg = reg;
				cpu->mmStat.opcodes = mmStatOpcode;
				cpu->mmStat.wr = (write ? 1 : 0);
				cpu->mmStat.fow = (write ? 1 : 0);
				cpu->mmStat._for = (write ? 0 : 1);
				cpu->mmStat.acv = 1;
				cpu->va = va;
				break;

			case AXP_IACV:
				cpu->excSum.bad_iva = 0;	/* VA contains the address */
				cpu->va = va;
				break;

			case AXP_ARITH:
			case AXP_FEN:
			case AXP_MT_FPCR_TRAP:
				cpu->excSum.reg = reg;
				break;

			case AXP_OPCDEC:
				cpu->mmStat.opcodes = mmStatOpcode;
				break;

			case AXP_INTERRUPT:
				cpu->iSum.ei = cpu->irqH;
				cpu->irqH = 0;
				break;

			case AXP_MCHK:
			case AXP_RESET_WAKEUP:
				break;
		}

		/*
		 * Sign-extend the set_iov bit.
		 */
		if (cpu->excSum.set_iov == 1)
			cpu->excSum.sext_set_iov = 0xffff;

		/*
		 * Set the exception PC, which the main line will pick up when
		 * processing the exception.
		 */
		cpu->excPC = AXP_21264_GetPALFuncVPC(cpu, fault);

		/*
		 * Make sure to unlock the IPR mutex.
		 */
		pthread_mutex_unlock(&cpu->iBoxIPRMutex);

		/*
		 * Let the main loop know that there is an exception pending.
		 */
		cpu->excPend = true;

		/*
		 * We, the Ibox, did not call this function, then we need signal the
		 * Ibox to process this fault.
		 */
		if (self == false)
			pthread_cond_signal(&cpu->iBoxCondition);
	}

	/*
	 * Now unlock the Ibox mutex.
	 */
	if (self == false)
		pthread_mutex_unlock(&cpu->iBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_UpdateIcache
 *	This function is called by the Cbox to update a particular block within the
 *	Icache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	pa:
 *		A value representing the physical address associated with the block of
 *		instructions.
 *	data:
 *		A pointer to a buffer containing data returned from a Load/Store from
 *		physical memory.
 *	status:
 *		A value indicating the bits to be set in the ITAG (dirty and shared).
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Ibox_UpdateIcache(
						AXP_21264_CPU *cpu,
						u64 pa,
						u8 *data,
						u8 status)
{

	/*
	 * First things first, we have to lock the Mbox mutex.
	 */
	pthread_mutex_lock(&cpu->iBoxMutex);

#pragma message "This function, AXP_21264_Ibox_UpdateIcache, is not even close to being finished."

	/*
	 * Write the data to the Dcache block.
	 */
	AXP_IcacheAdd(cpu, *(AXP_PC *) &pa, (u32 *) data, NULL);

	/*
	 * Let the Ibox know that there are more instructions to process.
	 */
	pthread_cond_signal(&cpu->iBoxCondition);

	/*
	 * Last things last, we have to unlock the Mbox mutex.
	 */
	pthread_mutex_unlock(&cpu->mBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_Retire_HW_MFPR
 *	This function is called to move a value from a processor register to an
 *	architectural register.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *	instr:
 *		A pointer to the Instruction being retired.  We already know it is a
 *		HW_MFPR instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Ibox_Retire_HW_MFPR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * Before we do anything, we need to lock the appropriate IPR mutex.
	 */
	if ((instr->type_hint_index <= AXP_IPR_SLEEP) ||
		((instr->type_hint_index >= AXP_IPR_PCXT0) &&
		 (instr->type_hint_index <= AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER_ASN)))
	{
		 pthread_mutex_lock(&cpu->iBoxIPRMutex);
			switch (instr->type_hint_index)
			{
				case AXP_IPR_EXC_ADDR:
					AXP_IBOX_READ_EXC_ADDR(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_IVA_FORM:
					AXP_IBOX_READ_IVA_FORM(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_CM:
					AXP_IBOX_READ_CM(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_IER:
					AXP_IBOX_READ_IER(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_IER_CM:
					AXP_IBOX_READ_IER_CM(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_SIRR:
					AXP_IBOX_READ_SIRR(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_ISUM:
					AXP_IBOX_READ_ISUM(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_EXC_SUM:
					AXP_IBOX_READ_EXC_SUM(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_PAL_BASE:
					AXP_IBOX_READ_PAL_BASE(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_I_CTL:
					AXP_IBOX_READ_I_CTL(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_PCTR_CTL:
					AXP_IBOX_READ_PCTR_CTL(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_I_STAT:
					AXP_IBOX_READ_I_STAT(instr->destv.r.uq, cpu);
					break;

				case AXP_IPR_PCXT0:
				case AXP_IPR_PCXT0_ASN:
				case AXP_IPR_PCXT0_ASTER:
				case AXP_IPR_PCXT0_ASTER_ASN:
				case AXP_IPR_PCXT0_ASTRR:
				case AXP_IPR_PCXT0_ASTRR_ASN:
				case AXP_IPR_PCXT0_ASTRR_ASTER:
				case AXP_IPR_PCXT0_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT0_PPCE:
				case AXP_IPR_PCXT0_PPCE_ASN:
				case AXP_IPR_PCXT0_PPCE_ASTER:
				case AXP_IPR_PCXT0_PPCE_ASTER_ASN:
				case AXP_IPR_PCXT0_PPCE_ASTRR:
				case AXP_IPR_PCXT0_PPCE_ASTRR_ASN:
				case AXP_IPR_PCXT0_PPCE_ASTRR_ASTER:
				case AXP_IPR_PCXT0_PPCE_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT0_FPE:
				case AXP_IPR_PCXT0_FPE_ASN:
				case AXP_IPR_PCXT0_FPE_ASTER:
				case AXP_IPR_PCXT0_FPE_ASTER_ASN:
				case AXP_IPR_PCXT0_FPE_ASTRR:
				case AXP_IPR_PCXT0_FPE_ASTRR_ASN:
				case AXP_IPR_PCXT0_FPE_ASTRR_ASTER:
				case AXP_IPR_PCXT0_FPE_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT0_FPE_PPCE:
				case AXP_IPR_PCXT0_FPE_PPCE_ASN:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTER:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTER_ASN:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTRR:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASN:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER:
				case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT1:
				case AXP_IPR_PCXT1_ASN:
				case AXP_IPR_PCXT1_ASTER:
				case AXP_IPR_PCXT1_ASTER_ASN:
				case AXP_IPR_PCXT1_ASTRR:
				case AXP_IPR_PCXT1_ASTRR_ASN:
				case AXP_IPR_PCXT1_ASTRR_ASTER:
				case AXP_IPR_PCXT1_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT1_PPCE:
				case AXP_IPR_PCXT1_PPCE_ASN:
				case AXP_IPR_PCXT1_PPCE_ASTER:
				case AXP_IPR_PCXT1_PPCE_ASTER_ASN:
				case AXP_IPR_PCXT1_PPCE_ASTRR:
				case AXP_IPR_PCXT1_PPCE_ASTRR_ASN:
				case AXP_IPR_PCXT1_PPCE_ASTRR_ASTER:
				case AXP_IPR_PCXT1_PPCE_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT1_FPE:
				case AXP_IPR_PCXT1_FPE_ASN:
				case AXP_IPR_PCXT1_FPE_ASTER:
				case AXP_IPR_PCXT1_FPE_ASTER_ASN:
				case AXP_IPR_PCXT1_FPE_ASTRR:
				case AXP_IPR_PCXT1_FPE_ASTRR_ASN:
				case AXP_IPR_PCXT1_FPE_ASTRR_ASTER:
				case AXP_IPR_PCXT1_FPE_ASTRR_ASTER_ASN:
				case AXP_IPR_PCXT1_FPE_PPCE:
				case AXP_IPR_PCXT1_FPE_PPCE_ASN:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTER:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTER_ASN:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTRR:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASN:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER:
				case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER_ASN:
					AXP_IBOX_READ_PCTX(instr->destv.r.uq, cpu);
					break;

				default:
					break;
			}
		 pthread_mutex_unlock(&cpu->iBoxIPRMutex);
	}
	else if (((instr->type_hint_index >= AXP_IPR_DTB_TAG0) &&
			  (instr->type_hint_index <= AXP_IPR_DC_STAT)) ||
			 ((instr->type_hint_index >= AXP_IPR_DTB_TAG1) &&
			  (instr->type_hint_index <= AXP_IPR_DTB_ASN1)))
	{
		pthread_mutex_lock(&cpu->mBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_MM_STAT:
				AXP_MBOX_READ_MM_STAT(instr->destv.r.uq, cpu);
				break;

			case AXP_IPR_DC_STAT:
				AXP_MBOX_READ_DC_STAT(instr->destv.r.uq, cpu);
				break;

			default:
				break;
		}
		pthread_mutex_unlock(&cpu->mBoxIPRMutex);
	}
	else if ((instr->type_hint_index >= AXP_IPR_CC) &&
			 (instr->type_hint_index <= AXP_IPR_VA_CTL))
	{
		pthread_mutex_lock(&cpu->eBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_CC:
				AXP_EBOX_READ_CC(instr->destv.r.uq, cpu);
				break;

			case AXP_IPR_VA:
				AXP_EBOX_READ_VA(instr->destv.r.uq, cpu);
				break;

			case AXP_IPR_VA_FORM:
				AXP_EBOX_READ_VA_FORM(instr->destv.r.uq, cpu);
				break;

			default:
				break;
		}
		pthread_mutex_unlock(&cpu->eBoxIPRMutex);
	}
	else
	{
		pthread_mutex_lock(&cpu->cBoxIPRMutex);
		if (instr->type_hint_index == AXP_IPR_C_DATA)
		{
			AXP_CBOX_READ_C_DATA(instr->destv.r.uq, cpu);
		}
		pthread_mutex_unlock(&cpu->cBoxIPRMutex);
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_Retire_HW_MTPR
 *	This function is called to move a value from an architectural register to a
 *	processor register.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *	instr:
 *		A pointer to the Instruction being retired.  We already know it is a
 *		HW_MTPR instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Ibox_Retire_HW_MTPR(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	AXP_IBOX_PCTX	*pctx = (AXP_IBOX_PCTX *) &cpu->pCtx;
	u32				ccCtlCounter;

	/*
	 * Determine which IPR is being updated and lock the appropriate mutex, set
	 * the new value, and unlock the mutex.
	 *
	 * Ibox IPR
	 */
	if ((instr->type_hint_index <= AXP_IPR_SLEEP) ||
		((instr->type_hint_index >= AXP_IPR_PCXT0) &&
		 (instr->type_hint_index <= AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER_ASN)))
	{
		pthread_mutex_lock(&cpu->iBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_ITB_TAG:
				AXP_IBOX_WRITE_ITB_TAG(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_ITB_PTE:
				AXP_IBOX_WRITE_ITB_PTE(instr->src1v.r.uq, cpu);

				/*
				 * Retiring this instruction causes the TAG and PTE to be
				 * written into the ITB entry.
				 */
				AXP_addTLBEntry(
						cpu,
						*((u64 *) &cpu->itbTag),
						*((u64 *) &cpu->itbPte),
						false);
				break;

			case AXP_IPR_ITB_IAP:

				/*
				 * This is a Pseudo register.  Writing to it clears all the ITB
				 * PTE entries with an ASM bit clear.
				 */
				AXP_tbiap(cpu, false);
				break;

			case AXP_IPR_ITB_IA:

				/*
				 * This is a Pseudo register.  Writing to it clears all the ITB
				 * PTE entries.
				 */
				AXP_tbia(cpu, false);
				break;

			case AXP_IPR_ITB_IS:
				AXP_IBOX_WRITE_ITB_IS(instr->src1v.r.uq, cpu);

				/*
				 * Writing to it clears the ITB PTE entries that matches the
				 * ITB_IS IPR.
				 */
				AXP_tbis(cpu, *((u64 *) &cpu->itbIs), false);
				break;


			case AXP_IPR_CM:
				AXP_IBOX_WRITE_CM(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_IER:
				AXP_IBOX_WRITE_IER(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_IER_CM:
				AXP_IBOX_WRITE_IER_CM(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_SIRR:
				AXP_IBOX_WRITE_SIRR(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_HW_INT_CLR:
				AXP_IBOX_WRITE_HW_INT_CLR(instr->src1v.r.uq, cpu);
				if (cpu->hwIntClr.sl == 1)
					cpu->iSum.sl = 0;
				if (cpu->hwIntClr.cr == 1)
					cpu->iSum.cr = 0;
				if (cpu->hwIntClr.pc == 1)
					cpu->iSum.pc = cpu->iSum.pc & 0x2;
				if (cpu->hwIntClr.pc == 2)
					cpu->iSum.pc = cpu->iSum.pc & 0x1;
				break;

			case AXP_IPR_PAL_BASE:
				AXP_IBOX_WRITE_PAL_BASE(instr->src1v.r.uq, cpu);
				break;

			case AXP_IPR_I_CTL:
				AXP_IBOX_WRITE_I_CTL(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_IC_FLUSH_ASM:
				AXP_IcacheFlush(cpu, true);
				break;

			case AXP_IPR_IC_FLUSH:
				cpu->iCacheFlushPending = true;
				break;

			case AXP_IPR_PCTR_CTL:
				AXP_IBOX_WRITE_PCTR_CTL(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_CLR_MAP:
				AXP_21264_Ibox_ResetRegMap(cpu);
				break;

			case AXP_IPR_I_STAT:
				AXP_IBOX_WRITE_I_STAT(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_SLEEP:
				/* TODO: Pseudo register */
				break;

			case AXP_IPR_PCXT0:
			case AXP_IPR_PCXT1:
				/* No process context field selected */
				break;

			case AXP_IPR_PCXT0_ASN:
			case AXP_IPR_PCXT1_ASN:
				cpu->pCtx.asn = pctx->asn;
				break;

			case AXP_IPR_PCXT0_ASTER:
			case AXP_IPR_PCXT1_ASTER:
				cpu->pCtx.aster = pctx->aster;
				break;

			case AXP_IPR_PCXT0_ASTER_ASN:
			case AXP_IPR_PCXT1_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				break;

			case AXP_IPR_PCXT0_ASTRR:
			case AXP_IPR_PCXT1_ASTRR:
				cpu->pCtx.astrr = pctx->astrr;
				break;

			case AXP_IPR_PCXT0_ASTRR_ASN:
			case AXP_IPR_PCXT1_ASTRR_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.astrr = pctx->astrr;
				break;

			case AXP_IPR_PCXT0_ASTRR_ASTER:
			case AXP_IPR_PCXT1_ASTRR_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				break;

			case AXP_IPR_PCXT0_ASTRR_ASTER_ASN:
			case AXP_IPR_PCXT1_ASTRR_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				break;

			case AXP_IPR_PCXT0_PPCE:
			case AXP_IPR_PCXT1_PPCE:
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASN:
			case AXP_IPR_PCXT1_PPCE_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTER:
			case AXP_IPR_PCXT1_PPCE_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTER_ASN:
			case AXP_IPR_PCXT1_PPCE_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTRR:
			case AXP_IPR_PCXT1_PPCE_ASTRR:
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTRR_ASN:
			case AXP_IPR_PCXT1_PPCE_ASTRR_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTRR_ASTER:
			case AXP_IPR_PCXT1_PPCE_ASTRR_ASTER:
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_PPCE_ASTRR_ASTER_ASN:
			case AXP_IPR_PCXT1_PPCE_ASTRR_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				break;

			case AXP_IPR_PCXT0_FPE:
			case AXP_IPR_PCXT1_FPE:
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASN:
			case AXP_IPR_PCXT1_FPE_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTER:
			case AXP_IPR_PCXT1_FPE_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTER_ASN:
			case AXP_IPR_PCXT1_FPE_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTRR:
			case AXP_IPR_PCXT1_FPE_ASTRR:
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTRR_ASN:
			case AXP_IPR_PCXT1_FPE_ASTRR_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTRR_ASTER:
			case AXP_IPR_PCXT1_FPE_ASTRR_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_ASTRR_ASTER_ASN:
			case AXP_IPR_PCXT1_FPE_ASTRR_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE:
			case AXP_IPR_PCXT1_FPE_PPCE:
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASN:
			case AXP_IPR_PCXT1_FPE_PPCE_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTER:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTER_ASN:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTRR:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTRR:
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASN:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER:
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

			case AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER_ASN:
			case AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER_ASN:
				cpu->pCtx.asn = pctx->asn;
				cpu->pCtx.aster = pctx->aster;
				cpu->pCtx.astrr = pctx->astrr;
				cpu->pCtx.ppce = pctx->ppce;
				cpu->pCtx.fpe = pctx->fpe;
				break;

				default:
					break;
			}
		 pthread_mutex_unlock(&cpu->iBoxIPRMutex);
	}

	/*
	 * Mbox IPR
	 */
	else if (((instr->type_hint_index >= AXP_IPR_DTB_TAG0) &&
			  (instr->type_hint_index <= AXP_IPR_DC_STAT)) ||
			 ((instr->type_hint_index >= AXP_IPR_DTB_TAG1) &&
			  (instr->type_hint_index <= AXP_IPR_DTB_ASN1)))
	{
		pthread_mutex_lock(&cpu->mBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_DTB_TAG0:
				AXP_MBOX_WRITE_DTB_TAG0(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DTB_PTE0:
				AXP_MBOX_WRITE_DTB_PTE0(instr->src1v.r.uq,cpu);

				/*
				 * Retiring this instruction causes the TAG0 and PTE0 to be
				 * written into the DTB entry.
				 */
				AXP_addTLBEntry(
						cpu,
						*((u64 *) &cpu->dtbTag0),
						*((u64 *) &cpu->dtbPte0),
						true);
				break;

			case AXP_IPR_DTB_IS0:
				AXP_MBOX_WRITE_DTB_IS0(instr->src1v.r.uq,cpu);

				/*
				 * Writing to it clears the DTB PTE entries that matches the
				 * DTB_IS0 IPR.
				 */
				AXP_tbis(cpu, *((u64 *) &cpu->dtbIs0), true);
				break;

			case AXP_IPR_DTB_ASN0:
				AXP_MBOX_WRITE_DTB_ASN0(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DTB_ALTMODE:
				AXP_MBOX_WRITE_DTB_ALTMODE(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_M_CTL:
				AXP_MBOX_WRITE_M_CTL(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DC_CTL:
				AXP_MBOX_WRITE_DC_CTL(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DC_STAT:
				AXP_MBOX_WRITE_DC_STAT(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DTB_TAG1:
				AXP_MBOX_WRITE_DTB_TAG1(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_DTB_PTE1:
				AXP_MBOX_WRITE_DTB_PTE1(instr->src1v.r.uq,cpu);

				/*
				 * Retiring this instruction causes the TAG and PTE to be
				 * written into the DTB entry.
				 */
				AXP_addTLBEntry(
						cpu,
						*((u64 *) &cpu->dtbTag1),
						*((u64 *) &cpu->dtbPte1),
						true);
				break;

			case AXP_IPR_DTB_IAP:

				/*
				 * This is a Pseudo register.  Writing to it clears all the DTB
				 * PTE entries with an ASM bit clear.
				 */
				AXP_tbiap(cpu, true);
				break;

			case AXP_IPR_DTB_IA:

				/*
				 * This is a Pseudo register.  Writing to it clears all the DTB
				 * PTE entries.
				 */
				AXP_tbia(cpu, true);
				break;

			case AXP_IPR_DTB_IS1:
				AXP_MBOX_WRITE_DTB_IS1(instr->src1v.r.uq,cpu);

				/*
				 * Writing to it clears the DTB PTE entries that matches the
				 * DTB_IS0 IPR.
				 */
				AXP_tbis(cpu, *((u64 *) &cpu->dtbIs1), true);
				break;

			case AXP_IPR_DTB_ASN1:
				AXP_MBOX_WRITE_DTB_ASN1(instr->src1v.r.uq,cpu);
				break;

			default:
				break;
		}
		pthread_mutex_unlock(&cpu->mBoxIPRMutex);
	}

	/*
	 * Ebox IPR
	 */
	else if ((instr->type_hint_index >= AXP_IPR_CC) &&
			 (instr->type_hint_index <= AXP_IPR_VA_CTL))
	{
		pthread_mutex_lock(&cpu->eBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_CC:
				AXP_EBOX_WRITE_CC(instr->src1v.r.ul, cpu);
				break;

			case AXP_IPR_CC_CTL:
				AXP_EBOX_WRITE_CC_CTL(instr->src1v.r.uq, cpu);
				ccCtlCounter = cpu->ccCtl.counter;
				cpu->cc.counter = ccCtlCounter << 4;
				break;

			case AXP_IPR_VA_CTL:
				AXP_EBOX_WRITE_VA_CTL(instr->src1v.r.uq, cpu);
				break;

			default:
				break;
		}
		pthread_mutex_unlock(&cpu->eBoxIPRMutex);
	}

	/*
	 * Cbox IPR
	 */
	else
	{
		pthread_mutex_lock(&cpu->cBoxIPRMutex);
		switch (instr->type_hint_index)
		{
			case AXP_IPR_C_DATA:
				AXP_CBOX_WRITE_C_DATA(instr->src1v.r.uq,cpu);
				break;

			case AXP_IPR_C_SHFT:
				AXP_CBOX_WRITE_C_SHFT(instr->src1v.r.uq,cpu);
				break;

			default:
				break;
		}
		pthread_mutex_unlock(&cpu->cBoxIPRMutex);
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_AbortIns
 *	This function is called to abort any instructions starting at the indicated
 *	location in the array to the end location.  Then reset the end to be the
 *	same as the indicated starting location.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *	start:
 *		A value indicating where we should begin within the ROB array.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Ibox_AbortIns(AXP_21264_CPU *cpu, u32 start)
{
	AXP_INSTRUCTION		*rob;
	AXP_21264_REG_STATE *destState;
	AXP_QUEUE_ENTRY		*entry, *next;
	u16					*destMap;
	u16					*destFreeList, *flEnd;
	bool				split, destFloat, found = false;
	u32					ii, end;

	/*
	 * OK, we need to start at the next instruction in the Re-Order Buffer.
	 */
	start = (start + 1) == AXP_INFLIGHT_MAX ? 0 : start + 1;

	/*
	 * Are we going to wrap from the bottom of the array to the top?
	 */
	split = cpu->robEnd < start;
	end = (split ? AXP_INFLIGHT_MAX : cpu->robEnd);
	ii = start;
	while (ii < end)
	{

		/*
		 * Set up a pointer to the next instruction that needs to be aborted.
		 */
		rob = &cpu->rob[ii];

		/*
		 * First things first, we need to remove the IQ or FQ entry from the
		 * queue so it does not get processed any further.  Note, if the
		 * instruction has already be remove from the queue, then we either
		 * are executing the instruction, or the instruction is pending
		 * retirement.
		 *
		 * IQ first.
		 */
		pthread_mutex_lock(&cpu->eBoxMutex);
		entry = (AXP_QUEUE_ENTRY *) cpu->iq.flink;
		found = false;
		while (((void *) entry != &cpu->iq) && (found == false))
		{

			/*
			 * Log what we are about to do.
			 */
			if (AXP_IBOX_OPT1)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"ABORTING instruction at pc: 0x%016llx, opcode: 0x%02x, "
						"state = %s",
						entry->ins->pc,
						entry->ins->opcode,
						_ins_state_[entry->ins->state]);
				AXP_TRACE_END();
			}

			/*
			 * Get the address of the next entry now, because dequeuing the
			 * entry means that the flink and blink are unpredictable.
			 */
			next = (AXP_QUEUE_ENTRY *) entry->header.flink;

			/*
			 * If this next instruction in the IQ as an address that matches
			 * ROB entry being aborted, then remove it from the queue and
			 * return it back for future processing.
			 */
			if (entry->ins == rob)
			{
				AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
				AXP_ReturnIQEntry(cpu, entry);
				found = true;
			}

			/*
			 * OK, let's take a look at the next entry.
			 */
			entry = next;
		}
		pthread_mutex_unlock(&cpu->eBoxMutex);

		/*
		 * FQ next.  NOTE: Don't reset the found flag, as a particular ROB
		 * entry and only be in the IQ or FQ, but never both.  So, if we found
		 * it in the IQ, there is no need to search the FQ.
		 */
		pthread_mutex_lock(&cpu->fBoxMutex);
		entry = (AXP_QUEUE_ENTRY *) cpu->fq.flink;
		while (((void *) entry != &cpu->fq) && (found == false))
		{

			/*
			 * Get the address of the next entry now, because dequeuing the
			 * entry means that the flink and blink are unpredictable.
			 */
			next = (AXP_QUEUE_ENTRY *) entry->header.flink;

			/*
			 * If this next instruction in the IQ as an address that matches
			 * ROB entry being aborted, then remove it from the queue and
			 * return it back for future processing.
			 */
			if (entry->ins == rob)
			{
				AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
				AXP_ReturnIQEntry(cpu, entry);
				found = true;
			}

			/*
			 * OK, let's take a look at the next entry.
			 */
			entry = next;
		}
		pthread_mutex_unlock(&cpu->fBoxMutex);

		/*
		 * Clear out any exceptions that may have occurred.
		 */
		rob->excRegMask = NoException;

		/*
		 * If the destination register was not R31/F31, then we need to return
		 * it to the free list.
		 */
		if (rob->aDest != AXP_UNMAPPED_REG)
		{

			/*
			 * Determine if the destination register is a
			 * floating-point or integer register.
			 */
			destFloat =
				((rob->decodedReg.bits.dest & AXP_DEST_FLOAT) ==
						AXP_DEST_FLOAT);

			/*
			 * Set up some pointers so that the code below is a bit simpler.
			 */
			if (destFloat == true)
			{
				destState = cpu->pfState;
				destMap = cpu->pfMap;
				destFreeList = cpu->pfFreeList;
				flEnd = &cpu->pfFlEnd;
			}
			else
			{
				destState = cpu->prState;
				destMap = cpu->prMap;
				destFreeList = cpu->prFreeList;
				flEnd = &cpu->prFlEnd;
			}

			/*
			 * Log what we are about to do.
			 */
			if (AXP_IBOX_OPT1)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
					"AXP_21264_Ibox_AbortIns freeing register: P%c%02u",
					(destFloat ? 'F' :'R'),
					destMap[rob->aDest]);
				AXP_TRACE_END();
			}

			/*
			 * Put the destination register back on to the free-list and mark
			 * this physical register as being free.
			 */
			destFreeList[*flEnd] = destMap[rob->aDest];
			*flEnd = (*flEnd + 1) % AXP_F_FREELIST_SIZE;
			destState[destMap[rob->aDest]] = Free;
		}

		/*
		 * TODO: We need to go and clear out all events from the event queue.
		 * TODO: We need to roll back the VPC stack.
		 */

		/*
		 * Mark this instruction as being retired (its reset state).
		 */
		rob->state = Retired;

		/*
		 * Increment the loop counter.  If we reached the end of the array and
		 * it wrapped around, then reset the counter and the end value.  Also,
		 * note that the split is no longer relevant (avoids an infinite loop).
		 */
		ii++;
		if ((ii == end) && (split == true))
		{
			ii = 0;
			end = cpu->robEnd;
			split = false;
		}
	}

	/*
	 * The new end is the first aborted instruction.
	 */
	cpu->robEnd = start;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ibox_Retire
 *	This function is called whenever an instruction is transitioned to
 *	WaitingRetirement state.  This function will search through the ReOrder
 *	Buffer (ROB) from the oldest to the newest and retire all the instructions
 *	it can, in order.  If there was an exception, this should cause the
 *	remaining instructions to be flushed and not retired.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Ibox_Retire(AXP_21264_CPU *cpu)
{
	AXP_INSTRUCTION	*rob;
	u32				ii, end;
	bool			split;
	bool			done = false;
	bool			signalEbox = false;
	bool			signalFbox = false;
	bool			updateDest = false;

	/*
	 * First lock the ROB mutex so that it is not updated by anyone but this
	 * function.
	 */
	pthread_mutex_lock(&cpu->robMutex);

	if (AXP_IBOX_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(
				"AXP_Ibox_Retire called (start: %u, end: %u)",
				cpu->robStart, cpu->robEnd);
		AXP_TRACE_END();
	}

	/*
	 * The split flag is used to determine when the end index has wrapped to
	 * the start of the list, making it less than the beginning index (at least
	 * until the beginning index wraps as well).
	 */
	split = cpu->robEnd < cpu->robStart;

	/*
	 * Determine out initial start and end entries.  If the end has wrapped
	 * around, then we search in 2 passes (start to list end; list beginning to
	 * end).
	 */
	ii = cpu->robStart;
	end = (split ? AXP_INFLIGHT_MAX : cpu->robEnd);

	if (AXP_IBOX_OPT1)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("AXP_Ibox_Retire loop1 from: %u, to: %u", ii, end);
		AXP_TRACE_END();
	}

	/*
	 * Loop until we reach the end or we find an entry that is not ready for
	 * retirement (maybe its 401K is not where it should be or his employer
	 * bankrupt the pension fund).
	 */
	while ((ii < end) && (done == false))
	{
		rob = &cpu->rob[ii];
		if (AXP_IBOX_BUFF)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite(
				"ROB[%u] instruction at pc: 0x%016llx, opcode: 0x%02x, "
				"state = %s",
				ii,
				rob->pc,
				rob->opcode,
				_ins_state_[rob->state]);
			AXP_TRACE_END();
		}

		/*
		 * If the next entry is ready for retirement, then complete the work
		 * necessary for this instruction.  If it is not, then because
		 * instructions need to be completed in order, then we are done trying
		 * to retire instructions.
		 */
		if (rob->state == WaitingRetirement)
		{

			/*
			 * If an exception occurred, we need to process it.  Otherwise, the
			 * destination value should be written to the destination
			 * (physical) register.  If it is a store operation, then we need
			 * to update the Dcache.
			 */
			if (rob->excRegMask != NoException)
			{

				/*
				 * TODO:	There is quite a bit to re-do/do here.  Right now,
				 *			events can be generated from the Mbox and Cbox for
				 *			things that happen while executing an instruction.
				 *			We need to do some of these in-line with retiring
				 *			the instruction (namely, right here).
				 */
			}
			else
			{

				/*
				 * We do this here so that the subsequent code can move the IPR
				 * value into the correct register.  The HW_MTPR is handled
				 * below (in the switch statement).
				 */
				if (rob->opcode == HW_MFPR)
					AXP_21264_Ibox_Retire_HW_MFPR(cpu, rob);

				/*
				 * If this is a branch, we need to do the following:
				 *
				 *	1)	Update the branch prediction with whether we are taking
				 *		branch or not.
				 *	2)	If the branch is taken, update the destination
				 *		register.
				 *	3)	If the branch prediction did not match the actual
				 *		branch taken or not, the we may have to abort the
				 *		instructions loaded immediately after the branch.
				 *	4)	Set the add the branchPC to the VPC stack.
				 */
				if (rob->type == Branch)
				{
					bool	taken = (*(u64 *) &rob->branchPC) != 0;

					/*
					 * Step 1:
					 *
					 * Update the branch prediction logic.
					 */
					AXP_Branch_Direction(
									cpu,
									rob->pc,
									taken,
									rob->localPredict,
									rob->globalPredict);


					/*
					 * Step 2:
					 *
					 * If we took the branch, then we need to update the
					 * destination register, as well as the PC.
					 */
					if (taken)
					{
						if (((rob->decodedReg.bits.dest & AXP_DEST_FLOAT) ==
							 AXP_DEST_FLOAT) &&
							(rob->aDest != AXP_UNMAPPED_REG))
						{
							updateDest = true;
							signalFbox = true;
						}
						else if ((rob->decodedReg.bits.dest != 0) &&
								 (rob->aDest != AXP_UNMAPPED_REG))
						{
							updateDest = true;
							signalEbox = true;
						}

						/*
						 * Step 4:
						 */
						AXP_21264_AddVPC(cpu, rob->branchPC);
					}

					/*
					 * Step 3:
					 *
					 * If the branch prediction logic did not match the actual
					 * branch results, then we will need to abort all
					 * instructions subsequent to this one (just like they
					 * never happened - even if they are pending retirement).
					 */
					if (taken != rob->branchPredict)
					{

						/*
						 * Call the function to abort all instructions
						 * immediately after the current one.  This may change
						 * the value of cpu->robEnd.
						 */
						AXP_21264_Ibox_AbortIns(cpu, ii);

						/*
						 * Since we just change the end of the ROB, we need to
						 * recalculate the while loop values.
						 */
						split = cpu->robEnd < cpu->robStart;
						end = (split ? AXP_INFLIGHT_MAX : cpu->robEnd);
					}
				}
				else if (((rob->decodedReg.bits.dest & AXP_DEST_FLOAT) ==
						  AXP_DEST_FLOAT) &&
						 (rob->aDest != AXP_UNMAPPED_REG))
				{
					updateDest = true;
					signalFbox = true;
				}
				else if ((rob->decodedReg.bits.dest != 0) &&
						 (rob->aDest != AXP_UNMAPPED_REG))
				{
					updateDest = true;
					signalEbox = true;
				}

				/*
				 * If the destination register needs to be updated, then do so
				 * now.
				 */
				if (updateDest == true)
				{

					/*
					 * If we will be signaling the Fbox (floating-point
					 * processing), then we need to update the destination
					 * physical register.
					 *
					 * NOTE:	We already determined that we are not writing
					 *			to F31.
					 */
					if (signalFbox == true)
					{
						cpu->pf[rob->dest] = rob->destv.fp.uq;
						cpu->pfState[rob->dest] = Valid;
					}

					/*
					 * We will be signaling the Ebox (integer processing), then
					 * we need to update the destination physical register.
					 *
					 * NOTE:	We already determined that we are not writing
					 *			to R31.
					 */
					else
					{
						cpu->pr[rob->dest] = rob->destv.r.uq;
						cpu->prState[rob->dest] = Valid;
					}
				}

				/*
				 * If a store, write it to the Dcache.
				 */
				switch (rob->opcode)
				{
					case STW:
					case STB:
					case STQ_U:
					case HW_ST:
					case STF:
					case STG:
					case STS:
					case STT:
					case STL:
					case STQ:
					case STL_C:
					case STQ_C:
						AXP_21264_Mbox_RetireWrite(cpu, rob->slot);
						break;

					case HW_MTPR:
						AXP_21264_Ibox_Retire_HW_MTPR(cpu, rob);
						break;

					case HW_RET:

						/*
						 * If this is a HW_RET/STALL and a write to the
						 * IC_FLUSH Pseudo register was previously made, then
						 * we now need to flush the Icache.
						 */
						if ((rob->type_hint_index == AXP_HW_RET) &&
							(rob->len_stall == 1) &&
							(cpu->iCacheFlushPending == true))
						{
							AXP_IcacheFlush(cpu, false);
						}
						break;

					default:
						break;
				}
			}

			/*
			 * Mark the instruction retired and move the top of the stack to
			 * the next instruction location.
			 */
			cpu->rob[ii].state = Retired;
			cpu->robStart = (cpu->robStart + 1) % AXP_INFLIGHT_MAX;
			if (AXP_IBOX_INST)
			{
				char	insBuf[256];
				char	regBuf[128];

				AXP_Decode_Instruction(&rob->pc, rob->instr, false, insBuf);
				AXP_Dump_Registers(rob, cpu->pr, cpu->pf, regBuf);
				AXP_TRACE_BEGIN();
				AXP_TraceWrite("%s : %s", insBuf, regBuf);
				AXP_TRACE_END();
			}
		}
		else
			done = true;

		/*
		 * We processed the current ROB.  Time to move onto the next.
		 */
		ii++;

		/*
		 * If we reached the end, but the search is split, then change the
		 * index to the start of the list and the end to the end of the list.
		 * Clear the split flag, so that we don't get ourselves into an
		 * infinite loop.
		 */
		if ((ii == end) && (split == true))
		{
			ii = 0;
			end = cpu->robEnd;
			split = false;

			if (AXP_IBOX_OPT1)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite("AXP_Ibox_Retire loop2 from: 0, to: %u", end);
				AXP_TRACE_END();
			}
		}
	}

	/*
	 * Finally, unlock the ROB mutex so that it can be updated by another
	 * thread.
	 */
	pthread_mutex_unlock(&cpu->robMutex);

	/*
	 * There may have been an instruction that was waiting for one of the
	 * destination registers to be written to before it could execute.  If one
	 * of them was for the Fbox, signal it to wake up and check to see if there
	 * is one or more Queued Floating Point instructions that can now be
	 * executed.
	 */
	if (signalFbox == true)
		pthread_cond_broadcast(&cpu->fBoxCondition);

	/*
	 * There may have been an instruction that was waiting for one of the
	 * destination registers to be written to before it could execute.  If one
	 * of them was for the Ebox, signal it to wake up and check to see if there
	 * is one or more Queued Integer instructions that can now be executed.
	 */
	if (signalEbox == true)
		pthread_cond_broadcast(&cpu->eBoxCondition);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_IboxMain
 * 	This function is called to perform the emulation for the Ibox within the
 * 	Alpha AXP 21264 CPU.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the structure holding the  fields required to emulate an
 * 		Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void *AXP_21264_IboxMain(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;
	AXP_PC			nextPC, branchPC;
	AXP_INS_LINE	nextCacheLine;
	AXP_EXCEPTIONS	exception;
	AXP_INSTRUCTION	*decodedInstr;
	AXP_QUEUE_ENTRY	*xqEntry;
	u32				ii, fault;
	bool			choice, wasRunning = false;
	bool			_asm;
	u16				whichQueue;
	int				qFull;
	bool			noop;

	/*
	 * Make sure to initialize the line and set prediction information.
	 */
	nextCacheLine.branch2bTaken = false;
	nextCacheLine.linePrediction = 0;
	nextCacheLine.setPrediction = 0;

	/*
	 * OK, we are just starting out and there is probably nothing available to
	 * process, yet.  Lock the CPU mutex, which the state of the CPU and if not
	 * in a Run or ShuttingDown state, then wait on the CPU condition variable.
	 */
	pthread_mutex_lock(&cpu->cpuMutex);
	while ((cpu->cpuState != Run) && (cpu->cpuState != ShuttingDown))
	{
		if (AXP_IBOX_CALL)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite(
				"Ibox is waiting for CPU to be in Run State (%d)",
				cpu->cpuState);
			AXP_TRACE_END();
		}
		pthread_cond_wait(&cpu->cpuCond, &cpu->cpuMutex);
	}
	pthread_mutex_unlock(&cpu->cpuMutex);

	/*
	 * OK, we've either been successfully initialized or we are shutting-down
	 * before we even started.  If it is the former, then we need to lock the
	 * iBox mutex.
	 */
	if (cpu->cpuState == Run)
	{
		if (AXP_IBOX_OPT1)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite("Ibox is in Running State");
			AXP_TRACE_END();
		}
		pthread_mutex_lock(&cpu->iBoxMutex);
		wasRunning = true;
	}

	/*
	 * Here we'll loop starting at the current PC and working our way through
	 * all the instructions.  We will do the following steps.
	 *
	 *	1) Fetch the next set of instructions.
	 *	2) If step 1 returns a Miss, then get the Cbox to fill the Icache with
	 *		the next set of instructions.
	 *	3) If step 1 returns a WayMiss, then we need to generate an ITB Miss
	 *		exception, with the PC address we were trying to step to as the
	 *		return address.
	 *	4) If step 1 returns a Hit, then process the next set of instructions.
	 *		a) Decode and rename the registers in each instruction into the ROB.
	 *		b) If the decoded instruction is a branch, then predict if this
	 *			branch will be taken.
	 *		c) If step 4b is true, then adjust the line and set predictors
	 *			appropriately.
	 *		d) Fetch and insert an instruction entry into the appropriate
	 *			instruction queue (IQ or FQ).
	 *	5) If the branch predictor indicated a branch, then determine if
	 *		we have to load an ITB entry and ultimately load the iCache
	 *	6) Loop back to step 1.
	 */

	/*
	 * We keep looping while the CPU is in a running state.
	 */
	while (cpu->cpuState == Run)
	{

		/*
		 * Exceptions take precedence over normal CPU processing.  IF an
		 * exception occurred, then make this the next PC and clear the
		 * exception pending flag.
		 */
		if (cpu->excPend == true)
		{

			/*
			 * TODO: Push the excAddr onto the prediction stack.
			 */
			nextPC = cpu->excPC;
			cpu->excPend = false;

		}
		else

			/*
			 * Get the PC for the next set of instructions to be fetched from
			 * the Icache and Fetch those instructions.
			 */
			nextPC = AXP_21264_GetNextVPC(cpu);

		/*
		 * The cache fetch will return true or false.  If true, we received the
		 * next four instructions.  If false, we need to to determine if we
		 * need to call the PALcode to add a TLB entry to the ITB and/or then
		 * get the Cbox to fill the iCache.  If the former, store the faulting
		 * PC and generate an exception.
		 */
		if (AXP_IcacheFetch(cpu, nextPC, &nextCacheLine) == true)
		{
			for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
			{

				/*
				 * Lock the ROB mutex so that it is not updated by anyone but
				 * this function.
				 */
				pthread_mutex_lock(&cpu->robMutex);
				decodedInstr = &cpu->rob[cpu->robEnd];
				if (AXP_IBOX_BUFF)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
						"ROB[%u] getting instruction at pc: 0x%016llx",
						cpu->robEnd,
						*((u64 *) &nextPC));
					AXP_TRACE_END();
				}

				cpu->robEnd = (cpu->robEnd + 1) % AXP_INFLIGHT_MAX;

				/*
				 * We are done with the ROB mutex.
				 */
				pthread_mutex_unlock(&cpu->robMutex);

				/*
				 * Go and decode the instruction, as well as rename the
				 * architectural registers to their physical equivalent.
				 */
				AXP_Decode_Rename(cpu, &nextCacheLine, ii, decodedInstr);
				if (decodedInstr->type == Branch)
				{
					decodedInstr->branchPredict = AXP_Branch_Prediction(
								cpu,
								nextPC,
								&decodedInstr->localPredict,
								&decodedInstr->globalPredict,
								&choice);

					/*
					 * TODO:	First, we can use the PC handling functions
					 *			to calculate the branch PC.
					 * TODO:	Second, we need to make sure that we are
					 *			calculating the branch address correctly.
					 * TODO:	Third, We need to be able to handle
					 *			returns, and utilization of the, yet to be
					 *			implemented, prediction stack.
					 * TODO:	Finally, we need to flush the remaining
					 *			instructions to be decoded and go get the
					 *			predicted instructions.
					 */
					if (decodedInstr->branchPredict == true)
					{
						branchPC.pc = nextPC.pc + 1 + decodedInstr->displacement;
						if (AXP_IcacheValid(cpu, branchPC) == false)
						{
							u64		pa;
							bool	_asm;
							u32		fault;

							/*
							 * We are branching to a location that is not
							 * currently in the Icache.  We have to do the
							 * following:
							 *	1) Convert the virtual address to a physical
							 *	   address.
							 * 	2) Request the Cbox fetch the next set of
							 * 	   instructions.
							 */
							pa = AXP_va2pa(
									cpu,
									*((u64 *) &branchPC),
									nextPC,
									false,
									Execute,
									&_asm,
									&fault,
									&exception);

							/*
							 * TODO:	We need to check that we don't have a
							 * 			hit in the Bcache, before requesting
							 * 			it.  Also, not if we fill in the Icache
							 * 			from the Bcache, then we need to check
							 * 			the value of cpu->->hwIntClr.fbtp to
							 * 			generate a 'Bad Icache fill parity'.
							 */
							AXP_21264_Add_MAF(
									cpu,
									Istream,
									pa,
									0,
									AXP_ICACHE_BUF_LEN,
									false);
						}

						/*
						 * TODO:	If we get a Hit, there is nothing else
						 * 			to do.  If we get a Miss, we probably
						 * 			should have someone fill the Icache
						 * 			with the next set of instructions.  If
						 * 			a WayMiss, we don't do anything either.
						 * 			In this last case, we will end up
						 * 			generating an ITB_MISS event to be
						 * 			handled by the PALcode.
						 */
					}
				}

				/*
				 * If this is one of the potential NOOP instructions, then the
				 * instruction is already completed and does not need to be
				 * queued up.
				 */
				noop = (decodedInstr->pipeline == PipelineNone ? true : false);
				if (decodedInstr->aDest == AXP_UNMAPPED_REG)
				{
					switch (decodedInstr->opcode)
					{
						case INTA:
						case INTL:
						case INTM:
						case INTS:
						case LDQ_U:
						case ITFP:
							if (decodedInstr->aDest == AXP_UNMAPPED_REG)
								noop = true;
							break;

						case FLTI:
						case FLTL:
						case FLTV:
							if ((decodedInstr->aDest == AXP_UNMAPPED_REG) &&
								(decodedInstr->function != AXP_FUNC_MT_FPCR))
								noop = true;
							break;
					}
				}
				if (noop == false)
				{

					/*
					 * Before we do much more, if we have a load/store, we need
					 * to request an entry in either the LQ or SQ in the Mbox.
					 */
					switch (decodedInstr->opcode)
					{
						case LDBU:
						case LDQ_U:
						case LDW_U:
						case HW_LD:
						case LDF:
						case LDG:
						case LDS:
						case LDT:
						case LDL:
						case LDQ:
						case LDL_L:
						case LDQ_L:
							decodedInstr->slot = AXP_21264_Mbox_GetLQSlot(cpu);
							break;

						case STW:
						case STB:
						case STQ_U:
						case HW_ST:
						case STF:
						case STG:
						case STS:
						case STT:
						case STL:
						case STQ:
						case STL_C:
						case STQ_C:
							decodedInstr->slot = AXP_21264_Mbox_GetSQSlot(cpu);
							break;

						default:
							break;
					}
					whichQueue = AXP_InstructionQueue(decodedInstr->opcode);
					if(whichQueue == AXP_COND)
					{
						if (decodedInstr->opcode == ITFP)
						{
							if ((decodedInstr->function == AXP_FUNC_ITOFS) ||
								(decodedInstr->function == AXP_FUNC_ITOFF) ||
								(decodedInstr->function == AXP_FUNC_ITOFT))
								whichQueue = AXP_IQ;
							else
								whichQueue = AXP_FQ;
						}
						else	/* FPTI */
						{
							if ((decodedInstr->function == AXP_FUNC_FTOIT) ||
								(decodedInstr->function == AXP_FUNC_FTOIS))
								whichQueue = AXP_FQ;
							else
								whichQueue = AXP_IQ;
							}
					}
					if (whichQueue == AXP_IQ)
					{
						xqEntry = AXP_GetNextIQEntry(cpu);
						xqEntry->ins = decodedInstr;
						pthread_mutex_lock(&cpu->eBoxMutex);
						qFull = AXP_InsertCountedQueue(
								(AXP_QUEUE_HDR *) &cpu->iq,
								(AXP_CQUE_ENTRY *) xqEntry);
						pthread_cond_broadcast(&cpu->eBoxCondition);
						pthread_mutex_unlock(&cpu->eBoxMutex);
					}
					else	/* FQ */
					{
						xqEntry = AXP_GetNextFQEntry(cpu);
						xqEntry->ins = decodedInstr;
						pthread_mutex_lock(&cpu->fBoxMutex);
						qFull = AXP_InsertCountedQueue(
								(AXP_QUEUE_HDR *) &cpu->fq,
								(AXP_CQUE_ENTRY *) xqEntry);
						pthread_cond_broadcast(&cpu->fBoxCondition);
						pthread_mutex_unlock(&cpu->fBoxMutex);
					}

					/*
					 * TODO:	We need to make sure that there is at least four
					 *			entries available in the IQ/FQ, not just 1.
					 */
					if (qFull < 0)
						printf("\n>>>>> We need to determine if there are any instructions to parse <<<<<\n");
					decodedInstr->state = Queued;
				}
				else
					decodedInstr->state = WaitingRetirement;

				/*
				 * Increment the PC, then add it to the PC list.
				 */
				nextPC = AXP_21264_IncrementVPC(cpu);
				AXP_21264_AddVPC(cpu, nextPC);
			}
		}

		/*
		 * We failed to get the next instruction.  We need to request an Icache
		 * Fill, or we have an ITB_MISS
		 */
		else
		{
			AXP_21264_TLB *itb;

			itb = AXP_findTLBEntry(cpu, *((u64 *) &nextPC), false);

			/*
			 * If we didn't get an ITB, then we got to a virtual address that
			 * has not yet to be mapped.  We need to call the PALcode to get
			 * this mapping for us, at which time we'll attempt to fetch the
			 * instructions again, which will cause us to get here again, but
			 * this time the ITB will be found.
			 */
			if (itb == NULL)
			{
				AXP_21264_Ibox_Event(
							cpu,
							AXP_ITB_MISS,
							nextPC,
							*((u64 *) &nextPC),
							PAL00,
							AXP_UNMAPPED_REG,
							false,
							true);
			}

			/*
			 * We failed to get the next set of instructions from the Icache.
			 * We need to request the Cbox to get them and put them into the
			 * cache.  We are going to have some kind of pending Cbox indicator
			 * to know when the Cbox has actually filled in the cache block.
			 *
			 * TODO:	We may want to try and utilize the branch predictor to
			 * 			"look ahead" and request the Cbox fill in the Icache
			 * 			before we have a cache miss, in oder to avoid this
			 * 			waiting on the Cbox (at least for too long a period of
			 * 			time).
			 */
			else
			{
				u64	pa;
				AXP_EXCEPTIONS exception;

				/*
				 * First, try and convert the virtual address of the PC into
				 * its physical address equivalent.
				 */
				pa = AXP_va2pa(
						cpu,
						*((u64 *) &nextPC),
						nextPC,
						false,
						Execute,
						&_asm,
						&fault,
						&exception);

				/*
				 * If converting the VA to a PA generated an exception, then we
				 * need to handle this now.  Otherwise, put in a request to the
				 * Cbox to perform a Icache Fill.
				 */
				if (exception != NoException)
					AXP_21264_Ibox_Event(
								cpu,
								fault,
								nextPC,
								*((u64 *) &nextPC),
								PAL00,
								AXP_UNMAPPED_REG,
								false,
								true);
				else
					AXP_21264_Add_MAF(
							cpu,
							Istream,
							pa,
							0,
							AXP_ICACHE_BUF_LEN,
							false);
			}
		}

		/*
		 * Before we loop back to the top, we need to see if there is something
		 * to process or places to put what needs to be processed (IQ and/or FQ
		 * cannot handle another entry).
		 */
		if (((cpu->excPend == false) &&
			 (AXP_IcacheValid(cpu, nextPC) == false)) ||
			((AXP_CountedQueueFull(&cpu->iq) < 0) ||
			 (AXP_CountedQueueFull(&cpu->fq) < 0)))
			pthread_cond_wait(&cpu->iBoxCondition, &cpu->iBoxMutex);
	}
	if (AXP_IBOX_OPT1)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(
			"Ibox is not/no longer in the Run State (%d)",
			cpu->cpuState);
		AXP_TRACE_END();
	}

	/*
	 * If we set the wasRunning flag, then we locked the iBox mutex.  Make
	 * make sure we unlock it.
	 */
	if (wasRunning == true)
	{
		pthread_mutex_unlock(&cpu->iBoxMutex);
	}
	return(NULL);
}
