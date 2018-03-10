/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	This source file contains the functions needed to implement the instruction
 *	execution loop for both the Ebox and Fbox.
 *
 *	Revision History:
 *
 *	V01.000		26-June-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Ibox.h"
#include "AXP_21264_Ibox_InstructionInfo.h"
#include "AXP_Trace.h"

#define AXP_PIPE_OPTIONS	10
static AXP_PIPELINE pipeCond[AXP_PIPE_OPTIONS][3] =
{
	{PipelineNone,	PipelineNone,	PipelineNone},
	{EboxU0,		EboxU0U1,		EboxL0L1U0U1},
	{EboxU1,		EboxU0U1,		EboxL0L1U0U1},
	{PipelineNone,	PipelineNone,	PipelineNone},
	{EboxL0,		EboxL0L1,		EboxL0L1U0U1},
	{EboxL1,		EboxL0L1,		EboxL0L1U0U1},
	{PipelineNone,	PipelineNone,	PipelineNone},
	{PipelineNone,	PipelineNone,	PipelineNone},
	{FboxMul,		FboxMul,		FboxMul},
	{FboxOther,		FboxOther,		FboxOther}
};

static char *pipelineStr[] =
{
	"None",
	"Ebox U0",
	"Ebox U1",
	"",
	"Ebox L0",
	"Ebox L1",
	"",
	"",
	"Fbox Multiply",
	"Fbox Other"
};

static char *insPipelineStr[] =
{
	"None",
	"U0",
	"U1",
	"U0, U1",
	"L0",
	"L1",
	"L0, L1",
	"L0, L1, U0, U1",
	"Multiply",
	"Other"
};
static char *insStateStr[] =
{
	"Retired",
	"Queued",
	"Executing",
	"WaitingRetirement",
	"Aborted"
};

static char *regStateStr[] =
{
	"Free",
	"Pending Update",
	"Valid"
};

/*
 * AXP_RegisterReady
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
static bool AXP_RegistersReady(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{
	bool			retVal;
	bool			src1Float;
	bool			src2Float;
	bool			destFloat;
	AXP_REGISTERS	*src1Reg;
	AXP_REGISTERS	*src2Reg;
	AXP_REGISTERS	*destReg;

	src1Float = ((entry->ins->decodedReg.bits.src1 & AXP_REG_FP) == AXP_REG_FP);
	src2Float = ((entry->ins->decodedReg.bits.src2 & AXP_REG_FP) == AXP_REG_FP);
	destFloat = ((entry->ins->decodedReg.bits.dest & AXP_REG_FP) == AXP_REG_FP);

	src1Reg = (src1Float ? cpu->pf : cpu->pr);
	src2Reg = (src2Float ? cpu->pf : cpu->pr);
	destReg = (destFloat ? cpu->pf : cpu->pr);

	if (AXP_UTL_OPT2)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(
				"AXP_RegistersReady checking registers at pc = 0x%016llx, "
				"opcode = 0x%02x:",
				*((u64 *) &entry->ins->pc),
				(u32) entry->ins->opcode);
		AXP_TraceWrite(
				"\tSrc1 (%c%02u) = %s",
				(src1Float ? 'F' : 'R'),
				entry->ins->aSrc1,
				regStateStr[src1Reg[entry->ins->src1].state],
				(src1Float ? 'F' : 'R'),
				entry->ins->src1);
		AXP_TraceWrite(
				"\tSrc2 (%c%02u) = %s",
				(src2Float ? 'F' : 'R'),
				entry->ins->aSrc2,
				regStateStr[src2Reg[entry->ins->src2].state],
				(src2Float ? 'F' : 'R'),
				entry->ins->src2);
		AXP_TraceWrite(
				"\tDest (%c%02u) = %s (P%c%02u)",
				(destFloat ? 'F' : 'R'),
				entry->ins->aDest,
				regStateStr[destReg[entry->ins->dest].state],
				(destFloat ? 'F' : 'R'),
				entry->ins->dest);
		AXP_TRACE_END();
	}

	retVal = ((src1Reg[entry->ins->src1].state == Valid) &&
			  (src2Reg[entry->ins->src2].state == Valid) &&
			  (destReg[entry->ins->dest].state ==
					  ((entry->ins->dest == AXP_UNMAPPED_REG) ?
							  Valid :
							  PendingUpdate)));

	/*
	 * If the return value is true, then move the contents of the source
	 * registers into the location where the instruction execution expects to
	 * find them.
	 */
	if (src1Float)
		entry->ins->src1v.fp.uq = src1Reg[entry->ins->src1].value;
	else
		entry->ins->src1v.r.uq = src1Reg[entry->ins->src1].value;
	if (src2Float)
		entry->ins->src1v.fp.uq = src2Reg[entry->ins->src1].value;
	else
		entry->ins->src1v.r.uq = src2Reg[entry->ins->src1].value;

	/*
	 * Return the result back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_Execution_Box
 *	This function is called by both the Ebox and Fbox.  The processing loops
 *	for both of them are incredibly similar.  The only real differences are the
 *	determination if a particular pipeline is allowed to execute a particular
 *	instruction, and returning a completed instruction queue entry back to the
 *	pool for a subsequent instruction.
 *
 * Input Parameters:
 *	cpu: 
 *		A pointer to the CPU structure where the instruction queues are
 *		located.
 *	pipeline:
 *		A value indicating the pipeline this function is to process.  This can
 *		be one of the following:
 *			EboxU0
 *			EboxU1
 *			EboxL0
 *			EboxL1
 *			FboxMul
 *			FboxOther
 *	cond:
 *		A pointer to the condition variable associated with the pipeline.
 *	mutex:
 *		A pointer to the mutex variable associated with the pipeline.
 *	regCheckEntry:
 *		A pointer to the function to determine if the registers are ready for
 *		the instruction to be allowed to execute.
 *	returnEntry:
 *		A pointer to the function to return the dequeued entry back to the
 *		pool for a later instruction to be executed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_Execution_Box(
				AXP_21264_CPU *cpu,
				AXP_PIPELINE pipeline,
				AXP_COUNTED_QUEUE *queue,
				pthread_cond_t *cond,
				pthread_mutex_t *mutex,
				void (*returnEntry)(AXP_21264_CPU *, AXP_QUEUE_ENTRY *))
{
	AXP_QUEUE_ENTRY	*entry, *next;
	AXP_INS_STATE	state;
	bool			notMe = true;
	bool			fpEnable;

	/*
	 * While we are not shutting down, we'll continue to try and process
	 * instructions.
	 */
	while (cpu->cpuState != ShuttingDown)
	{

		/*
		 * Before we go checking the queue, lock the Ebox mutex.
		 */
		pthread_mutex_lock(mutex);

		/*
		 * Next we need to do is see if there is nothing to process,
		 * then wait for something to get queued up.
		 */
		while (((AXP_CQUEP_EMPTY(queue) == true) &&
			    (cpu->cpuState != ShuttingDown)) ||
			   (notMe == true))
		{
			pthread_cond_wait(cond, mutex);
			notMe = false;

			if (AXP_UTL_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite("%s signaled.", pipelineStr[pipeline]);
				AXP_TRACE_END();
			}
		}
		notMe = false;

		/*
		 * If we are not shutting down, then we may have something to process.
		 * Let's go looking for trouble.
		 */
		if (cpu->cpuState != ShuttingDown)
		{
			entry = (AXP_QUEUE_ENTRY *) queue->flink;

			/*
			 * Search through the queue of pending integer pipeline
			 * instructions.  If we find one for this cluster, then break out
			 * of this loop.  Otherwise, move on to the next entry in the
			 * queue.  Since the queue eventually points back to the
			 * parent/header, this will exit the loop as well.
			 */
			while ((AXP_COUNTED_QUEUE *) entry != queue)
			{

				/*
				 * Get the next queued entry, because if an instruction was
				 * aborted, but not yet dequeued, we are going to have to get
				 * rid of this entry and not process it.
				 */
				next = (AXP_QUEUE_ENTRY *) entry->header.flink;

				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"%s queue = 0x%016llx, entry = 0x%016llx, "
							"next = 0x%016llx",
							pipelineStr[pipeline],
							queue,
							entry,
							next);
					AXP_TraceWrite(
							"%s checking at "
							"pc = 0x%016llx, "
							"opcode = 0x%02x, "
							"pipeline = %s, "
							"state = %s.",
							pipelineStr[pipeline],
							*((u64 *) &entry->ins->pc),
							(u32) entry->ins->opcode,
							insPipelineStr[entry->pipeline],
							insStateStr[entry->ins->state]);
					AXP_TRACE_END();
				}

				/*
				 * If this instruction is not supposed to be executed by this
				 * pipeline, then move onto the next entry.  Otherwise, dequeue
				 * the entry from the queue and unlock the mutex, so that other
				 * Ebox or Fbox threads can try to execute another queued
				 * instruction, if one exists.
				 */
				if ((entry->pipeline != pipeCond[pipeline][0]) &&
					(entry->pipeline != pipeCond[pipeline][1]) &&
					(entry->pipeline != pipeCond[pipeline][2]))
				{
					if (AXP_UTL_OPT2)
					{
						AXP_TRACE_BEGIN();
						AXP_TraceWrite(
								"%s CANNOT execute "
								"pc = 0x%016llx, "
								"opcode = 0x%02x",
								pipelineStr[pipeline],
								*((u64 *) &entry->ins->pc),
								(u32) entry->ins->opcode);
						AXP_TRACE_END();
					}
				}
				else if (entry->processing == false)
				{
					entry->processing = true;
					break;
				}

				/*
				 * Go to the next entry.
				 */
				entry = next;
			}

			/*
			 * If we did not find an instruction to execute, then go back to
			 * the beginning of the loop.  Since we did not unlock the mutex,
			 * we do not need to lock it now.
			 */
			if ((AXP_COUNTED_QUEUE *) entry == queue)
			{
				notMe = true;

				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"%s has nothing to process.",
							pipelineStr[pipeline]);
					AXP_TRACE_END();
				}

				/*
				 * Before going back to the top of the loop, unlock the mutex.
				 */
				pthread_mutex_unlock(mutex);
				continue;
			}

			/*
			 * Unlock the mutex, we have what we need to process this
			 * instruction.
			 */
			pthread_mutex_unlock(mutex);

			/*
			 * First we need to lock the ROB mutex.  We don't want some
			 * other thread changing the contents while we are looking at
			 * it.  We are looking to see if the instruction was aborted.
			 */
			pthread_mutex_lock(&cpu->robMutex);
			state = entry->ins->state;
			pthread_mutex_unlock(&cpu->robMutex);
			if (state == Aborted)
			{

				/*
				 * The instruction should only be in a Queued state on the
				 * IQ, and it is not.  So, dequeue it and return the
				 * the entry for a subsequent instruction.
				 */
				AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
				entry->processing = false;
				AXP_ReturnIQEntry(cpu, entry);
				continue;
			}

			/*
			 * OK, the instruction was not aborted.  See if the registers are
			 * ready to be used to execute the instruction.  If not, clear the
			 * processing flag and go to the beginning of the loop.
			 */
			else if (AXP_RegistersReady(cpu, entry) == false)
			{
				entry->processing = false;
				continue;
			}

			/*
			 * OK, we have something to execute.  Mark the entry as such and
			 * dequeue it from the queue.  Then, dispatch it to the function
			 * to execute the instruction.
			 */
			if (AXP_UTL_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"%s has something to process at "
						"pc = 0x%016llx, opcode = 0x%02x.",
						pipelineStr[pipeline],
						*((u64 *) &entry->ins->pc),
						(u32) entry->ins->opcode);
				AXP_TRACE_END();
			}
			pthread_mutex_lock(mutex);
			AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
			pthread_mutex_unlock(mutex);

			/*
			 * Before we change the state
			 */
			pthread_mutex_lock(&cpu->robMutex);
			entry->ins->state = Executing;
			pthread_mutex_unlock(&cpu->robMutex);

			/*
			 * If Floating-Point instructions are enabled, then call the
			 * dispatcher to dispatch this instruction to the correct function
			 * to execute the instruction.  Otherwise, set the appropriate
			 * exception value.  To keep the following code simpler, we set the
			 * fpEnable flag to true for all integer instructions.
			 */
			if ((pipeline == FboxMul) || (pipeline == FboxOther))
			{
				pthread_mutex_lock(&cpu->iBoxIPRMutex);
				fpEnable = cpu->pCtx.fpe == 1;
				pthread_mutex_unlock(&cpu->iBoxIPRMutex);
			}
			else
				fpEnable = true;

			if (fpEnable == true)
			{

				/*
				 * Call the dispatcher to dispatch this instruction to the correct
				 * function to execute the instruction.
				 */
				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"%s dispatching instruction, opcode = 0x%02x",
							pipelineStr[pipeline],
							entry->ins->opcode);
					AXP_TRACE_END();
				}

				/*
				 * Now we can call the dispatcher to execute the instruction.
				 */
				AXP_Dispatcher(cpu, entry->ins);
				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"%s dispatched instruction, opcode = 0x%02x",
							pipelineStr[pipeline],
							entry->ins->opcode);
					AXP_TRACE_END();
				}
			}
			else
			{
				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Fbox %s : Floating point instructions are "
							"currently disabled.",
							pipelineStr[pipeline]);
					AXP_TRACE_END();
				}
				pthread_mutex_lock(&cpu->robMutex);
				entry->ins->excRegMask = FloatingDisabledFault;
				entry->ins->state = WaitingRetirement;
				pthread_mutex_unlock(&cpu->robMutex);
			}

			/*
			 * Return the entry back to the pool for future instructions.
			 */
			entry->processing = false;
			(*returnEntry)(cpu, entry);
		}
	}

	/*
	 * Last things last, lock the Ebox mutex.
	 */
	pthread_mutex_unlock(mutex);

	/*
	 * Return back to the caller.
	 */
	return;
}
