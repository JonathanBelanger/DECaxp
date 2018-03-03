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

static char *queueStr[] =
{
	"None",
	"IQ",
	"IQ",
	"",
	"IQ",
	"IQ",
	"",
	"",
	"FQ",
	"FQ"
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
	"WaitingRetirement"
};

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
 *		located, along with a number of other
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
		}
		notMe = false;

		if (AXP_UTL_OPT2)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite(
					"%s signaled an instruction has been put on the %s.",
					pipelineStr[pipeline],
					queueStr[pipeline]);
			AXP_TRACE_END();
		}

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
				if (AXP_UTL_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"%s checking at "
							"pc = 0x%016llx, "
							"opcode = 0x%02x, "
							"pipeline = %s, "
							"state = %s.",
							pipelineStr[pipeline],
							*((u64 *) &entry->ins->pc),
							(u32) entry->ins->opcode,
							insPipelineStr[entry->ins->pipeline],
							insStateStr[entry->ins->state]);
					AXP_TRACE_END();
				}

				/*
				 * Get the next queued entry, because if an instruction was
				 * aborted, but not yet dequeued, we are going to have to get
				 * rid of this entry and not process it.
				 */
				next = (AXP_QUEUE_ENTRY *) entry->header.flink;

				/*
				 * First we need to lock the ROB mutex.  We don't want some
				 * other thread changing the contents while we are looking at
				 * it.  We are looking to see if the instruction was aborted.
				 */
				pthread_mutex_lock(&cpu->robMutex);
				if ((entry->ins->state != Queued) || (cpu->aborting == true))
				{

					/*
					 * The instruction should only be in a Queued state on the
					 * IQ, and it is not.  So, dequeue it and return the
					 * the entry for a subsequent instruction.  If the aborting
					 * flag is set, then the iBox is already aborting things.
					 */
					if (cpu->aborting == false)
					{
						AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
						AXP_ReturnIQEntry(cpu, entry);
					}
					notMe = true;
				}
				pthread_mutex_unlock(&cpu->robMutex);

				/*
				 * We are only looking for entries that can be executed in the
				 * correct Integer cluster and have only been queued for
				 * processing and the registers needed for the instruction are
				 * ready to be used (the source registers need to not be
				 * waiting for previous instruction to write to it).
				 */
				if (((entry->ins->pipeline == pipeCond[pipeline][0]) ||
					 (entry->ins->pipeline == pipeCond[pipeline][1]) ||
					 (entry->ins->pipeline == pipeCond[pipeline][2])) &&
					(notMe == false))
				{
					if (AXP_UTL_OPT2)
					{
						AXP_TRACE_BEGIN();
						AXP_TraceWrite(
								"%s can execute "
								"pc = 0x%016llx, "
								"opcode = 0x%02x",
								pipelineStr[pipeline],
								*((u64 *) &entry->ins->pc),
								(u32) entry->ins->opcode);
						AXP_TRACE_END();
					}
					break;
				}

				/*
				 * Go to the next entry.
				 */
				entry = next;
				notMe = false;
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
				 * Before going back to the top of the loop, unlock the Ebox
				 * mutex.
				 */
				pthread_mutex_unlock(mutex);
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
			AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
			pthread_mutex_lock(&cpu->robMutex);
			entry->ins->state = Executing;
			pthread_mutex_unlock(&cpu->robMutex);

			/*
			 * Now that we have the instruction to be executed, we can unlock
			 * the Ebox mutex and allow the other Ebox threads a chance to
			 * execute.
			 */
			pthread_mutex_unlock(mutex);

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
				entry->ins->excRegMask = FloatingDisabledFault;
				entry->ins->state = WaitingRetirement;
			}

			/*
			 * Return the entry back to the pool for future instructions.
			 */
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
