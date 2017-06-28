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
 *	functionality of the Mbox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Mbox.h"

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
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As loads are completed, the index will be reduced.
	 */
	if (cpu->lqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->lqNext++;
		cpu->lq[retVal].state = Assigned;
	}

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}
/*
 * AXP_21264_Mbox_ReadMem
 *	This function is called to queue up a read from Dcache based on a virtual
 *	address, size of the data to be read and the instsruction that is queued up
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
 *	length:
 *		A value indicating the length, in bytes, of the value to read from the
 *		Dcache.
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
							u64 virtAddr,
							u32 length)
{
	
	/*
	 * Store the information in the
	 */
	cpu->lq[slot].length = length;
	cpu->lq[slot].virtAddress = virtAddr;
	cpu->lq[slot].instr = instr;
	cpu->lq[slot].state = ReadPending;

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
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As stores are completed, the index will be reduced.
	 */
	if (cpu->sqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->sqNext++;
		cpu->sq[retVal].state = Assigned;
	}

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_WriteMem
 *	This function is called to queue up a write to the Dcache based on a
 *	virtual address, size of the data to be written, the value of the data and
 *	the instsruction that is queued up to be completed in order.  This function
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
 *	length:
 *		A value indicating the length, in bytes, of the value to be written to
 *		the Dcache.
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
							 u64 value,
							 u32 length)
{
	
	/*
	 * Store the information in the
	 */
	cpu->sq[slot].value = value;
	cpu->sq[slot].length = length;
	cpu->sq[slot].virtAddress = virtAddr;
	cpu->sq[slot].instr = instr;
	cpu->sq[slot].state = WritePending;

	/*
	 * Return back to the caller.
	 */
	return;
}