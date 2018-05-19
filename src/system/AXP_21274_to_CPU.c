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
 *	This module contains the code for the protocol sent from the System to the
 *	CPU over the Sysbus.
 *
 *	Revision History:
 *
 *	V01.000		30-Mar2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21274_21264_Common.h"
#include "AXP_Utility.h"
#include "AXP_Configure.h"

/*
 * AXP_21274_SendToCPU
 *	This function is called with a pointer to the SysBus message and a pointer
 *	to the CPU structure, and send the message to the CPU, locking the correct
 *	mutex and signaling the correct condition variable.
 *
 * Input Parameters:
 * 	msg:
 * 		A pointer to the message to send to the specified CPU.
 * 	cpu:
 * 		A pointer to the CPU to receive the message.
 */
void AXP_21264_SendToCPU(AXP_21274_SYSBUS_CPU *msg, AXP_21274_CPU *cpu)
{
    AXP_21274_CBOX_PQ *pq;

    /*
     * Lock the mutex so that no one else tries to manipulate the queue or the
     * index into it.
     */
    pthread_mutex_lock(cpu->mutex);

    /*
     * Queue up the next PQ entry for the CPU to process.
     */
    if (cpu->pq[*cpu->pqBottom].valid == true)
	*cpu->pqBottom = (*cpu->pqBottom + 1) & (AXP_21274_PQ_LEN - 1);
    pq = &cpu->pq[*cpu->pqBottom];

    /*
     * Copy the data from the System structure and into the CPU structure.
     */
    pq->pa = msg->pa;
    pq->sysDc = msg->sysDc;
    pq->probeStatus = HitClean; /* Just initializing */
    pq->rvb = msg->rvb;
    pq->rpb = msg->rpb;
    pq->a = msg->a;
    pq->c = msg->c;
    pq->processed = false;
    pq->valid = true;
    pq->pendingRsp = false;
    pq->vs = false;
    pq->ms = false;
    pq->ID = msg->id;
    switch (msg->sysDc)
    {
	case ReadDataError:
	    memset(pq->sysData, 0xff, AXP_21274_DATA_SIZE);
	    pq->dm = true;
	    break;

	case ReadData:
	case ReadDataDirty:
	case ReadDataShared:
	case ReadDataSharedDirty:
	    memcpy(pq->sysData, msg->sysData, AXP_21274_DATA_SIZE);
	    pq->dm = true;
	    pq->wrap = msg->wrap;
	    break;

	default:
	    pq->dm = false;
	    break;
    }

    /*
     * OK, we are done here.  Signal the CPU that it has something to process.
     */
    pthread_cond_signal(cpu->cond);

    /*
     * Unlock the CPU's interface Mutex, so that the CPU can process the data
     * we just queued up to it.
     */
    pthread_mutex_unlock(cpu->mutex);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21274_InterruptToCPU
 *	This function is called with a pointer to the irq_H bits and a pointer
 *	to the CPU structure, and updates the equivalent field in the CPU, locking
 *	the correct mutex and signaling the correct condition variable.
 *
 * Input Parameters:
 * 	irq_H:
 * 		A value of the interrupt bits to be set in the CPU.
 * 	cpu:
 * 		A pointer to the CPU to receive the interrupts.
 */
void AXP_21264_SendToCPU(u8 irq_H, AXP_21274_CPU *cpu)
{

    /*
     * Lock the mutex so that no one else tries to manipulate the queue or the
     * index into it.
     */
    pthread_mutex_lock(cpu->mutex);

    /*
     * This function only sets flags, it does not clear them.  Therefore, OR
     * the bits we want to set with the bits that are already set.
     */
    *cpu->irq_H |= irq_H;

    /*
     * OK, we are done here.  Signal the CPU that it has something to process.
     */
    pthread_cond_signal(cpu->cond);

    /*
     * Unlock the CPU's interface Mutex, so that the CPU can process the
     * interrupts we just set.
     */
    pthread_mutex_unlock(cpu->mutex);

    /*
     * Return back to the caller.
     */
    return;
}
