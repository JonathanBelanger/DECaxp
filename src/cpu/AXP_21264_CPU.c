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
 *	This source file contains the functions needed to create and initialize
 *	the CPU structure and create each of the threads that implement all the
 *	functionality of a Digital Alpha AXP 21264 CPU.
 *
 *	Revision History:
 *
 *	V01.000		17-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		01-Jan-2018	Jonathan D. Belanger
 *	Added a call to pthread_once to make sure the mutex handling code has been
 *	initialized.
 */
#include "AXP_21264_CPUDefs.h"
#include "AXP_21264_Ibox.h"
#include "AXP_21264_Ibox_Initialize.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Mbox.h"
#include "AXP_21264_Cbox.h"

/*
 * AXP_21264_AllocateCPU
 * 	This function is called to allocate and initialize the CPU structure.  Some
 * 	of the initialization will be performed by the Cbox, but after all the
 * 	threads have been created in here.
 *
 * Input Parameters:
 *	cpuID:
 *		A 64-bit unsigned value to be assigned to this allocated CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	NULL:	An error occurred allocating and initializing the CPU structure.
 *	!NULL:	Normal successful completion.
 */
void *AXP_21264_AllocateCPU(u64 cpuID)
{
	AXP_21264_CPU	*cpu;
	int				pthreadRet;
	int				ii;
	bool			qRet = true;

	cpu = AXP_Allocate_Block(AXP_21264_CPU_BLK);
	if (cpu != NULL)
	{

		/*
		 * First things first.  Create all the mutexes and condition variables.
		 *
		 * Let's start with all the mutexes.
		 */
		pthreadRet = pthread_mutex_init(&cpu->cpuMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->iBoxMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->iBoxIPRMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->robMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->iCacheMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->itbMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->eBoxMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->eBoxIPRMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->fBoxMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->mBoxMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->mBoxIPRMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->dCacheMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->dtagMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->lqMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->sqMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->dtbMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->cBoxInterfaceMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->cBoxIPRMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&cpu->bCacheMutex, NULL);

		/*
		 * Let's create the condition variables.
		 */
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->cpuCond, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->iBoxCondition, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->eBoxCondition, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->fBoxCondition, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->mBoxCondition, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_cond_init(&cpu->cBoxInterfaceCond, NULL);
		else
			qRet = false;

		/*
		 * Let's initialize the counted queues (these have both a mutex and a
		 * condition variable.  We also have to initialize the preallocated
		 * queue entries.
		 */
		if (qRet == true)
			AXP_InitCountedQueue(&cpu->iq, AXP_IQ_LEN);
		if (qRet == true)
		{
			cpu->iqEFlStart = cpu->iqEFlEnd = 0;
			for(ii = 0; ii < AXP_IQ_LEN; ii++)
			{
				AXP_INIT_CQENTRY(cpu->iqEntries[ii].header, cpu->iq);
				cpu->iqEntries[ii].ins = NULL;
				cpu->iqEntries[ii].index = ii;
				cpu->iqEntries[ii].processing = false;
				cpu->iqEFreelist[ii] = ii;
			}
		}
		if (qRet == true)
			AXP_InitCountedQueue(&cpu->fq, AXP_IQ_LEN);
		if (qRet == true)
		{
			cpu->fqEFlStart = cpu->fqEFlEnd = 0;
			for(ii = 0; ii < AXP_FQ_LEN; ii++)
			{
				AXP_INIT_CQENTRY(cpu->fqEntries[ii].header, cpu->fq);
				cpu->fqEntries[ii].ins = NULL;
				cpu->fqEntries[ii].index = ii;
				cpu->fqEntries[ii].processing = false;
				cpu->fqEFreelist[ii] = ii;
			}
		}

		/*
		 * Go initialize the register map.
		 */
		AXP_21264_Ibox_ResetRegMap(cpu);

		/*
		 * Pull some configuration items out of the configuration and
		 * initialize the appropriate CPU fields.
		 */
		qRet = AXP_ConfigGet_CPUType(&cpu->majorType, &cpu->minorType);

		/*
		 * Get the CPU-ID and store it in the WHAMI IPR/
		 */
		cpu->whami = cpuID;

		/*
		 * At this point, we lock the CPU mutex, to hold back any of the CPU
		 * initialization that will occur when the iBox, mBox, dBox, eBoxes,
		 * fBoxes, and cBox threads are created.  We'll unlock it after the
		 * system has done its initialization and is ready for the CPUs.
		 */
		if (qRet == true)
			pthreadRet = pthread_mutex_lock(cpu->cpuMutex);

		/*
		 * At this point everything should be initialize.  Time to create all
		 * the threads.
		 */
		if ((pthreadRet == 0) || (qRet == true))
		{
			pthreadRet = pthread_create(
							&cpu->iBoxThreadID,
							NULL,
							AXP_21264_IboxMain,
							cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->eBoxU0ThreadID,
								NULL,
								AXP_21264_EboxU0Main,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->eBoxU1ThreadID,
								NULL,
								AXP_21264_EboxU1Main,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->eBoxL0ThreadID,
								NULL,
								AXP_21264_EboxL0Main,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->eBoxL1ThreadID,
								NULL,
								AXP_21264_EboxL1Main,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->fBoxMulThreadID,
								NULL,
								AXP_21264_FboxMulMain,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->fBoxOthThreadID,
								NULL,
								AXP_21264_FboxOthMain,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->mBoxThreadID,
								NULL,
								AXP_21264_MboxMain,
								cpu);
			if (pthreadRet == 0)
				pthreadRet = pthread_create(
								&cpu->cBoxThreadID,
								NULL,
								AXP_21264_CboxMain,
								cpu);
		}

		/*
		 * If anything happened in error, then deallocate the CPU block just
		 * allocated and return NULL to the caller.
		 */
		if ((cpu != NULL) && ((pthreadRet != 0) || (qRet != true)))
		{
			AXP_Deallocate_Block((AXP_BLOCK_DSC *) cpu);
			cpu = NULL;
		}
	}

	/*
	 * Return the address of the allocated CPU structure to the caller.
	 */
	return((void *) cpu);
}

/*
 * AXP_21264_Save_WHAMI
 *	This function is called by the System after creating the CPU structure.  It
 *	stores the identifier for the specified CPU into the specified location.
 *
 * Input Parameters:
 *	cpuPtr:
 *		A void pointer to the CPU structure.  This will be recast so that the
 *		System does not have to have knowledge of the specifics of the CPU.
 *
 * Output Parameters:
 *	cpuID:
 *		A pointer to an unsigned 64-bit value, the size of the WHAMI register,
 *		to receive the ID assigned to this particular CPU.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Save_WHAMI(void *cpuPtr, u64 *cpuID)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) cpuPtr;

	/*
	 * Set the CPU ID for this CPU into the output parameter.
	 */
	*cpuID = cpu->whami;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Save_SystemInterfaces
 *	This function is called by the System after creating the CPU structure.  It
 *	stores the information required for the CPU to be able to send to the
 *	system and the system to the CPU.
 *
 * Input Parameters:
 *	cpuPtr:
 *		A void pointer to the CPU structure.  This will be recast so that the
 *		System does not have to have knowledge of the specifics of the CPU.
 *	sysMutex:
 *		A pointer to the System Interface mutex to be stored into the CPU.
 *	sysCond:
 *		A pointer to the System Interface condition variable to be stored into
 *		the CPU.
 *	rq:
 *		A pointer to the System Interface request queue, where requests from
 *		the CPU will be queued up for processing by the System.
 *	rqStart:
 *		A pointer to the System Interface request queue starting entry.  This
 *		is the location where the System will remove entries for processing.
 *	rqEnd:
 *		A pointer to the System Interface request queue last entry.  This is
 *		incremented to the next location where the CPU will add entries to be
 *		processed by the System.
 *
 * Output Parameters:
 * 	cpuMutex:
 *		A pointer to the address where the CPU's System Interface mutex is to
 *		be stored.
 * 	cpuCond:
 *		A pointer to the address where the CPU's System Interface condition
 *		variable is to be stored.
 *	pq:
 *		A pointer to the address, of type void in this function, of the CPU's
 *		Probe Queue (System Interface), where the System queues up requests for
 *		the CPU to process.
 *	pqTop:
 *		A pointer to an unsigned 8-bit value where the CPU can store the top
 *		entry to be processed by the CPU.  The CPU removes entries from this
 *		location within the PQ for processing.
 *	pqBottm:
 *		A pointer to an unsigned 8-bit value where the CPU can store the bottom
 *		entry to be processed by the CPU.  The System adds entries to  this
 *		location within the PQ.
 *	irq_H:
 *		A pointer to an unsigned 8-bit value where the System can store the
 *		interrupts for the CPU to process.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Save_SystemInterfaces(
						void *cpuPtr,
						pthread_mutex_t **cpuMutex,
						pthread_cond_t **cpuCond,
						void **pq,
						u8 *pqTop,
						u8 *pqBottom,
						u8 *irq_H,
						pthread_mutex_t *sysMutex,
						pthread_cond_t *sysCond,
						void *rq,
						u32	*rqStart,
						u32 *rqEnd)
{
	AXP_21264_CPU		*cpu = (AXP_21264_CPU *) cpuPtr;

	/*
	 * First, set the data needed for the CPU to be able to communicate with
	 * the System into the CPU structure.
	 */
	cpu->system.cond = sysCond;
	cpu->system.mutex = sysMutex;
	cpu->system.rq = (AXP_21264_RQ_ENTRY *) rq;
	cpu->system.rqStart = rqStart;
	cpu->system.rqEnd = rqEnd;

	/*
	 * Finally, set the data needed for the System to be able to communicate
	 * with the CPU into the output parameters.
	 */
	*cpuMutex = &cpu->cBoxInterfaceMutex;
	*cpuCond = &cpu->cBoxInterfaceCond;
	*pq = (void *) cpu->pq;
	*pqTop = &cpu->pqTop;
	*pqBottom = &cpu->pqBottom;
	*irq_H = &cpu->irqH;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Unlock_CPU
 *	This function is called to unlock the CPU mutex.  It is locked prior to the
 *	CPU threads getting created, so that the BiST will not execute and an
 *	initial load requested until the system is ready.
 *
 * Input Parameters:
 *	cpuPtr:
 *		A void pointer to the CPU structure.  This will be recast so that the
 *		System does not have to have knowledge of the specifics of the CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Unlock_CPU(void *cpuPtr)
{
	AXP_21264_CPU		*cpu = (AXP_21264_CPU *) cpuPtr;

	/*
	 * Unlock the CPU mutex.  This will allow all the CPU threads to being
	 * there executions.
	 */
	pthread_mutex_unlock(cpu->cpuMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}
