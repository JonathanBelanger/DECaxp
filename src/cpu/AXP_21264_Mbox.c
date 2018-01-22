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
 *	functionality of the Mbox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		16-Dec-2017	Jonathan D. Belanger
 *	Updating extensively with threading, processing, and interfacing functions.
 *
 *	V01.002		20-Dec-2017	Jonathan D. Belanger
 *	The Mbox is effectively done.  The interfaces to the Ibox, Cbox, and Cache
 *	have all be written.  there is one 'TO-DO' item that will need to be
 *	resolved round whether the Cbox completing a IOWB versus a Dcache Fill
 *	needs to be handled differently.
 *
 *	V01.003		01-Jan-2018	Jonathan D. Belanger
 *	Changed the way instructions are completed when they need to utilize the
 *	Mbox.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Mbox.h"
#include "AXP_21264_Cache.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Cbox.h"

/*
 * AXP_21264_Mbox_GetLQSlot
 *	This function is called to get the next available Load slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The LQ slot index is incremented, if there is room for another load
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Load instruction.  If there are no
 *	slots available a value of the size of the LoadQueue will be returned.
 */
u32 AXP_21264_Mbox_GetLQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * Let's make sure the index does not get updated while we are accessing
	 * it.
	 */
	pthread_mutex_lock(&cpu->lqMutex);

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As loads are completed, the index will be reduced.
	 */
	if (cpu->lqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->lqNext++;
		cpu->lq[retVal].state = Assigned;
	}

	/*
	 * Don't forget to unlock the LQ mutex.
	 */
	pthread_mutex_unlock(&cpu->lqMutex);

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_PutLQSlot
 *	This function is called to put the current Load slot back into the
 *	available pool.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the entry too  be put back.
 *
 * Output Parameters:
 *	cpu:
 *		The LQ slot index is decremented to  the next available slot.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_PutLQSlot(AXP_21264_CPU *cpu, u32 entry)
{
	bool done = false;

	/*
	 * First, lock the LQ mutex so that we are not interrupted while we play
	 * with the LQ entries and index.
	 */
	pthread_mutex_lock(&cpu->lqMutex);

	/*
	 * OK, we can set the current entry to not being in use.
	 */
	cpu->lq[entry].state = QNotInUse;

	/*
	 * OK, we loop starting at the first available entry end decrementing this
	 * index if the previous entry is also not in use.
	 */
	while (done == false)
	{
		if (cpu->lqNext > 0)
		{
			if (cpu->lq[cpu->lqNext - 1].state == QNotInUse)
				cpu->lqNext--;
			else
				done = true;
		}
		else
			done = true;
	}

	/*
	 * OK, we are done.  Unlock the mutex and return back to the caller.
	 */
	pthread_mutex_unlock(&cpu->lqMutex);
	return;
}

/*
 * AXP_21264_Mbox_ReadMem
 *	This function is called to queue up a read from Dcache based on a virtual
 *	address, size of the data to be read and the instruction that is queued up
 *	to be completed in order.  This function works with the AXP_Mbox_WriteMem
 *	function to ensure correct Alpha memory reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the read is complete, the
 *		value from memory is store in the instr->destv location and the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Load Queue (LQ) where this read entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be read is located.
 *
 * Output Parameters:
 *	instr:
 *		A pointer to the decoded instruction.  It is updated with the value
 *		read from the Dcache.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_ReadMem(AXP_21264_CPU *cpu,
							AXP_INSTRUCTION *instr,
							u32 slot,
							u64 virtAddr)
{

	/*
	 * Let's make sure the Mbox does not try to update or access the LQ while
	 * we are accessing it.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);

	/*
	 * Store the information in the
	 */
	cpu->lq[slot].virtAddress = virtAddr;
	cpu->lq[slot].instr = instr;
	cpu->lq[slot].instr->excRegMask = NoException;
	cpu->lq[slot].state = Initial;

	/*
	 * Notify the Mbox that there is something to process and unlock the Mbox
	 * mutex so it can start performing the processing we just requested.
	 */
	pthread_cond_signal(&cpu->mBoxCondition);
	pthread_mutex_unlock(&cpu->mBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
 }

/*
 * AXP_21264_Mbox_GetSQSlot
 *	This function is called to get the next available Store slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The SQ slot index is incremented, if there is room for another store
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Store instruction.  If there are no
 *	slots available a value of the size of the StoreQueue will be returned.
 */
u32 AXP_21264_Mbox_GetSQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * Let's make sure the index does not get updated while we are accessing
	 * it.
	 */
	pthread_mutex_lock(&cpu->sqMutex);

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As stores are completed, the index will be reduced.
	 */
	if (cpu->sqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->sqNext++;
		cpu->sq[retVal].state = Assigned;
	}

	/*
	 * Don't forget to unlock the SQ mutex.
	 */
	pthread_mutex_unlock(&cpu->sqMutex);

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_PutSQSlot
 *	This function is called to put the current Store slot back into the
 *	available pool.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the entry too  be put back.
 *
 * Output Parameters:
 *	cpu:
 *		The SQ slot index is decremented to the next available slot.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_PutSQSlot(AXP_21264_CPU *cpu, u32 entry)
{
	bool done = false;

	/*
	 * First, lock the SQ mutex so that we are not interrupted while we play
	 * with the SQ entries and index.
	 */
	pthread_mutex_lock(&cpu->sqMutex);

	/*
	 * OK, we can set the current entry to not being in use.
	 */
	cpu->sq[entry].state = QNotInUse;

	/*
	 * OK, we loop starting at the first available entry end decrementing this
	 * index if the previous entry is also not in use.
	 */
	while (done == false)
	{
		if (cpu->sqNext > 0)
		{
			if (cpu->sq[cpu->sqNext - 1].state == QNotInUse)
				cpu->sqNext--;
			else
				done = true;
		}
		else
			done = true;
	}

	/*
	 * OK, we are done.  Unlock the mutex and return back to the caller.
	 */
	pthread_mutex_unlock(&cpu->sqMutex);
	return;
}

/*
 * AXP_21264_Mbox_WriteMem
 *	This function is called to queue up a write to the Dcache based on a
 *	virtual address, size of the data to be written, the value of the data and
 *	the instruction that is queued up to be completed in order.  This function
 *	works with the AXP_Mbox_ReadMem function to ensure correct Alpha memory
 *	reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the write is completed, the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Store Queue (SQ) where this write entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be written is located.
 *	value:
 *		The value to be be written to the Dcache (and ultimately memory).
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_WriteMem(AXP_21264_CPU *cpu,
							 AXP_INSTRUCTION *instr,
							 u32 slot,
							 u64 virtAddr,
							 u64 value)
{

	/*
	 * Let's make sure the Mbox does not try to update or access the SQ while
	 * we are accessing it.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);

	/*
	 * Store the information in the
	 */
	cpu->sq[slot].value = value;
	cpu->sq[slot].virtAddress = virtAddr;
	cpu->sq[slot].instr = instr;
	cpu->sq[slot].instr->excRegMask = NoException;
	cpu->sq[slot].state = Initial;

	/*
	 * Notify the Mbox that there is something to process and unlock the Mbox
	 * mutex so it can start performing the processing we just requested.
	 */
	pthread_cond_signal(&cpu->mBoxCondition);
	pthread_mutex_unlock(&cpu->mBoxMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_CboxCompl
 *	This  function is called by the Cbox because a request from the Mbox, MAF
 *	or IOWB, has been completed.  Determine which queue entry, in the LQ or SQ,
 *	needs to be completed and signal the Mbox so it can perform the completion.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the signed index+1 into the LQ/SQ.  A value < 0 is for the
 *		SQ, otherwise the LQ.
 *	data:
 *		A pointer to a buffer containing data returned from an Load to the I/O
 *		Address Space.  Data that is supposed to go into the Dcache will have
 *		already been copied there by the Cbox prior to calling this function.
 *	dataLen:
 *		A value indicating the length of the data returned in the 'data'
 *		parameter.
 *	error:
 *		A value indicating that an error was returned.  The buffer returned on
 *		a load is set to an all ones bit pattern.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 *
 * NOTE:	When we are called, the Mbox mutex is NOT locked.  We need to lock
 *			it before we do anything, and unlock it when we are done.
 *
 */
void AXP_21264_Mbox_CboxCompl(
					AXP_21264_CPU *cpu,
					i8 lqSqEntry,
					u8 *data,
					int dataLen,
					bool error)
{
	bool			signalCond = false;
	u8				entry = abs(lqSqEntry) - 1;
	u64				retData;

	/*
	 * TODO: What are we supposed to do with the 'error' parameter.
	 */

	/*
	 * Before we do anything, if data was returned, this is done for I/O reads,
	 * then convert the data supplied on the call to a 64-bit register format.
	 * This will make the code below a bit simpler.  NOTE: The dataLen
	 * parameter can only be, 0, 1, 2, 4, 8.
	 */
	switch (dataLen)
	{
		case 0:
			retData = 0;
			break;

		case 1:
			retData = *((u64 *) data) & AXP_LOW_BYTE;
			break;

		case 2:
			retData = *((u64 *) data) & AXP_LOW_WORD;
			break;

		case 4:
			retData = *((u64 *) data) & AXP_LOW_LONG;
			break;

		case 8:
			retData = *((u64 *) data);
			break;

		default:
			printf(
				"\n%%DECAXP-F-UNEXLEN, Unexpected data length returned from "
				"Cbox, %d at %s, line %d\n",
				dataLen,
				__FILE__,
				__LINE__);
			break;
	}

	/*
	 * Now, lock the Mbox mutex.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);

	/*
	 * Determine which Mbox queue we are interested in.  For SQ, the lqSqEntry
	 * value on the call will be less than zero.  For LQ, the lqSqEntry value
	 * on the call will be greater than one.
	 *
	 * NOTE:	One (1) was added to the real entry value before being signed.
	 * 			We did this because there is no such thing as -0, which is a
	 * 			legitimate value.
	 */
	if (lqSqEntry <= 0)
	{
		AXP_MBOX_QUEUE	*sqEntry = &cpu->sq[entry];

		/*
		 * We need to signal the Mbox if the entry was in a pending Cbox state.
		 * If so, we need to change the state to Write Pending.
		 */
		signalCond = sqEntry->state == CboxPending;
		if (signalCond == true)
			sqEntry->state = SQWritePending;
	}
	else
	{
		AXP_MBOX_QUEUE	*lqEntry = &cpu->lq[entry];

		/*
		 * We need to signal the Mbox if the entry was in a pending Cbox state.
		 * If so, we need to change the state to Read Pending.
		 */
		signalCond = lqEntry->state == CboxPending;
		if (signalCond == true)
			lqEntry->state = LQReadPending;

		if (lqEntry->IOflag == true)
			lqEntry->IOdata = retData;
	}

	/*
	 * If we changed one of the SQ/LQ states, then signal the Mbox that there
	 * may be something to process.
	 */
	if (signalCond == true)
		pthread_cond_signal(&cpu->mBoxCondition);

	/*
	 * Unlock the Mbox mutex and get out of here.
	 */
	pthread_mutex_unlock(&cpu->mBoxMutex);
	return;
}

/*
 * AXP_21264_Mbox_TryCaches
 *	This function is called to see if what we are looking to do with the cache
 *	can be done.  It checks the Dcache state and if acceptable, does the things
 *	needed for the Ibox to retire the associated instruction.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the LQ.
 *
 * Output Parameters:
 *	lqEntry:
 *		A pointer to the entry which has been set up so that the instruction
 *		can be retired (either when the data arrives or now).
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_TryCaches(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE	*lqEntry = &cpu->lq[entry];
	AXP_DCACHE_LOC	dcacheLoc;
	u32				cacheStatus;
	u8				nextOlderStore;
	int				ii;
	bool			olderStore = false;

	/*
	 * Loop through each of the SQ entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{

		/*
		 * Look deeper at the SQ entries that are in process.
		 */
		if ((cpu->sq[ii].state == Initial) ||
			(cpu->sq[ii].state == CboxPending) ||
			(cpu->sq[ii].state == SQWritePending) ||
			(cpu->sq[ii].state == SQComplete))
		{

			/*
			 * If the address for the store matches that for the load and
			 * the length for the store is greater than or equal to the load
			 * and the store is older than the load,, then we can source from
			 * the store (instead of the Dcache and/or Bcache).
			 */
			if ((lqEntry->virtAddress == cpu->sq[ii].virtAddress) &&
				(lqEntry->len <= cpu->sq[ii].len) &&
				(lqEntry->instr->uniqueID < cpu->sq[ii].instr->uniqueID))
			{

				/*
				 * OK, there may have been multiple stores to the same address,
				 * so get the most recent older store.  Otherwise, if this is
				 * the first one, set the index and the flag.
				 */
				if ((olderStore == true) &&
					(cpu->sq[ii].instr->uniqueID >
					 cpu->sq[nextOlderStore].instr->uniqueID))
					nextOlderStore = ii;
				else if (olderStore == false)
				{
					nextOlderStore = ii;
					olderStore = true;
				}
			}
		}
	}

	/*
	 * IF we didn't find an older store, we need to see if the information we
	 * need is in the Dcache or Bcache and in the proper state.
	 */
	if (olderStore == false)
	{
		bool	DcHit = false;

		/*
		 * Get the status for the Dcache for the current Va/PA pair.
		 */
		lqEntry->instr->excRegMask = AXP_Dcache_Status(
									cpu,
									lqEntry->virtAddress,
									lqEntry->physAddress,
									lqEntry->len,
									true,
									&cacheStatus,
									&dcacheLoc,
									false);

		/*
		 * If we did not get a DcHit (See HRM Table 4-1), then we need to check
		 * the Bcache.
		 */
		if (AXP_CACHE_MISS(cacheStatus) == true)
		{

			/*
			 * Get the status for the Bcache for the current PA.
			 */
			cacheStatus = AXP_21264_Bcache_Status(
									cpu,
									lqEntry->physAddress);

			/*
			 * Missed both Caches (Dcache and Bcache).  Put an entry in the
			 * Missed Address File (MAF) for the Cbox to process.  There is
			 * nothing else for us to do here.
			 */
			if (AXP_CACHE_MISS(cacheStatus) == true)
			{
				lqEntry->state = CboxPending;
				AXP_21264_Add_MAF(
					cpu,
					LDx,
					lqEntry->physAddress,
					(entry + 1),	/* We need to take zero out of play */
					lqEntry->len,
					false);
			}

			/*
			 * Hit in the Bcache, move the data to the Dcache, which may
			 * require evicting the current entry.
			 */
			else
			{

				/*
				 * We found what we were looking for in the Bcache.  We may
				 * need to evict the current block (possibly the same index and
				 * set, but not the same physical tag).
				 */
				AXP_CopyBcacheToDcache(
								cpu,
								&dcacheLoc,
								lqEntry->physAddress);
				DcHit = true;
			}
		}
		else
			DcHit = true;

		/*
		 * If we hit in the Dcache, them read the data out of it.
		 */
		if (DcHit == true)
		{
			lqEntry->instr->destv.r.uq = 0;
			(void) AXP_DcacheRead(
						cpu,
						lqEntry->virtAddress,
						lqEntry->physAddress,
						lqEntry->len,
						&lqEntry->instr->destv.r,
						NULL);
			lqEntry->state = LQComplete;
		}
	}

	/*
	 * We found what we were looking for in a store that is older than the load
	 * currently being processed.
	 */
	else
	{
		lqEntry->instr->destv.r.uq = 0;
		switch (lqEntry->len)
		{
			case 1:
				lqEntry->instr->destv.r.ub = *((u8 *) &cpu->sq[nextOlderStore].value);
				break;

			case 2:
				lqEntry->instr->destv.r.uw = *((u16 *) cpu->sq[nextOlderStore].value);
				break;

			case 4:
				lqEntry->instr->destv.r.ul = *((u32 *) cpu->sq[nextOlderStore].value);
				break;

			case 8:
				lqEntry->instr->destv.r.uq = *((u64 *) cpu->sq[nextOlderStore].value);
				break;
		}
		lqEntry->state = LQComplete;
	}
	return;
}

/*
 * AXP_21264_Mbox_LQ_Init
 *	This function is called to initialize the entry.  This is a new entry in
 *	in the LQ for processing.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the LQ.
 *
 * Output Parameters:
 *	lqEntry:
 *		A pointer to the entry which has been initialized.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_LQ_Init(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE	*lqEntry = &cpu->lq[entry];
	u32				fault;
	bool			_asm;

	/*
	 * First, determine the length of the load.
	 */
	switch (lqEntry->instr->opcode)
	{

		case LDBU:
			lqEntry->len = 1;
			break;

		case LDW_U:
			lqEntry->len = 2;
			break;

		case LDF:
		case LDS:
		case LDL:
		case LDL_L:
			lqEntry->len = 4;
			break;

		case LDA:
		case LDAH:
		case LDQ_U:
		case LDG:
		case LDT:
		case LDQ:
		case LDQ_L:
			lqEntry->len = 8;
			break;

		case HW_LD:
			if (lqEntry->instr->len_stall == AXP_HW_LD_LONGWORD)
				lqEntry->len = 4;
			else
				lqEntry->len = 8;
			break;
	}

	/*
	 * Then we need to do is translate the virtual address into its physical
	 * equivalent.
	 */
	lqEntry->physAddress = AXP_va2pa(
								cpu,
								lqEntry->virtAddress,
								lqEntry->instr->pc,
								true,	/* use the DTB */
								Read,
								&_asm,
								&fault,
								&lqEntry->instr->excRegMask);

	/*
	 * If a physical address was returned, then we have some more to do.
	 */
	if (lqEntry->physAddress != 0)
	{

		/*
		 * Set a flag indicating the the address is for an I/O device (and not
		 * memory)
		 */
		lqEntry->IOflag = AXP_21264_IS_IO_ADDR(lqEntry->physAddress);

		/*
		 * At this point we have 2 options.  First, this is a load from memory.
		 * Second, this is a load from an I/O device.
		 */
		if (lqEntry->IOflag == false)
		{
			lqEntry->state = LQReadPending;	/* We'll start with this value */
			AXP_21264_Mbox_TryCaches(cpu, entry);
		}

		/*
		 * OK, this is a load from an I/O device.  We just send the request to
		 * the Cbox.
		 */
		else
		{
			lqEntry->state = CboxPending;
			AXP_21264_Add_MAF(
					cpu,
					LDx,
					lqEntry->physAddress,
					(entry + 1),	/* We need to take zero out of play */
					lqEntry->len,
					false);
		}
	}
	else
	{

		/*
		 * We attempted to translate the Virtual Address to a Physical Address,
		 * but failed.  The translation code returned an indicator as to the
		 * fault that prevented the translation from succeeding.  Let the Ibox
		 * know, so that it can call the PALcode that needs to resolve the
		 * fault.  When the fault is resolved, this function will be called
		 * again and should be able to complete the load.
		 */
		AXP_21264_Ibox_Event(
					cpu,
					fault,
					lqEntry->instr->pc,
					lqEntry->virtAddress,
					lqEntry->instr->opcode,
					lqEntry->instr->aDest,
					false,
					false);

		/*
		 * If the fault that occurred is DFAULT, then we found the DTB entry,
		 * but the privileges on it were not what is needed to complete the
		 * instruction.  For the other possible exceptions, we should get
		 * called back.
		 */
		if (fault == AXP_DFAULT)
			lqEntry->state = LQComplete;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_SQ_Pending
 *	This function is called to determine if a store to memory (Dcache) can be
 *	completed.  This is determined by the state of the cache block.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the SQ.
 *
 * Output Parameters:
 *	sqEntry:
 *		A pointer to the entry which has been processed so a store can be
 *		completed.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_SQ_Pending(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE		*sqEntry = &cpu->sq[entry];
	AXP_CBOX_MAF_TYPE	type;
	u32					cacheStatus;
	bool				DcHit = false;
	bool				DcW = false;
	bool				shared;

	/*
	 * First, get the status for the Dcache for the current VA/PA pair.
	 */
	sqEntry->instr->excRegMask = AXP_Dcache_Status(
						cpu,
						sqEntry->virtAddress,
						sqEntry->physAddress,
						sqEntry->len,
						((sqEntry->instr->opcode == STQ_U) ||
						 (sqEntry->instr->opcode == HW_ST)),
						&cacheStatus,
						&sqEntry->dcacheLoc,
						false);

	/*
	 * If not exception was returned from the Dcache status call, then let's
	 * try to determine what we need to do next.
	 */
	if (sqEntry->instr->excRegMask == NoException)
	{

		/*
		 * If we did not hit in the Dcache, go see if the information is in the
		 * Bcache.
		 */
		if (AXP_CACHE_MISS(cacheStatus) == true)
		{

			/*
			 * Because we may make 2 calls to check the Bcache and then to
			 * copy a block from it, we need to lock the Bcache mutex, because
			 * we don't want the Cbox to change anything between calls.
			 */
			pthread_mutex_lock(&cpu->bCacheMutex);

			/*
			 * Get the status for the Bcache for the current PA.
			 */
			cacheStatus = AXP_21264_Bcache_Status(
									cpu,
									sqEntry->physAddress);

			/*
			 * Hit in the Bcache, move the data to the Dcache, which may
			 * require evicting the current entry.
			 */
			if (AXP_CACHE_HIT(cacheStatus) == true)
			{

				/*
				 * We found what we were looking for in the Bcache.  We may
				 * need to evict the current block (possibly the same index and
				 * set, but not the same physical tag).
				 */
				AXP_CopyBcacheToDcache(
								cpu,
								&sqEntry->dcacheLoc,
								sqEntry->physAddress);
				DcHit = true;

				/*
				 * If the cache status is Dirty and Not Shared, then it is
				 * writable.
				 */
				if (AXP_CACHE_DIRTY(cacheStatus) == true)
					DcW = true;
			}

			/*
			 * Unlock the Bcache mutex, as we no longer need it.
			 */
			pthread_mutex_unlock(&cpu->bCacheMutex);
		}
		else
		{
			DcHit = true;
			if (AXP_CACHE_DIRTY(cacheStatus) == true)
				DcW = true;
		}

		/*
		 * OK, we can finally determine what may need to happen next.  First,
		 * if we did hit in either cache, then we need to go get the block from
		 * memory.
		 */
		shared = (AXP_CACHE_CLEAN_SHARED(cacheStatus) ||
				  AXP_CACHE_DIRTY_SHARED(cacheStatus));
		if (DcHit == false)
		{
			sqEntry->state = CboxPending;
			if ((sqEntry->instr->opcode == STL_C) ||
				(sqEntry->instr->opcode == STQ_C))
				type = STx_C;
			else
				type = STx;

			AXP_21264_Add_MAF(
				cpu,
				type,
				sqEntry->physAddress,
				-(entry + 1),	/* We need to take zero out of play */
				sqEntry->len,
				shared);
		}

		/*
		 * We hit the cache.  Depending upon the state, we either need to make it
		 * writable or we are done.
		 */
		else
		{
			AXP_Dcache_Lock(cpu, sqEntry->virtAddress, sqEntry->physAddress);

			/*
			 * If the block is not writable, then we need to request the block be
			 * changed to dirty and not shared (writable).
			 */
			if (DcW ==false)
			{
				sqEntry->state = CboxPending;
				if ((sqEntry->instr->opcode == STL_C) ||
					(sqEntry->instr->opcode == STQ_C))
					type = STxCChangeToDirty;
				else
					type = STxChangeToDirty;

				AXP_21264_Add_MAF(
					cpu,
					type,
					sqEntry->physAddress,
					-(entry + 1),	/* We need to take zero out of play */
					sqEntry->len,
					shared);
			}

			/*
			 * We hit in the cache and the block is writable.  Therefore, we are
			 * done here.
			 */
			else
				sqEntry->state = SQComplete;
		}
	}
	else
	{

		/*
		 * The only exception we can get from the call to check the Dcache is
		 * an alignment fault.  If we did, then we need to generate the correct
		 * event for the Ibox to process (into the PALcode).  Also, there
		 * is no more processing associated with this store instruction.
		 */
		AXP_21264_Ibox_Event(
					cpu,
					AXP_UNALIGNED,
					sqEntry->instr->pc,
					sqEntry->virtAddress,
					sqEntry->instr->opcode,
					sqEntry->instr->aSrc1,
					true,
					false);
		sqEntry->state = SQComplete;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_SQ_Init
 *	This function is called to initialize the entry.  This is a new entry in
 *	in the SQ for processing.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	entry:
 *		The value of the index into the SQ.
 *
 * Output Parameters:
 *	sqEntry:
 *		A pointer to the entry which has been initialized.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_SQ_Init(AXP_21264_CPU *cpu, u8 entry)
{
	AXP_MBOX_QUEUE *sqEntry = &cpu->sq[entry];
	u32		fault;
	bool 	_asm;

	/*
	 * First, determine the length of the store, and get the data to be stored.
	 */
	switch (sqEntry->instr->opcode)
	{
		case STB:
			sqEntry->len = 1;
			sqEntry->value = sqEntry->instr->src1v.r.ub;
			break;

		case STW:
			sqEntry->len = 2;
			sqEntry->value = sqEntry->instr->src1v.r.uw;
			break;

		case STF:
		case STS:
			sqEntry->len = 8;
			sqEntry->value = sqEntry->instr->src1v.fp.uq;
			break;

		case STL:
		case STL_C:
			sqEntry->len = 4;
			sqEntry->value = sqEntry->instr->src1v.r.ul;
			break;

		case STQ_U:
			sqEntry->len = 8;
			sqEntry->value = sqEntry->instr->src1v.r.uq;
			break;

		case STG:
		case STT:
			sqEntry->len = 8;
			sqEntry->value = sqEntry->instr->src1v.fp.uq;
			break;

		case STQ:
		case STQ_C:
			sqEntry->len = 8;
			sqEntry->value = sqEntry->instr->src1v.r.sq;
			break;

		case HW_ST:
			if (sqEntry->instr->len_stall == AXP_HW_LD_LONGWORD)
			{
				sqEntry->len = 4;
				sqEntry->value = sqEntry->instr->src1v.r.ul;
			}
			else
			{
				sqEntry->len = 8;
				sqEntry->value = sqEntry->instr->src1v.r.uq;
			}
			break;
	}

	/*
	 * Then we need to do is translate the virtual address into its physical
	 * equivalent.
	 */
	sqEntry->physAddress = AXP_va2pa(
								cpu,
								sqEntry->virtAddress,
								sqEntry->instr->pc,
								true,	/* use the DTB */
								Write,
								&_asm,
								&fault,
								&sqEntry->instr->excRegMask);

	/*
	 * If a physical address was returned, then we have some more to do.
	 */
	if (sqEntry->physAddress != 0)
	{

		/*
		 * Set a flag indicating the the address is for an I/O device (and not
		 * memory)
		 */
		sqEntry->IOflag = AXP_21264_IS_IO_ADDR(sqEntry->physAddress);

		/*
		 * At this point we have 2 options.  First, this is a store to memory.
		 * Second, this is a store to an I/O device.
		 */
		if (sqEntry->IOflag == false)
		{
			sqEntry->state = SQWritePending;	/* We'll start with this value */
			AXP_21264_Mbox_SQ_Pending(cpu, entry);
		}

		/*
		 * OK, this is a store to an I/O device.  We just send the request to
		 * the Cbox.  There is nothing more to do here, so just indicate that
		 * the store is complete.
		 */
		else
		{
			AXP_21264_Add_IOWB(
					cpu,
					sqEntry->physAddress,
					-(entry + 1),	/* We need to take zero out of play */
					(u8 *) &sqEntry->value,
					sqEntry->len);
			sqEntry->state = SQComplete;
		}
	}
	else
	{

		/*
		 * We attempted to translate the Virtual Address to a Physical Address,
		 * but failed.  The translation code returned an indicator as to the
		 * fault that prevented the translation from succeeding.  Let the Ibox
		 * know, so that it can call the PALcode that needs to resolve the
		 * fault.  When the fault is resolved, this function will be called
		 * again and should be able to complete the store.
		 */
		AXP_21264_Ibox_Event(
					cpu,
					fault,
					sqEntry->instr->pc,
					sqEntry->virtAddress,
					sqEntry->instr->opcode,
					sqEntry->instr->aSrc1,
					true,
					false);

		/*
		 * If the fault that occurred is DFAULT, then we found the DTB entry,
		 * but the privileges on it were not what is needed to complete the
		 * instruction.  For the other possible exceptions, we should get
		 * called back.
		 */
		if (fault == AXP_DFAULT)
			sqEntry->state = SQComplete;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_Process_Q
 *	This function is called because we just received and indication that one or
 *	more entries in the LQ and/or SQ require processed.  This function searches
 *	through all the entries and determines the next processing that is
 *	required.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 *
 * NOTE: When we are called, the Mbox mutex is already locked.  No need to lock
 * it here.
 */
void AXP_21264_Mbox_Process_Q(AXP_21264_CPU *cpu)
{
	u32 ii;

	/*
	 * First the Load Queue (LQ) entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		switch (cpu->lq[ii].state)
		{
			case Initial:
				AXP_21264_Mbox_LQ_Init(cpu, ii);
				break;

			case LQReadPending:
				if (cpu->lq[ii].IOflag == false)
					AXP_21264_Mbox_TryCaches(cpu, ii);
				else
				{
					cpu->lq[ii].instr->destv.r.uq = cpu->lq[ii].IOdata;
					cpu->lq[ii].state = LQComplete;
				}
				break;

			default:
				/* nothing to be done */
				break;
		}

		/*
		 * Because the above calls can and do complete LQ entries by the time
		 * they return.  If the state of the entry is now Complete, then call
		 * the code to finish up with this request and get it back to the Ebox
		 * or Fbox.
		 */
		if (cpu->lq[ii].state == LQComplete)
		{
			void (*complRtn)();

			/*
			 * We are either completing an Integer Load or a Floating-Point
			 * Load.  Select the completion routine, based on the instruction
			 * type.
			 */
			if ((cpu->lq[ii].instr->opcode == LDBU) ||
				(cpu->lq[ii].instr->opcode == LDW_U) ||
				(cpu->lq[ii].instr->opcode == LDL) ||
				(cpu->lq[ii].instr->opcode == LDL_L) ||
				(cpu->lq[ii].instr->opcode == LDQ) ||
				(cpu->lq[ii].instr->opcode == LDQ_U) ||
				(cpu->lq[ii].instr->opcode == LDQ_L) ||
				(cpu->lq[ii].instr->opcode == HW_LD))
				complRtn = AXP_21264_Ebox_Compl;
			else
				complRtn = AXP_21264_Fbox_Compl;
			complRtn(cpu, cpu->lq[ii].instr);
			AXP_21264_Mbox_PutLQSlot(cpu, ii);
		}
	}

	/*
	 * Last the store Queue (SQ) entries.
	 */
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		switch (cpu->sq[ii].state)
		{
			case Initial:
				AXP_21264_Mbox_SQ_Init(cpu, ii);
				break;

			case SQWritePending:
				AXP_21264_Mbox_SQ_Pending(cpu, ii);
				break;

			default:
				/* nothing to be done */
				break;

		}

		/*
		 * Because the above calls can and do complete SQ entries by the time
		 * they return.  If the state of the entry is now Complete, then call
		 * the code to finish up with this request and get it back to the Ibox.
		 */
		if (cpu->sq[ii].state == SQComplete)
		{
			void (*complRtn)();

			/*
			 * We are either completing an Integer Store or a Floating-Point
			 * store.  Use the instruction to determine which completion
			 * function to call (Ebox - Integer or Fbox - Floating-point).
			 */
			if ((cpu->sq[ii].instr->opcode == STB) ||
				(cpu->sq[ii].instr->opcode == STW) ||
				(cpu->sq[ii].instr->opcode == STL) ||
				(cpu->sq[ii].instr->opcode == STL_C) ||
				(cpu->sq[ii].instr->opcode == STQ) ||
				(cpu->sq[ii].instr->opcode == STQ_U) ||
				(cpu->sq[ii].instr->opcode == STQ_C) ||
				(cpu->sq[ii].instr->opcode == HW_ST))
				complRtn = AXP_21264_Ebox_Compl;
			else
				complRtn = AXP_21264_Fbox_Compl;
			complRtn(cpu, cpu->sq[ii].instr);
		}
	}
	return;
}

/*
 * AXP_21264_Mbox_RetireWrite
 *	This function is called by the Ibox when retiring the associated Store.
 *	We do this like a 2-phase commit.  The Mbox prepares the write for
 *	retirement and informs the Ibox.  When the Ibox goes to retire the
 *	instruction, it calls this function to commit the actual write into the
 *	Dcache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	slot:
 *		A value indicating the assigned Store Queue (SQ) where this write entry
 *		has to be stored.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 * 	None.
 *
 * NOTE:	The Ibox calls this function with no Mbox mutexes locked.  Also,
 *			this call does not result in a signal to the Mbox.
 */
void AXP_21264_Mbox_RetireWrite(AXP_21264_CPU *cpu, u8 slot)
{
	AXP_DcacheWrite(
				cpu,
				&cpu->sq[slot].dcacheLoc,
				cpu->sq[slot].len,
				&cpu->sq[slot].value,
				0);
	AXP_21264_Mbox_PutSQSlot(cpu, slot);
	return;
}

/*
 * AXP_21264_Mbox_WorkQueued
 *	This function is called to determine if there is any work to be processed.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	There is work to process.
 *	false:	There is nothing to do.
 *
 */
bool AXP_21264_Mbox_WorkQueued(AXP_21264_CPU *cpu)
{
	bool	retVal = false;
	int		ii;

	/*
	 * If anything has been queued up for processing, then return true as soon
	 * as one is found.
	 */
	for (ii = 0; ((ii < AXP_MBOX_QUEUE_LEN) && (retVal == false)); ii++)
		if (cpu->lq[ii].state == Initial)
			retVal = true;
	for (ii = 0; ((ii < AXP_MBOX_QUEUE_LEN) && (retVal == false)); ii++)
		if (cpu->sq[ii].state == Initial)
			retVal = true;

	/*
	 * Return the results back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_UpdateDcache
 *	This function is called by the Cbox to update a particular block within the
 *	Dcache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	lsSqEntry:
 *		The value of the signed index+1 into the LQ/SQ.  A value < 0 is for the
 *		SQ, otherwise the LQ.
 *	data:
 *		A pointer to a buffer containing data returned from a Load/Store from
 *		physical memory.
 *	status:
 *		A value indicating the bits to be set in the DTAG (dirty and shared).
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Mbox_UpdateDcache(
						AXP_21264_CPU *cpu,
						i8 lqSqEntry,
						u8 *data,
						u8 status)
{
	bool			signalCond = false;
	bool			loadFlag = lqSqEntry <= 0;
	u8				entry = abs(lqSqEntry) - 1;
	AXP_MBOX_QUEUE	*qEntry = (loadFlag == true) ?
							&cpu->lq[entry] :
							&cpu->sq[entry];

	/*
	 * First things first, we have to lock the Mbox mutex.
	 */
	pthread_mutex_lock(&cpu->mBoxMutex);

	/*
	 * Write the data to the Dcache block.
	 */
	AXP_DcacheWrite(
				cpu,
				&qEntry->dcacheLoc,
				AXP_DCACHE_DATA_LEN,
				data,
				status);

	/*
	 * We need to signal the Mbox if the entry was in a pending Cbox state.
	 * If so, we need to change the state to Write Pending.
	 */
	signalCond = qEntry->state == CboxPending;
	if (signalCond == true)
		qEntry->state = (loadFlag == true) ? LQReadPending : SQWritePending;

	/*
	 * If we changed one of the SQ/LQ states, then signal the Mbox that there
	 * may be something to process.
	 */
	if (signalCond == true)
		pthread_cond_signal(&cpu->mBoxCondition);

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
* AXP_21264_Mbox_Init
 *	This function is called by the Cbox to initialize the Mbox items.  These
 *	items are:
 *
 *		- Dcache
 *		- Dcache Tag
 *		- Duplicate Tag
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to initialize the Mbox
 *	false:	Successful normal completion.
 */
bool AXP_21264_Mbox_Init(AXP_21264_CPU *cpu)
{
	bool retVal = false;
	int ii, jj;

	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			memset(cpu->dCache[ii][jj].data, 0, AXP_DCACHE_DATA_LEN);
			cpu->dtag[ii][jj].physTag = 0;
			cpu->dtag[ii][jj].ctagIndex = AXP_CACHE_ENTRIES;
			cpu->dtag[ii][jj].ctagSet = AXP_2_WAY_CACHE;
			cpu->dtag[ii][jj].physTag = 0;
			cpu->dtag[ii][jj].valid = false;
			cpu->dtag[ii][jj].dirty = false;
			cpu->dtag[ii][jj].shared = false;
			cpu->dtag[ii][jj].modified = false;
			cpu->dtag[ii][jj].set_0_1 = false;;
			cpu->dtag[ii][jj].locked = false;;
			cpu->dtag[ii][jj].state = Invalid;
		}
	}

	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->lq[ii].value = 0;
		cpu->lq[ii].virtAddress = 0;
		cpu->lq[ii].instr = NULL;
		cpu->lq[ii].state = QNotInUse;
		cpu->lq[ii].IOflag = false;
		cpu->lq[ii].lockCond = false;
	}
	cpu->lqNext = 0;
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->sq[ii].value = 0;
		cpu->sq[ii].virtAddress = 0;
		cpu->sq[ii].instr = NULL;
		cpu->sq[ii].state = QNotInUse;
		cpu->sq[ii].IOflag = false;
		cpu->sq[ii].lockCond = false;
	}
	cpu->sqNext = 0;
	for (ii = 0; ii < AXP_TB_LEN; ii++)
	{
		cpu->dtb[ii].virtAddr = 0;
		cpu->dtb[ii].physAddr = 0;
		cpu->dtb[ii].matchMask = 0;
		cpu->dtb[ii].keepMask = 0;
		cpu->dtb[ii].kre = 0;
		cpu->dtb[ii].ere = 0;
		cpu->dtb[ii].sre = 0;
		cpu->dtb[ii].ure = 0;
		cpu->dtb[ii].kwe = 0;
		cpu->dtb[ii].ewe = 0;
		cpu->dtb[ii].swe = 0;
		cpu->dtb[ii].uwe = 0;
		cpu->dtb[ii].faultOnRead = 0;
		cpu->dtb[ii].faultOnWrite = 0;
		cpu->dtb[ii].faultOnExecute = 0;
		cpu->dtb[ii].res_1 = 0;
		cpu->dtb[ii].asn = 0;
		cpu->dtb[ii]._asm = false;
		cpu->dtb[ii].valid = false;
	}
	cpu->nextDTB = 0;
	cpu->tbMissOutstanding = false;;
	cpu->dtbTag0.res_1 = 0;
	cpu->dtbTag0.va = 0;
	cpu->dtbTag0.res_2 = 0;
	cpu->dtbTag1.res_1 = 0;
	cpu->dtbTag1.va = 0;
	cpu->dtbTag1.res_2 = 0;
	cpu->dtbPte0.res_1 = 0;
	cpu->dtbPte0.pa = 0;
	cpu->dtbPte0.res_2 = 0;
	cpu->dtbPte0.uwe = 0;
	cpu->dtbPte0.swe = 0;
	cpu->dtbPte0.ewe = 0;
	cpu->dtbPte0.kwe = 0;
	cpu->dtbPte0.ure = 0;
	cpu->dtbPte0.sre = 0;
	cpu->dtbPte0.ere = 0;
	cpu->dtbPte0.kre = 0;
	cpu->dtbPte0.res_3 = 0;
	cpu->dtbPte0.gh = 0;
	cpu->dtbPte0._asm = 0;
	cpu->dtbPte0.res_4 = 0;
	cpu->dtbPte0.fow = 0;
	cpu->dtbPte0._for = 0;
	cpu->dtbPte0.res_5 = 0;
	cpu->dtbPte1.res_1 = 0;
	cpu->dtbPte1.pa = 0;
	cpu->dtbPte1.res_2 = 0;
	cpu->dtbPte1.uwe = 0;
	cpu->dtbPte1.swe = 0;
	cpu->dtbPte1.ewe = 0;
	cpu->dtbPte1.kwe = 0;
	cpu->dtbPte1.ure = 0;
	cpu->dtbPte1.sre = 0;
	cpu->dtbPte1.ere = 0;
	cpu->dtbPte1.kre = 0;
	cpu->dtbPte1.res_3 = 0;
	cpu->dtbPte1.gh = 0;
	cpu->dtbPte1._asm = 0;
	cpu->dtbPte1.res_4 = 0;
	cpu->dtbPte1.fow = 0;
	cpu->dtbPte1._for = 0;
	cpu->dtbPte1.res_5 = 0;
	cpu->dtbAltMode.alt_mode = AXP_MBOX_ALTMODE_KERNEL;
	cpu->dtbAltMode.res = 0;
	cpu->dtbIs0.res_1 = 0;
	cpu->dtbIs0.inval_itb = 0;
	cpu->dtbIs0.res_2 = 0;
	cpu->dtbIs1.res_1 = 0;
	cpu->dtbIs1.inval_itb = 0;
	cpu->dtbIs1.res_2 = 0;
	cpu->dtbAsn0.res_1 = 0;
	cpu->dtbAsn0.asn = 0;
	cpu->dtbAsn0.res_2 = 0;
	cpu->dtbAsn1.res_1 = 0;
	cpu->dtbAsn1.asn = 0;
	cpu->dtbAsn1.res_2 = 0;
	cpu->mmStat.res = 0;
	cpu->mmStat.dc_tag_perr = 0;
	cpu->mmStat.opcodes = 0;
	cpu->mmStat.fow = 0;
	cpu->mmStat._for = 0;
	cpu->mmStat.acv = 0;
	cpu->mmStat.wr = 0;
	cpu->mCtl.res_1 = 0;
	cpu->mCtl.spe = 0;
	cpu->mCtl.res_2 = 0;
	cpu->dcCtl.dcdat_err_en = 0;
	cpu->dcCtl.dctag_par_en = 0;
	cpu->dcCtl.f_bad_decc = 0;
	cpu->dcCtl.f_bad_tpar = 0;
	cpu->dcCtl.f_hit = 0;
	cpu->dcCtl.set_en = 3;				/* use both Dcache sets */
	cpu->dcStat.res = 0;
	cpu->dcStat.seo = 0;
	cpu->dcStat.ecc_err_ld = 0;
	cpu->dcStat.ecc_err_st = 0;
	cpu->dcStat.tperr_p0 = 0;
	cpu->dcStat.tperr_p1 = 0;

	/*
	 * All done, return to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_MboxMain
 * 	This function is called to perform the emulation for the Mbox within the
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
void *AXP_21264_MboxMain(void *voidPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) voidPtr;

	/*
	 * While the CPU is not shutting down, we either have to wait before we can
	 * do anything, or we are ready to do some work.
	 */
	while (cpu->cpuState != ShuttingDown)
	{
		switch (cpu->cpuState)
		{

			/*
			 * The first 3 are the initial states when the cpu is being
			 * initialized, by the Cbox.  The last one means that something
			 * happened and we are basically resetting everything, also handled
			 * by the Cbox.  We just need to wait until the CPU is in the 'run'
			 * state again (or shutting down).
			 */
			case Cold:
			case WaitBiST:
			case WaitBiSI:
			case FaultReset:

				/*
				 * The Mbox needs to wait until the cpu is in the 'run' state.
				 */
				pthread_mutex_lock(&cpu->cpuMutex);
				while ((cpu->cpuState != Run) && (cpu->cpuState != ShuttingDown))
					pthread_cond_wait(&cpu->cpuCond, &cpu->cpuMutex);
				pthread_mutex_unlock(&cpu->cpuMutex);
				break;

			case Run:

				/*
				 * If there is something to process, then process it.
				 * Otherwise, wait for something to get queued up.
				 */
				pthread_mutex_lock(&cpu->mBoxMutex);
				if (AXP_21264_Mbox_WorkQueued(cpu) == false)
					pthread_cond_wait(&cpu->mBoxCondition, &cpu->mBoxMutex);
				AXP_21264_Mbox_Process_Q(cpu);
				pthread_mutex_unlock(&cpu->mBoxMutex);
				break;

			case Sleep:

				/*
				 * Need to quiesce everything and put the world to sleep
				 * waiting just for the wake-up signal.
				 */
				break;

			case ShuttingDown:

				/*
				 * Nothing to do here.
				 */
				break;
		}
	}

	/*
	 * We are shutting down.  Since we started everything, we need to clean
	 * ourself up.  The main function will be joining to all the threads it
	 * created and then freeing up the memory and exiting the image.
	 */
	pthread_exit(NULL);

	/*
	 * Return back to the caller.  Because of the call to pthread_exit(), we'll
	 * never get here.  We do this to keep the compiler happy.
	 */
	return(NULL);
}
