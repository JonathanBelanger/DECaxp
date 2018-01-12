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
 *	functionality of the Fbox.
 *
 *	Revision History:
 *
 *	V01.000		19-June-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	v01.001		03-Nov-2017		Jonathan D. Belanger
 *	Added Fbox initialization code.
 *
 *	V01.002		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
*/
#include "AXP_Configure.h"
#include "AXP_21264_Fbox.h"

/*
 * AXP_21264_Fbox_RegisterReady
 *	This function is called to determine if a queued instruction's registers
 *	are ready for execution.  If one or more registers is waiting for a
 *	previous instruction to finish its execution and store the value this
 *	instruction needs.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		A pointer to the entry containing all the pre-parsed information of the
 *		instruction so that we can determine which physical registers are being
 *		used and which are needed for this instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 * 	true:	The registers for instruction execution are ready.
 * 	false:	The registers for instruction execution are NOT ready.
 */
bool AXP_21264_Fbox_RegistersReady(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{
	return ((cpu->pfState[entry->ins->src1] == Valid) &&
			(cpu->pfState[entry->ins->src2] == Valid) &&
			(cpu->pfState[entry->ins->dest] == Valid));
}

/*
 * AXP_21264_Fbox_Compl
 *	This function is called by the Mbox for Integer Store operations.  This is
 *	very similar to the Integer Load Complete functions that are individually
 *	written for each kind of unique load instruction.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
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
void AXP_21264_Fbox_Compl(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	u64 tmp, exp;
	AXP_F_MEMORY *tmpF = (AXP_F_MEMORY *) &tmp;
	AXP_G_MEMORY *tmpG = (AXP_G_MEMORY *) &tmp;
	AXP_S_MEMORY *tmpS = (AXP_S_MEMORY *) &tmp;
	AXP_T_MEMORY *tmpT = (AXP_T_MEMORY *) &tmp;

	if (instr->excRegMask == NoException)
	{
		switch (instr->opcode)
		{
			case LDF:

				/*
				 * The load operation put the value we were supposed to read
				 * into the destination value location.  Get it out of there so
				 * that we can convert it from memory format to register
				 * format.
				 */
				tmp = instr->destv.fp.uq;

				/*
				 * Extract the exponent, then expand it from 8-bits to 11-bits.
				 */
				exp = tmpF->exponent;
				if (exp != 0)
					exp += (AXP_G_BIAS - AXP_F_BIAS);

				/*
				 * Now put everything back together, but this time in register
				 * format and 64-bits.
				 */
				instr->destv.fp.fCvt.sign = tmpF->sign;
				instr->destv.fp.fCvt.exponent = exp;
				instr->destv.fp.fCvt.fractionHigh = tmpF->fractionHigh;
				instr->destv.fp.fCvt.fractionLow = tmpF->fractionLow;
				instr->destv.fp.fCvt.zero = 0;
				break;

			case LDG:

				/*
				 * The load operation put the value we were supposed to read into the
				 * destination value location.  Get it out of there so that we can convert
				 * it from memory format to register format.
				 */
				tmp = instr->destv.fp.uq;

				/*
				 * Now put everything back together, but this time in register format and
				 * 64-bits.
				 */
				instr->destv.fp.gCvt.sign = tmpG->sign;
				instr->destv.fp.gCvt.exponent = tmpG->exponent;
				instr->destv.fp.gCvt.fractionHigh = tmpG->fractionHigh;
				instr->destv.fp.gCvt.fractionMidHigh = tmpG->fractionMidHigh;
				instr->destv.fp.gCvt.fractionMidLow = tmpG->fractionMidLow;
				instr->destv.fp.gCvt.fractionLow = tmpG->fractionLow;
				break;

			case LDS:

				/*
				 * The load operation put the value we were supposed to read into the
				 * destination value location.  Get it out of there so that we can convert
				 * it from memory format to register format.
				 */
				tmp = instr->destv.fp.uq;

				/*
				 * Extract the exponent, then expand it from 8-bits to 11-bits.
				 */
				exp = tmpS->exponent;
				if (exp == AXP_S_NAN)
					exp = AXP_R_NAN;
				else if (exp != 0)
					exp += (AXP_T_BIAS - AXP_S_BIAS);

				/*
				 * Now put everything back together, but this time in register format and
				 * 64-bits.
				 */
				instr->destv.fp.sCvt.sign = tmpS->sign;
				instr->destv.fp.sCvt.exponent = exp;
				instr->destv.fp.sCvt.fraction = tmpS->fraction;
				instr->destv.fp.sCvt.zero = 0;
				break;

			default:
				break;
		}
	}

	/*
	 * Indicate that the instruction is ready to be retired.
	 */
	instr->state = WaitingRetirement;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Fbox_Retire
 *	This function is called whenever a floating-point instruction is
 *	transitioned to WaitingRetirement state.  This function will search through
 *	the ReOrder Buffer (ROB) from the oldest to the newest and retire all the
 *	instructions it can, in order.  If there was an exception, this should
 *	cause the remaining instructions to be flushed and not retired.
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
void AXP_21264_Fbox_Retire(AXP_21264_CPU *cpu)
{
	AXP_INSTRUCTION	*rob;
	u32				ii, start, end;
	bool			split;
	bool			done = false;

	/*
	 * TODO:	While doing this, we need to lock a mutex so that the ROB will
	 *			not be update while we are processing through it.
	 */

	split = cpu->robEnd < cpu->robStart;

	/*
	 * Determine out initial start and end entries.  If the end has wrapped
	 * around, then we search in 2 passes (start to list end; list beginning to
	 * end).
	 */
	start = cpu->robStart;
	end = (split ? end = cpu->robEnd : AXP_INFLIGHT_MAX);

	/*
	 * Set our starting value.  Loop until we reach the end or we find an entry
	 * that is not ready for retirement (maybe its 401K is not where it should
	 * be or his employer bankrupt the pension fund).
	 */
	ii = start;
	while ((ii < end) && (done == false))
	{
		rob = &cpu->rob[ii];

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
			 * destingation value should be written to the destination
			 * (physical) register.  If it is a store operation, then we need
			 * to update the Dcache.
			 */
			if (rob->excRegMask != NoException)
			{
				/* TODO: We need to report the exception for the instruction */
			}
			else
			{
				cpu->pf[rob->dest] = rob->destv;

				/*
				 * If a store, write it to the Dcache.
				 */
				if ((rob->opcode == STF) || (rob->opcode == STG) ||
					(rob->opcode == STS) || (rob->opcode == STT))
					AXP_21264_Mbox_RetireWrite(cpu, rob->slot);
			}

			/*
			 * Mark the instruction retired and move the top of the stack to
			 * the next instruction location.
			 */
			cpu->rob[ii].state = Retired;
			cpu->robStart = (cpu->robStart + 1) % AXP_INFLIGHT_MAX;
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
		}
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Fbox_Init(AXP_21264_CPU *cpu)
 *	This function is called to initialize the Fbox.  The Fbox only contains one
 *	IPR, but there are other items that also need initialization.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure for the emulated Alpha AXP 21264
 *		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to perform all initialization processing.
 *	false:	Normal Successful Completion.
 */
bool AXP_21264_Fbox_Init(AXP_21264_CPU *cpu)
{
	bool		retVal = false;
	int			ii;

	/*
	 * Initlaize the one Fbox IPR.
	 */
	cpu->fpcr.res = 0;
	cpu->fpcr.dnz = 0;
	cpu->fpcr.invd = 0;
	cpu->fpcr.dzed = 0;
	cpu->fpcr.ovfd = 0;
	cpu->fpcr.inv = 0;
	cpu->fpcr.dze = 0;
	cpu->fpcr.ovf = 0;
	cpu->fpcr.unf = 0;
	cpu->fpcr.ine = 0;
	cpu->fpcr.iov = 0;
	cpu->fpcr.dyn = 0;
	cpu->fpcr.undz = 0;
	cpu->fpcr.unfd = 0;
	cpu->fpcr.ined = 0;
	cpu->fpcr.sum = 0;

	/*
	 * Set up the initial register map.  We do not map F31.
	 */
	for (ii = 0; ii < (AXP_MAX_REGISTERS-1); ii++)
	{
		cpu->pf[ii] = 0;
		cpu->pfMap[ii].pr = ii;
		cpu->pfMap[ii].prevPr = AXP_UNMAPPED_REG;
		cpu->pfState[ii] = Valid;
	}


	/*
	 * The above loop initialized the pfMap array entries from 0 to 30 to be
	 * mapped to the physical registers also from 0 to 30.  The next lines of
	 * code initialize the mapping for F31 to be mapped to an invalid physical
	 * register.  This is used to indicate to the code that implements the
	 * Alpha AXP instructions that, as a source register is always a value
	 * of 0, and as a destination register, never updated.  This will greatly
	 * simplify the register (architectural and physical) handling.
	 */
	cpu->pfMap[AXP_MAX_REGISTERS-1].pr = AXP_FP_PHYS_REG + 1;
	cpu->pfMap[AXP_MAX_REGISTERS-1].prevPr = AXP_FP_PHYS_REG + 1;

	/*
	 * The remaining physical registers need to be put on the free list.
	 */
	cpu->pfFlStart = 0;
	cpu->pfFlEnd = 0;
	for (ii = AXP_MAX_REGISTERS; ii < AXP_FP_PHYS_REG; ii++)
	{
		cpu->pfState[ii] = Free;
		cpu->pfFreeList[cpu->pfFlEnd++] = ii;
	}

	/*
	 * Initialize the instruction queue for the Fbox.
	 */
	AXP_INIT_CQUE(cpu->fq, AXP_IQ_LEN);

	/*
	 * Initialize the instruction queue cache.  These are pre-allocated queue
	 * entries for the above.
	 */
	for (ii = 0; ii < AXP_FQ_LEN; ii++)
	{
		cpu->fqEFreelist[cpu->fqEFlEnd++] = ii;
		AXP_INIT_CQENTRY(cpu->fqEntries[ii].header, cpu->fq);
		cpu->fqEntries[ii].index = ii;
	}

	/*
	 * Return a success back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_FboxMulMain
 *	This is the main function for the Multiply Cluster of the Fbox (Floating
 *	Point) pipeline.  It is called by the pthread_create function.  It calls
 *	the Fbox main function to perform the instruction execution for the
 *	Mutliply Cluster of teh Digital Alpha AXP 21264 Central Processing Unit
 *	emulation.
 *
 *	NOTE:	In the real Alpha AXP 21264 CPU, only multiplication is merformed
 *			in this pipeline.  In the emulator, I have both multiply and
 *			divide executing in this pipleline.  If testing determines that
 *			these should be separated, then it is a simple data change (and
 *			recompile to change this).
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_FboxMulMain(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_FboxMain(cpu, FboxMul);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_FboxOthMain
 *	This is the main function for the Cluster of the Fbox (Floating
 *	Point) pipeline.  It is called by the pthread_create function.  It calls
 *	the Fbox main function to perform the instruction execution for the
 *	Mutliply Cluster of teh Digital Alpha AXP 21264 Central Processing Unit
 *	emulation.
 *
 *	NOTE:	In the real Alpha AXP 21264 CPU, division is not merformed in this
 *			pipeline.  In the emulator, I have both multiply and divide
 *			executing in the Multiply pipleline.  If testing determines that
 *			these should be separated, then it is a simple data change (and
 *			recompile to change this).
 *
 * Input Parameters:
 *	voidPtr:
 *		A void pointer which is actually the address of the Digital Alpha AXP
 *		21264 cpu data structure.  It'll be recast within the code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void *AXP_21264_FboxOthMain(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_EboxMain(cpu, FboxOther);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_FboxMain
 *	This is the main function for all the Floating Point pipelines.  It is
 *	called with the inforamtion needed to perform the processing for a specific
 *	pipeline within the Fbox pipeline  It waits on something needing processing
 *	to be put onto the FQ.  It scans from oldest to newest looking for the next
 *	instruction to be able to be processed by the Multiply/(Add/Divide/Square
 *	Root - Other) CPU pipeline.
 *
 *	The Fbox is broken up into 2 pipelines, Mutliply and Add/Divide/Square
 *	Root.  Each of these pipelines is a separate thread, but they both share
 *	the same FQ.  In order to be able to handle this, there is a single
 *	mutex/condition variable.  To avoid one thread locking out another, the
 *	mutex is only locked while either looking for the next instruction to
 *	process or waiting for the condition variable to be broadcast.  Once a
 *	queued instruction is found that can be processed by this pipeline, its
 *	state will be set to executing and then the mutex unlocked.  If nothing is
 *	found that can be executed, then this thread will wait on the condition
 *	again, which will unlock the mutex.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the Digital Alpha AXP 21264 cpu data structure.
 *	pipeline:
 *		A value indicating which pipeline this function will be executing.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_FboxMain(AXP_21264_CPU *cpu, AXP_PIPELINE pipeline)
{
	AXP_QUEUE_ENTRY		*entry;

	/*
	 * First things first, lock the Ebox mutex.
	 */
	pthread_mutex_lock(&cpu->fBoxMutex);

	/*
	 * While we are not shutting down, we'll continue to try and process
	 * instructions.
	 */
	while (cpu->cpuState != ShuttingDown)
	{

		/*
		 * Next we need to do is see if there is nothing to process,
		 * then wait for something to get queued up.
		 */
		while ((AXP_CQUE_EMPTY(cpu->fq) == true) &&
			   (cpu->cpuState != ShuttingDown))
			pthread_cond_wait(&cpu->fBoxCondition, &cpu->fBoxMutex);

		/*
		 * If we are not shutting down, then we may have something to process.
		 * Let's go looking for trouble.
		 */
		if (cpu->cpuState != ShuttingDown)
		{
			entry = (AXP_QUEUE_ENTRY *) cpu->fq.flink;

			/*
			 * Search through the queue of pending floating point pipeline
			 * instructions.  If we find one for this cluster, then break out
			 * of this loop.  Otherwise, move on to the next entry in the
			 * queue.  Since the queue eventually points back to the
			 * parent/header, this will exit the loop as well.
			 */
			while ((void *) entry != (void *) &cpu->fq)
			{

				/*
				 * We are only looking for entries that can be executed in the
				 * correct Floating Point cluster and have only been queued for
				 * processing and the registers needed for the instruction are
				 * ready to be used (the source registers need to not be
				 * waiting for pervious instruction to write to it).
				 */
				if ((entry->ins->pipeline == pipeline) &&
					(entry->ins->state == Queued) &&
					(AXP_21264_Fbox_RegistersReady(cpu, entry) == true))
					break;
				else
					entry = (AXP_QUEUE_ENTRY *) entry->header.flink;
			}

			/*
			 * If we did not find an instruction to execute, then go back to
			 * the beginning of the loop.  Since we did not unlock the mutex,
			 * we do not need to lock it now.
			 */
			if ((void *) entry == (void *) &cpu->fq)
				continue;

			/*
			 * OK, we have something to execute.  Mark the entry as such and
			 * dequeue it from the queue.  Then, dispatch it to the function
			 * to execute the instruction.
			 */
			entry->ins->state = Executing;
			AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
			pthread_mutex_unlock(&cpu->fBoxMutex);

			/*
			 * Call the dispatcher to dispatch this instruction to the correct
			 * function to execute the instruction.
			 */
			AXP_Dispatcher(cpu, entry->ins);
		}

		/*
		 * before going to the top of the loop, lock the Fbox mutex.
		 */
		pthread_mutex_lock(&cpu->fBoxMutex);
	}

	/*
	 * Last things lasst, lock the Fbox mutex.
	 */
	pthread_mutex_unlock(&cpu->fBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}
