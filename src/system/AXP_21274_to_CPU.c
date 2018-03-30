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
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_21274_to_CPU.h"
#include "AXP_21264_CPU.h"

/*
 * AXP_21274_SendToCPU
 *	This funciton is called with a pointer to the SysBus message and a pointer
 *	to the CPU structure, and send the message to the CPU, locking the correct
 *	mutex and signalling the correct condition variable.
 *
 *	TODO:	Do we want to System to have the whole CPU structure or just the
 *			things it needs to interface with it.  This way, the System does
 *			not have to have any knowledge of the CPU, except how to
 *			communicate with it.
 *
 * Input Parameters:
 * 	msg:
 * 		A pointer to the message to send to the specified CPU.
 * 	cpu:
 * 		A pointer to the CPU to receive the message.
 */
void AXP_21264_SendToCPU(AXP_21274_SYSBUS_MSG *msg, void *cpuPtr)
{
	AXP_21264_CPU	*cpu = (AXP_21264_CPU *) cpuPtr;

	/*
	 * Lock the mutex, queue up the message, signal the condition variable,
	 * then unlock the mutex and we are done.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);
	/* Need a location to store the message */
	pthread_cond_signal(&cpu->cBoxInterfaceCond);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

