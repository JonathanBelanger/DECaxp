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
 *	I/O Write Buffer (IOWB) functionality of the Cbox.
 *
 * Revision History:
 *
 *	V01.000		29-Dec-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21264_Cbox.h"
#include "AXP_Configure.h"
#include "AXP_21264_CacheDefs.h"
#include "AXP_21264_Mbox.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Ibox.h"

/*
 * AXP_21264_IOWB_Empty
 *  This function is called to determine of there is a record in the I/O
 *  Write Buffer (IOWB) that needs to be processed.
 *
 * Input Parameters:
 *  cpu:
 * 	A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 	processor.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  -1:		No entries requiring processing.
 *  >=0:	Index of next entry that can be processed.
 */
int AXP_21264_IOWB_Empty(AXP_21264_CPU *cpu)
{
    int retVal = -1;
    int ii;
    int end, start1, end1, start2 = -1, end2;

    if (cpu->iowbTop > cpu->iowbBottom)
    {
	start1 = cpu->iowbTop;
	end1 = AXP_21264_IOWB_LEN - 1;
	start2 = 0;
	end2 = cpu->iowbBottom;
    }
    else
    {
	start1 = cpu->iowbTop;
	end1 = cpu->iowbBottom;
    }

    /*
     * Search through the list to find the first entry that can be processed.
     */
    ii = start1;
    end = end1;
    while ((ii <= end) && (retVal == -1))
    {
	if ((cpu->iowb[ii].valid == true) && (cpu->iowb[ii].processed == false))
	    retVal = ii;
	if ((retVal == -1) && (start2 != -1) && (ii = end))
	{
	    ii = start2;
	    end = end2;
	    start2 = -1;
	}
	else
	    ii++;
    }

    /*
     * Return what we found, if anything.
     */
    return (retVal);
}

/*
 * AXP_21264_Process_IOWB
 *  This function is called to check the first unprocessed entry on the queue
 *  containing the IOWB records.
 *
 * Input Parameters:
 *  cpu:
 *	A pointer to the CPU structure for the emulated Alpha AXP 21264
 *	processor.
 *  entry:
 *	An integer value that is the entry in the IOWB to be processed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_21264_Process_IOWB(AXP_21264_CPU *cpu, int entry)
{
    AXP_21264_CBOX_IOWB *iowb = &cpu->iowb[entry];
    AXP_System_Commands cmd;
    bool m1 = false;
    bool m2 = false;
    bool rv = true;
    bool ch = false;

    /*
     * Process the next IOWB entry that needs it.
     */
    switch (iowb->storeLen)
    {
	case BYTE_LEN:
	    cmd = WrBytes;
	    break;

	case WORD_LEN:
	    cmd = WrBytes;
	    break;

	case LONG_LEN:
	    cmd = WrLWs;
	    break;

	case QUAD_LEN:
	    cmd = WrQWs;
	    break;
    }

    /*
     * Go check the Oldest pending PQ and set the flags for it here and now.
     */
    AXP_21264_OldestPQFlags(cpu, &m1, &m2, &ch);

    /*
     * OK, send what we have to the System.
    AXP_System_CommandSend(
	cmd,
	m2,
	entry,
	rv,
	iowb->mask,
	ch,
	iowb->pa,
	iowb->sysData,
	iowb->bufLen);
     */

    /*
     * Indicate that the entry is now processed and return back to the caller.
     */
    iowb->processed = true;
    return;
}

/*
 * AXP_21264_Merge_IOWB
 *  This function is called to attempt to merge an request for a new I/O Write
 *  Block (IOWB) with an existing entry on to the queue for processing.
 *
 *  NOTE:	The Mbox calls this function. It does so when it has a SQ entry
 *		needs to be written to an I/O device.
 *
 * Input Parameters:
 *  iowb:
 *	A pointer to the next queued IOWB to see if we can merge the next data
 *	with it.
 *  pa:
 *	A value indicating the physical address of the device the I/O Buffer is
 *	to be written.
 *  data:
 *	A pointer to the buffer to be written to the device.
 *  dataLen:
 *	A value indicating the length of the 'data' parameter.
 *  maxLen:
 *	A value indicating the longest merged buffer that is allowed (32 or
 *	64).
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	A new IOWB still needs to be allocated.
 *  false:	The data was merged into an existing IOWB.
 */
bool AXP_21264_Merge_IOWB(
    AXP_21264_CBOX_IOWB *iowb,
    u64 pa,
    i8 lqSqEntry,
    u8 *data,
    int dataLen,
    int maxLen)
{
    u64 paEnd;
    bool retVal = true;

    /*
     * If the IOWB is valid, is being used for a length the same as
     * the current one, is for an ascending, consecutive address, has
     * not yet been processed, and the block is for aligned values.
     */
    if ((iowb->valid == true) && (iowb->storeLen == dataLen)
	&& (iowb->processed == false))
    {
	paEnd = iowb->pa + iowb->bufLen;

	/*
	 * If the merge register is not full, then copy this next block
	 * into it and update the length.  Also, indicate that an IOWB
	 * does not need to be allocated.
	 */
	if ((paEnd <= pa) && ((pa + dataLen) <= (iowb->pa + maxLen)))
	{
	    if (dataLen == LONG_LEN)
		*((u32 *) &iowb->sysData[pa - iowb->pa]) = *((u32 *) data);
	    else
		*((u64 *) &iowb->sysData[pa - iowb->pa]) = *((u64 *) data);
	    iowb->bufLen = (pa + dataLen) - iowb->pa;
	    AXP_MaskSet(&iowb->mask, iowb->pa, pa, dataLen);
	    retVal = false;
	}
    }

    /*
     * Let the caller know what just happened, if anything.
     */
    return (retVal);
}

/*
 * AXP_21264_Add_IOWB
 *  This function is called to add an I/O Write Block (IOWB) entry on to the
 *  queue for processing.
 *
 *  NOTE:	The Mbox calls this function. It does so when it has a SQ entry
 *		needs to be written to an I/O device.
 *
 * Input Parameters:
 *  cpu:
 *	A pointer to the CPU structure for the emulated Alpha AXP 21264
 *	processor.
 *  pa:
 *	A value indicating the physical address of the device the I/O Buffer is
 *	to be written.
 *  data:
 *	A pointer to the buffer to be written to the device.
 *  dataLen:
 *	A value indicating the length of the 'data' parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_21264_Add_IOWB(
    AXP_21264_CPU *cpu,
    u64 pa,
    i8 lqSqEntry,
    u8 *data,
    int dataLen)
{
    AXP_21264_CBOX_IOWB *iowb;
    u32 ii;
    bool allocateIOWB = true;

    /*
     * Before we do anything, lock the interface mutex to prevent multiple
     * accessors.
     */
    pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

    /*
     * HRM Table 2–8 Rules for I/O Address Space Store Instruction Data Merging
     *
     * -------------------------------------------------------------------------------------
     * Merge Register/		Store
     * Replayed Instruction	Byte/Word	Store Longword		Store Quadword
     * -------------------------------------------------------------------------------------
     * Byte/Word 		No merge	No merge		No merge
     * Longword			No merge	Merge up to 32 bytes	No merge
     * Quadword			No merge	No merge		Merge up to 64 bytes
     * -------------------------------------------------------------------------------------
     * Table 2–8 shows some of the following rules:
     * 	- Byte/word store instructions and different size store instructions
     * 	  are not allowed to merge.
     *	- A stream of ascending non-overlapping, but not necessarily
     *	  consecutive, longword store instructions are allowed to merge into
     *	  naturally aligned 32-byte blocks.
     *	- A stream of ascending non-overlapping, but not necessarily
     *	  consecutive, quadword store instructions are allowed to merge into
     *	  naturally aligned 64-byte blocks.
     *	- Merging of quadwords can be limited to naturally-aligned 32-byte
     *	  blocks based on the Cbox WRITE_ONCE chain 32_BYTE_IO field.
     *	- Issued MB, WMB, and I/O load instructions close the I/O register
     *	  merge window.  To minimize latency, the merge window is also closed
     *	  when a timer detects no I/O store instruction activity for 1024
     *	  cycles.
     */
    if ((dataLen != BYTE_LEN) || (dataLen != WORD_LEN)) /* Don't merge Bytes/Words */
    {
	int maxLen;
	int end, start1, end1, start2 = -1, end2 = -1;

	if (cpu->iowbTop > cpu->iowbBottom)
	{
	    start1 = cpu->iowbTop;
	    end1 = AXP_21264_IOWB_LEN - 1;
	    start2 = 0;
	    end2 = cpu->iowbBottom;
	}
	else
	{
	    start1 = cpu->iowbTop;
	    end1 = cpu->iowbBottom;
	}

	/*
	 * We either have a longword or a quadword.  Longwords can be merged
	 * upto 32-bytes long.  Quadwords can either be merged upto 64-bytes or
	 * 32-bytes, depending upon the setting of the 32_BYTE_IO field in the
	 * Cbox CSR.
	 */
	if (((dataLen == QUAD_LEN) && (cpu->csr.ThirtyTwoByteIo == 1))
	    || (dataLen == LONG_LEN))
	    maxLen = AXP_21264_SIZE_LONG;
	else
	    maxLen = AXP_21264_SIZE_QUAD;

	/*
	 * Search through each of the allocated IOWBs and see if they are
	 * candidates for merging.
	 */
	ii = start1;
	end = end1;
	while ((ii <= end) && (allocateIOWB == true))
	{
	    allocateIOWB = AXP_21264_Merge_IOWB(
		&cpu->iowb[ii],
		pa,
		lqSqEntry,
		data,
		dataLen,
		maxLen);
	    if ((allocateIOWB == false) && (start2 != -1) && (ii = end))
	    {
		ii = start2;
		end = end2;
		start2 = -1;
	    }
	    else
		ii++;
	}
    }

    /*
     * If we didn't perform a merge, then we need to add a record to the next
     * available IOWB.
     */
    if (allocateIOWB == true)
    {
	if (cpu->iowb[cpu->iowbBottom].valid == true)
	    cpu->iowbBottom = (cpu->iowbBottom + 1) & 0x03;
	iowb = &cpu->iowb[cpu->iowbBottom];
	iowb->pa = pa;
	iowb->lqSqEntry[0] = lqSqEntry;
	for (ii = 1; ii < AXP_21264_MBOX_MAX; ii++)
	    iowb->lqSqEntry[ii] = 0;
	iowb->storeLen = iowb->bufLen = dataLen;
	if (data != NULL) /* this is a store */
	    memcpy(iowb->sysData, data, dataLen);
	else
	    memset(iowb->sysData, 0, sizeof(iowb->sysData));
	iowb->bufLen = dataLen;
	AXP_MaskReset(&iowb->mask);
	AXP_MaskSet(&iowb->mask, iowb->pa, pa, dataLen);
	iowb->processed = false;
	iowb->valid = true;
    }

    /*
     * Let the Cbox know there is something for it to process, then unlock the
     * mutex so it can.
     */
    pthread_cond_signal(&cpu->cBoxInterfaceCond);
    pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
    return;
}

/*
 * AXP_21264_Free_IOWB
 *  This function is called to return a previously allocated IOWB buffer.  It
 *  does this by setting the valid bit to false and adjusting the iowbTop
 *  index, as necessary.
 *
 * Input Parameters:
 *  cpu:
 *	A pointer to the CPU structure for the emulated Alpha AXP 21264
 *	processor.
 *  entry:
 *	An integer value that is the entry in the IOWB to be invalidated.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_21264_Free_IOWB(AXP_21264_CPU *cpu, u8 entry)
{
    AXP_21264_CBOX_IOWB *iowb = &cpu->iowb[entry];
    int ii;
    int end, start1, end1, start2 = -1, end2;
    bool done = false;

    /*
     * First, clear the valid bit.
     */
    iowb->valid = false;

    /*
     * We now have to see if we can adjust the top of the queue.
     */
    if (cpu->iowbTop > cpu->iowbBottom)
    {
	start1 = cpu->iowbTop;
	end1 = AXP_21264_IOWB_LEN - 1;
	start2 = 0;
	end2 = cpu->iowbBottom;
    }
    else
    {
	start1 = cpu->iowbTop;
	end1 = cpu->iowbBottom;
    }

    /*
     * Search through the list to find the first entry that is in-use (valid).
     */
    ii = start1;
    end = end1;
    while ((ii <= end) && (done == false))
    {
	if (cpu->iowb[ii].valid == false)
	    cpu->iowbTop = (cpu->iowbTop + 1) & 0x07;
	else
	    done = true;
	if ((done == false) && (start2 != -1) && (ii == end))
	{
	    ii = start2;
	    end = end2;
	    start2 = -1;
	}
	else
	    ii++;
    }

    /*
     * Before returning back to the caller, let the Mbox know that a request
     * from it has been completed.
     */
    for (ii = 0; ii < AXP_21264_MBOX_MAX; ii++)
    {
	if (iowb->lqSqEntry != 0)
	    AXP_21264_Mbox_CboxCompl(cpu, iowb->lqSqEntry[ii], NULL, 0, false);
    }

    /*
     * Return back to the caller.
     */
    return;
}
