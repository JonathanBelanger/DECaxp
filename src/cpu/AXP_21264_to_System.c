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
 *	This module contains the code for the protocol sent from the CPU to the
 *	System over the Sysbus.
 *
 * Revision History:
 *
 *	V01.000		31-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_21264_CPU.h"

/*
 * AXP_21264_SendToSystem
 *  This function is called with a pointer to the SysBus message and a pointer
 *  to the CPU structure, and sends the message to the CPU, locking the correct
 *  mutex and signaling the correct condition variable.
 *
 * Input Parameters:
 *  cpu:
 *	A pointer to the CPU structure for the emulation.
 *  msg:
 *	A pointer to the message to send to the specified CPU.
 */
void AXP_21264_SendToSystem(AXP_21264_CPU *cpu, AXP_21264_SYSBUS_System *msg)
{
    AXP_21264_RQ_ENTRY *rq;

    /*
     * Lock the mutex so that no one else tries to manipulate the queue or the
     * index into it.
     */
    pthread_mutex_lock(cpu->system.mutex);

    /*
     * Queue up the next PQ entry for the CPU to process.
    if (cpu->system.rq[*cpu->system.rqEnd].valid == true)
	*cpu->system.rqEnd = (*cpu->system.rqEnd + 1) % AXP_21264_CCHIP_RQ_LEN;
     *
     * TODO:	The below is bogus.  We need to rationalize the interfaces
     *		between components.  This is just to keep the compiler happy.
     */
    rq = (AXP_21264_RQ_ENTRY *) cpu->system.rq; /*[*cpu->system.rqEnd]; */

    /*
     * Copy the data from the System structure and into the CPU structure.
     *
     * TODO:	We need to do something about data movement, mask, deal with a
     *		probeResponse, and wrapping data.
     *
     * TODO:	There is way too much copying of data from one buffer to the
     * 		next.  We should look at defining a pool of buffers that can
     * 		be copied once and passed around as needed.  Buffers should
     * 		only be copied out of the source and copied into the
     * 		destination.
     */
    rq->pa = msg->pa;
    rq->miss1 = msg->m1;
    rq->miss2 = msg->m2;
    rq->status = msg->cmd;
    rq->cacheHit = msg->ch;
    rq->rqValid = msg->rv;
    rq->valid = true;
    rq->waitVector = msg->id; /* TODO: Probably not correct */
    rq->mask = msg->mask;
    switch (msg->cmd)
    {
	case ReadDataError:
	    memset(rq->sysData, 0xff, AXP_21264_DATA_SIZE);
/*	    rq->dm = true; */
	    break;

	case ReadData:
	case ReadDataDirty:
	case ReadDataShared:
	case ReadDataSharedDirty:
	    memcpy(rq->sysData, msg->sysData, AXP_21264_DATA_SIZE);
/*	    rq->dm = true;
	    rq->wrap = msg->wrap; */
	    break;

	default:
/*	    rq->dm = false; */
	    break;
    }

    /*
     * OK, we are done here.  Signal the CPU that it has something to process.
     */
    pthread_cond_signal(cpu->system.cond);

    /*
     * Unlock the CPU's interface Mutex, so that the CPU can process the data
     * we just queued up to it.
     */
    pthread_mutex_unlock(cpu->system.mutex);

    /*
     * Return back to the caller.
     */
    return;
}

