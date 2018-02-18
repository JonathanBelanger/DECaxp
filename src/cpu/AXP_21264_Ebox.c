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
 *	functionality of the Ebox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		02-Nov-2017	Jonathan D. Belanger
 *	Coded the initialization code.
 *
 *	V01.002		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
 *
 *	V01.003		06-Jan-2018	Jonathan D. Belanger
 *	Continuing to implement the main function for the integer pipelines.  At
 *	some point I'll add a main for each pipeline, each of which calls a common
 *	main with parameters to have the common main execute specific to the
 *	pipeline main that called it.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Ibox_InstructionInfo.h"
#include "AXP_Trace.h"

static char *pipelineStr[] = {"U0", "U1", "L0", "L1"};
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
	"Mul",
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
 * AXP_21264_Ebox_RegisterReady
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
bool AXP_21264_Ebox_RegistersReady(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{
	return ((cpu->prState[entry->ins->src1] == Valid) &&
			(cpu->prState[entry->ins->src2] == Valid) &&
			(cpu->prState[entry->ins->dest] == Valid));
}

/*
 * AXP_21264_Ebox_Compl
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
void AXP_21264_Ebox_Compl(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

	/*
	 * If no exception occurred, then we have the data we need and just need to
	 * store the value that is going to be put into the destination register
	 * when the instruction is retired.  There is nothing specific that needs
	 * to happen to store instructions.
	 *
	 * NOTE: Any exception will be handled in the retirement code.
	 */
	if (instr->excRegMask == NoException)
	{
		switch (instr->opcode)
		{
			case LDBU:
				instr->destv.r.uq = AXP_ZEXT_BYTE(instr->destv.r.uq);
				break;

			case LDW_U:
				instr->destv.r.uq = AXP_ZEXT_WORD(instr->destv.r.uq);
				break;

			case LDL:
			case LDL_L:
				instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);
				break;

			case HW_LD:
				if (instr->len_stall == AXP_HW_LD_LONGWORD)
					instr->destv.r.uq = AXP_SEXT_LONG(instr->destv.r.uq);
				break;

			case STL_C:
			case STQ_C:
				instr->destv.r.uq = 1;
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
	 * We want the Ebox threads to handle their own completion.  The Mbox has
	 * done what is was supposed to and now we need to tell the Ebox that there
	 * is something to retire.
	 */
	pthread_mutex_lock(&cpu->eBoxMutex);
	cpu->eBoxWaitingRetirement = true;
	pthread_cond_signal(&cpu->eBoxCondition);
	pthread_mutex_unlock(&cpu->eBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Ebox_Init
 *	This function is called to initialize the Ebox.  It will set the IPRs
 *	associated with the Ebox to their initial/reset values.
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
bool AXP_21264_Ebox_Init(AXP_21264_CPU *cpu)
{
	bool	retVal = false;
	int		ii;

	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Ebox is initializing");
	AXP_TRACE_END();

	/*
	 * This bit us used when emulating the RC and BC VAX Compatibility
	 * instructions used by VAX-to-Alpha translator software.  ARM 4.12
	 */
	cpu->VAXintrFlag = false;

	/*
	 * Set up the initial register map.  We do not map R31.
	 */
	for (ii = 0; ii < (AXP_MAX_REGISTERS-1); ii++)
	{
		cpu->pr[ii] = 0;
		cpu->prMap[ii].pr = 11;
		cpu->prMap[ii].prevPr = AXP_UNMAPPED_REG;
		cpu->prState[ii] = Valid;
	}


	/*
	 * The above loop initialized the prMap array entries from 0 to 30 to be
	 * mapped to the physical registers also from 0 to 30.  The next two lines
	 * of code initialize the mapping for R31 to be mapped to an invalid
	 * physical register.  This is used to indicate to the code that implements
	 * the Alpha AXP instructions that, as a source register is always a value
	 * of 0, and as a destination register, never updated.  This will greatly
	 * simplify the register (architectural and physical) handling.
	 */
	cpu->prMap[AXP_MAX_REGISTERS-1].pr = AXP_INT_PHYS_REG + 1;
	cpu->prMap[AXP_MAX_REGISTERS-1].prevPr = AXP_INT_PHYS_REG + 1;

	/*
	 * The remaining physical registers need to be put on the free list.
	 */
	cpu->prFlStart = 0;
	cpu->prFlEnd = 0;
	for (ii = AXP_MAX_REGISTERS; ii < AXP_INT_PHYS_REG; ii++)
	{
		cpu->prState[ii] = Free;
		cpu->prFreeList[cpu->prFlEnd++] = ii;
	}

	/*
	 * Initialize the Ebox IPRs.
	 * NOTE: These will get real values from the PALcode.
	 */
	cpu->cc.counter = 0;
	cpu->cc.offset = 0;
	cpu->ccCtl.res_1 = 0;
	cpu->ccCtl.counter = 0;
	cpu->ccCtl.cc_ena = 0;
	cpu->ccCtl.res_2 = 0;
	cpu->va = 0;
	cpu->vaCtl.b_endian = 0;
	cpu->vaCtl.va_48 = 0;
	cpu->vaCtl.va_form_32 = 0;
	cpu->vaCtl.res = 0;
	cpu->vaCtl.vptb = 0;
	cpu->vaForm.form00.res = 0;
	cpu->vaForm.form00.va = 0;
	cpu->vaForm.form00.vptb = 0;

	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Ebox has initialized");
	AXP_TRACE_END();

	return(retVal);
}

/*
 * AXP_21264_EboxU0Main
 *	This is the main function for the Upper 0 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Upper 0 Cluster
 *	of teh Digital Alpha AXP 21264 Central Processing Unit emulation.
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
void *AXP_21264_EboxU0Main(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Ebox U0 is starting");
		AXP_TRACE_END();
	}

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_EboxMain(cpu, AXP_U0_PIPELINE);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_EboxU1Main
 *	This is the main function for the Upper 1 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Upper 1 Cluster
 *	of teh Digital Alpha AXP 21264 Central Processing Unit emulation.
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
void *AXP_21264_EboxU1Main(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Ebox U1 is starting");
		AXP_TRACE_END();
	}

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_EboxMain(cpu, AXP_U1_PIPELINE);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_EboxL0Main
 *	This is the main function for the Lower 0 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Lower 0 Cluster
 *	of the Digital Alpha AXP 21264 Central Processing Unit emulation.
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
void *AXP_21264_EboxL0Main(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Ebox L0 is starting");
		AXP_TRACE_END();
	}

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_EboxMain(cpu, AXP_L0_PIPELINE);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_EboxL1Main
 *	This is the main function for the Lower 1 Cluster of the Ebox (Integer)
 *	pipeline.  It is called by the pthread_create function.  It calls the Ebox
 *	main function to perform the instruction execution for the Lower 1 Cluster
 *	of the Digital Alpha AXP 21264 Central Processing Unit emulation.
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
void *AXP_21264_EboxL1Main(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	if (AXP_CPU_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Ebox L1 is starting");
		AXP_TRACE_END();
	}

	/*
	 * Call the actual main function with the information it needs to be able
	 * to execute instructions for a specific Integer Pipeline.
	 */
	AXP_21264_EboxMain(cpu, AXP_L1_PIPELINE);

	/*
	 * Return back to the caller.
	 */
	return(NULL);
}

/*
 * AXP_21264_EboxMain
 *	This is the main function for all the Integer pipelines.  It is called with
 *	the information needed to perform the processing for a specific pipeline
 *	within the Ebox pipeline  It waits on something needing processing to be
 *	put onto the IQ.  It scans from oldest to newest looking for the next
 *	instruction to be able to be processed by the U0/U1/L0/L1 CPU pipeline.
 *
 *	The Ebox is broken up into 4 pipelines, U0, U1, L0, and L1.  Some
 *	instructions can execute in one or the other, and a few can execute in
 *	either.  Each of these pipelines is a separate thread, but they all share
 *	the same IQ.  In order to be able to handle this, there is a single
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
void AXP_21264_EboxMain(AXP_21264_CPU *cpu, int pipeline)
{
	static AXP_PIPELINE	pipelineCond[AXP_EBOX_PIPELINE_MAX][3] =
	{
		{EboxU0, EboxU0U1, EboxL0L1U0U1},	/* U0 */
		{EboxU1, EboxU0U1, EboxL0L1U0U1},	/* U1 */
		{EboxL0, EboxL0L1, EboxL0L1U0U1},	/* L0 */
		{EboxL1, EboxL0L1, EboxL0L1U0U1}	/* L1 */
	};
	AXP_QUEUE_ENTRY		*entry;
	bool				notMe = true;

	/*
	 * First things first, lock the Ebox mutex.
	 */
	pthread_mutex_lock(&cpu->eBoxMutex);

	/*
	 * While we are not shutting down, we'll continue to try and process
	 * instructions.
	 */
	while (cpu->cpuState != ShuttingDown)
	{

		/*
		 * This may seem odd to put this here, but before we wait for anything,
		 * see if there is an instruction that needs to be retired.  Then make
		 * sure we indicate that the Ebox is no longer waiting to retire an
		 * instruction.
		 */
		if (notMe == false)
		{
			if (AXP_CPU_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"Ebox %s is retiring any completed instructions.",
						pipelineStr[pipeline]);
				AXP_TRACE_END();
			}
			AXP_21264_Ibox_Retire(cpu);
			cpu->eBoxWaitingRetirement = false;
		}

		/*
		 * Next we need to do is see if there is nothing to process,
		 * then wait for something to get queued up.
		 */
		while (((AXP_CQUE_EMPTY(cpu->iq) == true) &&
			    (cpu->eBoxWaitingRetirement == false) &&
			    (cpu->cpuState != ShuttingDown)) ||
			   (notMe == true))
		{
			notMe = false;
			pthread_cond_wait(&cpu->eBoxCondition, &cpu->eBoxMutex);
		}

		if (AXP_CPU_OPT2)
		{
			AXP_TRACE_BEGIN();
			AXP_TraceWrite(
					"Ebox %s may have something to process.",
					pipelineStr[pipeline]);
			AXP_TRACE_END();
		}

		/*
		 * If we are not shutting down, then we may have something to process.
		 * Let's go looking for trouble.
		 */
		if (cpu->cpuState != ShuttingDown)
		{
			entry = (AXP_QUEUE_ENTRY *) cpu->iq.flink;

			/*
			 * Search through the queue of pending integer pipeline
			 * instructions.  If we find one for this cluster, then break out
			 * of this loop.  Otherwise, move on to the next entry in the
			 * queue.  Since the queue eventually points back to the
			 * parent/header, this will exit the loop as well.
			 */
			while ((void *) entry != (void *) &cpu->iq)
			{
				if (AXP_CPU_OPT2)
				{
					AXP_TRACE_BEGIN();
					AXP_TraceWrite(
							"Ebox %s checking at "
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
				 * We are only looking for entries that can be executed in the
				 * correct Integer cluster and have only been queued for
				 * processing and the registers needed for the instruction are
				 * ready to be used (the source registers need to not be
				 * waiting for previous instruction to write to it).
				 */
				if (((entry->ins->pipeline == pipelineCond[pipeline][0]) ||
					 (entry->ins->pipeline == pipelineCond[pipeline][1]) ||
					 (entry->ins->pipeline == pipelineCond[pipeline][2])) &&
					(entry->ins->state == Queued) &&
					(AXP_21264_Ebox_RegistersReady(cpu, entry) == true))
				{
					if (AXP_CPU_OPT2)
					{
						AXP_TRACE_BEGIN();
						AXP_TraceWrite(
								"Ebox %s can execute "
								"pc = 0x%016llx, "
								"opcode = 0x%02x",
								pipelineStr[pipeline],
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
			if ((void *) entry == (void *) &cpu->iq)
			{
				notMe = true;
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
						"Ebox %s has something to process at "
						"pc = 0x%016llx, opcode = 0x%02x.",
						pipelineStr[pipeline],
						*((u64 *) &entry->ins->pc),
						(u32) entry->ins->opcode);
				AXP_TRACE_END();
			}
			entry->ins->state = Executing;
			AXP_RemoveCountedQueue((AXP_CQUE_ENTRY *) entry);
			pthread_mutex_unlock(&cpu->eBoxMutex);

			/*
			 * Call the dispatcher to dispatch this instruction to the correct
			 * function to execute the instruction.
			 */
			if (AXP_CPU_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"Ebox %s dispatching instruction, opcode = 0x%02x",
						pipelineStr[pipeline],
						entry->ins->opcode);
				AXP_TRACE_END();
			}
			AXP_Dispatcher(cpu, entry->ins);
			if (AXP_CPU_OPT2)
			{
				AXP_TRACE_BEGIN();
				AXP_TraceWrite(
						"Ebox %s dispatched instruction, opcode = 0x%02x",
						pipelineStr[pipeline],
						entry->ins->opcode);
				AXP_TRACE_END();
			}

			/*
			 * Return the entry back to the pool for future instructions.
			 */
			AXP_ReturnIQEntry(cpu, entry);
		}

		/*
		 * before going to the top of the loop, lock the Ebox mutex.
		 */
		pthread_mutex_lock(&cpu->eBoxMutex);
	}

	/*
	 * Last things lasst, lock the Ebox mutex.
	 */
	pthread_mutex_unlock(&cpu->eBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}
