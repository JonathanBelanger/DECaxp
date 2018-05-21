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
 *	Victim Data Buffer (VDB) functionality of the Cbox.
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
 * AXP_21264_VDB_Empty
 *	This function is called to determine of there is a record in the Victim
 *	Data Buffer (VDB) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_VDB_Empty(AXP_21264_CPU *cpu)
{
    int retVal = -1;
    int ii;
    int end, start1, end1, start2 = -1, end2;

    if (cpu->vdbTop > cpu->vdbBottom)
    {
	start1 = cpu->vdbTop;
	end1 = AXP_21264_VDB_LEN - 1;
	start2 = 0;
	end2 = cpu->vdbBottom;
    }
    else
    {
	start1 = cpu->vdbTop;
	end1 = cpu->vdbBottom;
    }

    /*
     * Search through the list to find the first entry that can be processed.
     */
    ii = start1;
    end = end1;
    while ((ii <= end) && (retVal == -1))
    {
	if ((cpu->vdb[ii].valid == true) && (cpu->vdb[ii].processed == false))
	    retVal = ii;
	if ((retVal == -1) && (start2 != -1) && (ii == end))
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
 * AXP_21264_Process_VDB
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the VDB records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the VDB to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_VDB(AXP_21264_CPU *cpu, int entry)
{
    AXP_21264_CBOX_VIC_BUF *vdb = &cpu->vdb[entry];
    u16 mask = AXP_LOW_QUAD;
    bool m1 = false;
    bool m2 = false;
    bool rv = true;
    bool ch = false;

    /*
     * Process the next VDB entry that needs it.
     */
    switch (vdb->type)
    {

	/*
	 * Istream cache blocks from memory or Dcache blocks to be written to
	 * the Bcache.
	 */
	case toBcache:
	    AXP_21264_Bcache_Write(cpu, vdb->pa, vdb->sysData);
	    break;

	    /*
	     * We need to write a Bcache block out to memory.
	     * 			or
	     * Dcache or Bcache blocks to send to the system in response to a
	     * probe command.
	     */
	case toMemory:
	case probeResponse:

	    /*
	     * Go check the Oldest pending PQ and set the flags for it here and
	     * now.  Only if the oldest PQ entry is a miss.
	     */
	    AXP_21264_OldestPQFlags(cpu, &m1, &m2, &ch);

	    /*
	     * OK, send what we have to the System.
	     */
	    AXP_System_CommandSend(
		WrVictimBlk,
		m2,
		entry,
		rv,
		mask,
		ch,
		vdb->pa,
		vdb->sysData,
		AXP_21264_SIZE_QUAD);
	    break;
    }

    /*
     * Indicate that the entry is now processed and return back to the caller.
     */
    vdb->processed = true;
    return;
}

/*
 * AXP_21264_Add_VDB
 *	This function is called to add an Victim Data Buffer (VDB) entry on to the
 *	queue for processing.
 *
 *	NOTE:	The Mbox and Cbox call this function.  The Mbox to have a Dcache
 *			block to be written to the Bcache.  The Cbox to have Istream blocks
 *			recently written to the Icache to the Bcache as well
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	entry used.
 */
u8 AXP_21264_Add_VDB(
    AXP_21264_CPU *cpu,
    AXP_21264_VDB_TYPE type,
    u64 pa,
    u8 *buf,
    bool probe,
    bool alreadyLocked)
{
    bool locked = false;

    /*
     * Before we do anything, lock the interface mutex to prevent multiple
     * accessors.
     */
    if (alreadyLocked == false)
    {
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);
	locked = true;
    }

    /*
     * Add a record to the next available VDB.
     */
    if (cpu->vdb[cpu->vdbBottom].valid == true)
	cpu->vdbBottom = (cpu->vdbBottom + 1) & 0x07;
    cpu->vdb[cpu->vdbBottom].type = type;
    cpu->vdb[cpu->vdbBottom].pa = pa;
    cpu->vdb[cpu->vdbBottom].validProbe = probe;
    memset(cpu->vdb[cpu->vdbBottom].sysData, '\0', AXP_21264_SIZE_QUAD);
    memcpy(cpu->vdb[cpu->vdbBottom].sysData, buf, AXP_21264_SIZE_QUAD);
    cpu->vdb[cpu->vdbBottom].valid = true;
    cpu->vdb[cpu->vdbBottom].processed = false;

    /*
     * Let the Cbox know there is something for it to process, then unlock the
     * mutex so it can.
     */
    if (locked == true)
    {
	pthread_cond_signal(&cpu->cBoxInterfaceCond);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
    }
    return (cpu->vdbBottom);
}

/*
 * AXP_21264_IsSetP_VDB
 *	This function is called to determine if the P bit is set on a VDB with a
 *	matching physical address specified in a PQ entry.  If this is the case,
 *	then sending ProbeResponses is inhibited until this bit is cleared.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	pa:
 *		A 64-bit virtual address to be matched to a VDB entry.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	true:	The P-bit is set, so no ProbeResponses will be returned to the
 *			system until this bit is cleared.
 *	false:	The P-bit is not set, so go ahead and send the response.
 */
bool AXP_21264_IsSetP_VDB(AXP_21264_CPU *cpu, u64 pa)
{
    bool retVal = false;
    int ii;
    int end, start1, end1, start2 = -1, end2;
    if (cpu->vdbTop > cpu->vdbBottom)
    {
	start1 = cpu->vdbTop;
	end1 = AXP_21264_VDB_LEN - 1;
	start2 = 0;
	end2 = cpu->vdbBottom;
    }
    else
    {
	start1 = cpu->vdbTop;
	end1 = cpu->vdbBottom;
    }

    /*
     * Search through the list to find the first entry that is in-use (valid).
     */
    ii = start1;
    end = end1;
    while ((ii <= end) && (retVal == false))
    {
	if ((cpu->vdb[ii].valid == true) && (cpu->vdb[ii].pa == pa))
	    retVal = true;
	if ((retVal == false) && (start2 != -1) && (ii == end))
	{
	    ii = start2;
	    end = end2;
	    start2 = -1;
	}
	else
	    ii++;
    }

    /*
     * Return the result back to the caller.
     */
    return (retVal);
}

/*
 * AXP_21264_ClearP_VDB
 * 	This function is called to clear the P (probe-valid) bit in a VDB.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the VDB to have it's P bit
 *		cleared.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_ClearP_VDB(AXP_21264_CPU *cpu, u8 entry)
{
    AXP_21264_CBOX_VIC_BUF *vdb = &cpu->vdb[entry];

    /*
     * All we have to do is clear the P bit.
     */
    vdb->validProbe = false;

    /*
     * Let the PQ send out any pending ProbeResponses.
     */
    AXP_21264_SendRsps_PQ(cpu);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21264_Free_VDB
 * 	This function is called to return a previously allocated VDB buffer.  It
 * 	does this by setting the valid bit to false and adjusting the vdbTop
 * 	index, as necessary.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the VDB to be invalidated.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Free_VDB(AXP_21264_CPU *cpu, u8 entry)
{
    AXP_21264_CBOX_VIC_BUF *vdb = &cpu->vdb[entry];
    int ii;
    int end, start1, end1, start2 = -1, end2;
    bool done = false;

    /*
     * First, clear the valid bit.
     */
    vdb->valid = false;

    /*
     * We now have to see if we can adjust the top of the queue.
     */
    if (cpu->vdbTop > cpu->vdbBottom)
    {
	start1 = cpu->vdbTop;
	end1 = AXP_21264_VDB_LEN - 1;
	start2 = 0;
	end2 = cpu->vdbBottom;
    }
    else
    {
	start1 = cpu->vdbTop;
	end1 = cpu->vdbBottom;
    }

    /*
     * Search through the list to find the first entry that is in-use (valid).
     */
    ii = start1;
    end = end1;
    while ((ii <= end) && (done == false))
    {
	if (cpu->vdb[ii].valid == false)
	    cpu->vdbTop = (cpu->vdbTop + 1) & 0x07;
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
     * Return back to the caller.
     */
    return;
}
