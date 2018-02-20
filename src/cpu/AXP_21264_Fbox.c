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
 *
 *	V01.003		20-Jan-2018	Jonathan D. Belanger
 *	Added a check to make sure the floating-point instructions are enabled.  If
 *	so, call the dispatcher.  If not, set an exception and indicate that the
 *	instruction is ready for retirement.
*/
#include "AXP_Configure.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Ibox.h"
#include "AXP_21264_Ibox_InstructionInfo.h"
#include "AXP_Trace.h"

static char *pipelineStr[] = {"Multiply", "FP Other"};
#define AXP_PIPE_STR(pipe)	(pipe == FboxMul ? pipelineStr[0] : pipelineStr[1])
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
	"FP Other"
};
static char *insStateStr[] =
{
	"Retired",
	"Queued",
	"Executing",
	"WaitingRetirement"
};
static char *regStateStr[] =
{
	"Free",
	"Pending Update",
	"Valid"
};

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
	if (AXP_CPU_OPT2)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite(
				"Fbox Checking registers at pc = 0x%016llx, opcode = 0x%02x:",
				*((u64 *) &entry->ins->pc),
				(u32) entry->ins->opcode);
		AXP_TraceWrite(
				"\tSrc1(F%02u) = %s",
				entry->ins->aSrc1,
				regStateStr[cpu->pfState[entry->ins->aSrc1]]);
		AXP_TraceWrite(
				"\tSrc2(F%02u) = %s",
				entry->ins->aSrc2,
				regStateStr[cpu->pfState[entry->ins->aSrc2]]);
		AXP_TraceWrite(
				"\tDest(F%02u) = %s",
				entry->ins->aDest,
				regStateStr[cpu->pfState[entry->ins->aDest]]);
		AXP_TRACE_END();
	}
	return ((cpu->pfState[entry->ins->src1] == Valid) &&
			(cpu->pfState[entry->ins->src2] == Valid) &&
			((cpu->pfState[entry->ins->dest] == Valid) ||
			 (cpu->pfState[entry->ins->dest] == PendingUpdate)));
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
	 * We want the Fbox threads to handle their own completion.  The Mbox has
	 * done what is was supposed to and now we need to tell the Fbox that there
	 * is something to retire.
	 */
	pthread_mutex_lock(&cpu->fBoxMutex);
	cpu->fBoxWaitingRetirement = true;
	pthread_cond_signal(&cpu->fBoxCondition);
	pthread_mutex_unlock(&cpu->fBoxMutex);

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

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Fbox is initializing");
		AXP_TRACE_END();
	}

	/*
	 * Initialize the one Fbox IPR.
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
	 * Initialize the instruction queue for the Fbox.
	 */
	AXP_INIT_CQUE(cpu->fq, AXP_FQ_LEN);

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

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Fbox has initialized");
		AXP_TRACE_END();
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
 *	Multiply Cluster of the Digital Alpha AXP 21264 Central Processing Unit
 *	emulation.
 *
 *	NOTE:	In the real Alpha AXP 21264 CPU, only multiplication is performed
 *			in this pipeline.  In the emulator, I have both multiply and
 *			divide executing in this pipeline.  If testing determines that
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

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Fbox Multiply is starting");
		AXP_TRACE_END();
	}

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
 *	Multiply Cluster of the Digital Alpha AXP 21264 Central Processing Unit
 *	emulation.
 *
 *	NOTE:	In the real Alpha AXP 21264 CPU, division is not performed in this
 *			pipeline.  In the emulator, I have both multiply and divide
 *			executing in the Multiply pipeline.  If testing determines that
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

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Fbox Other is starting");
		AXP_TRACE_END();
	}

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_FboxMain(cpu, FboxOther);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_FboxMain
 *	This is the main function for all the Floating Point pipelines.  It is
 *	called with the information needed to perform the processing for a specific
 *	pipeline within the Fbox pipeline  It waits on something needing processing
 *	to be put onto the FQ.  It scans from oldest to newest looking for the next
 *	instruction to be able to be processed by the Multiply/(Add/Divide/Square
 *	Root - Other) CPU pipeline.
 *
 *	The Fbox is broken up into 2 pipelines, Multiply and Add/Divide/Square
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
	bool				notMe = true;
	bool				notFirstTime = false;
	bool				fpEnabled;

	/*
	 * While we are not shutting down, we'll continue to try and process
	 * instructions.
	 */
	while (cpu->cpuState != ShuttingDown)
	{

		/*
		 * This may seem odd to put this here, but before we wait for anything,
		 * see if there is an instruction that needs to be retired.  Then make
		 * sure we indicate that the Fbox is no longer waiting to retire an
		 * instruction.
		 */
		if (notFirstTime)
		{
			if (AXP_CPU_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"Fbox %s is retiring any completed instructions.",
						AXP_PIPE_STR(pipeline));
				AXP_TRACE_END();
			}
			AXP_21264_Ibox_Retire(cpu);
		}
		else
			notFirstTime = true;
		cpu->fBoxWaitingRetirement = false;

		/*
		 * Before we go checking the queue, lock the Fbox mutex.
		 */
		pthread_mutex_lock(&cpu->fBoxMutex);

		/*
		 * Next we need to do is see if there is nothing to process,
		 * then wait for something to get queued up.
		 */
		while (((AXP_CQUE_EMPTY(cpu->fq) == true) &&
			    (cpu->fBoxWaitingRetirement == false) &&
			    (cpu->cpuState != ShuttingDown)) ||
			   (notMe == true))
		{
			notMe = false;
			pthread_cond_wait(&cpu->fBoxCondition, &cpu->fBoxMutex);
		}

		if (AXP_CPU_OPT2)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite(
					"Fbox %s may have something to process.",
					AXP_PIPE_STR(pipeline));
			AXP_TRACE_END();
		}

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
				if (AXP_CPU_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Fbox %s checking at "
							"pc = 0x%016llx, "
							"opcode = 0x%02x, "
							"pipeline = %s, "
							"state = %s.",
							AXP_PIPE_STR(pipeline),
							*((u64 *) &entry->ins->pc),
							(u32) entry->ins->opcode,
							insPipelineStr[entry->ins->pipeline],
							insStateStr[entry->ins->state]);
					AXP_TRACE_END();
				}

				/*
				 * We are only looking for entries that can be executed in the
				 * correct Floating Point cluster and have only been queued for
				 * processing and the registers needed for the instruction are
				 * ready to be used (the source registers need to not be
				 * waiting for previous instruction to write to it).
				 */
				if ((entry->ins->pipeline == pipeline) &&
					(entry->ins->state == Queued) &&
					(AXP_21264_Fbox_RegistersReady(cpu, entry) == true))
				{
					if (AXP_CPU_OPT2)
					{
						AXP_TRACE_BEGIN();
						AXP_TraceWrite(
								"Fbox %s can execute "
								"pc = 0x%016llx, "
								"opcode = 0x%02x",
								AXP_PIPE_STR(pipeline),
								*((u64 *) &entry->ins->pc),
								(u32) entry->ins->opcode);
						AXP_TRACE_END();
					}
					break;
				}
				else
					entry = (AXP_QUEUE_ENTRY *) entry->header.flink;
			}

			/*
			 * If we did not find an instruction to execute, then go back to
			 * the beginning of the loop.  Since we did not unlock the mutex,
			 * we do not need to lock it now.
			 */
			if ((void *) entry == (void *) &cpu->fq)
			{
				notMe = true;

				/*
				 * Before going back to the top of the loop, unlock the Ebox
				 * mutex.
				 */
				pthread_mutex_unlock(&cpu->fBoxMutex);
				continue;
			}

			/*
			 * OK, we have something to execute.  Mark the entry as such and
			 * dequeue it from the queue.  Then, dispatch it to the function
			 * to execute the instruction.
			 */
			if (AXP_CPU_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"Fbox %s has something to process at "
						"pc = 0x%016llx, opcode = 0x%02x.",
						AXP_PIPE_STR(pipeline),
						*((u64 *) &entry->ins->pc),
						(u32) entry->ins->opcode);
				AXP_TRACE_END();
			}

			/*
			 * OK, we have something to execute.  Mark the entry as such and
			 * dequeue it from the queue.  Then, dispatch it to the function
			 * to execute the instruction.
			 */
			entry->ins->state = Executing;
			AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);

			/*
			 * Now that we have the instruction to be executed, we can unlock
			 * the Fbox mutex and allow the other Fbox thread a chance to
			 * execute.
			 */
			pthread_mutex_unlock(&cpu->fBoxMutex);

			/*
			 * If Floating-Point instructions are enabled, then call the
			 * dispatcher to dispatch this instruction to the correct function
			 * to execute the instruction.  Otherwise, set the appropriate
			 * exception value.
			 */
			pthread_mutex_lock(&cpu->iBoxIPRMutex);
			fpEnabled = cpu->pCtx.fpe == 1;
			pthread_mutex_unlock(&cpu->iBoxIPRMutex);
			if (fpEnabled)
			{
				if (AXP_CPU_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Fbox %s dispatching instruction, opcode = 0x%02x",
							AXP_PIPE_STR(pipeline),
							entry->ins->opcode);
					AXP_TRACE_END();
				}
				AXP_Dispatcher(cpu, entry->ins);
				if (AXP_CPU_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Fbox %s dispatched instruction, opcode = 0x%02x",
							AXP_PIPE_STR(pipeline),
							entry->ins->opcode);
					AXP_TRACE_END();
				}
			}
			else
			{
				if (AXP_CPU_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Fbox %s : Floating point instructions are "
							"currently disabled.",
							AXP_PIPE_STR(pipeline));
					AXP_TRACE_END();
				}
				entry->ins->excRegMask = FloatingDisabledFault;
				entry->ins->state = WaitingRetirement;
			}

			/*
			 * Return the entry back to the pool for future instructions.
			 */
			AXP_ReturnFQEntry(cpu, entry);
		}
	}

	/*
	 * Last things last, lock the Fbox mutex.
	 */
	pthread_mutex_unlock(&cpu->fBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}
