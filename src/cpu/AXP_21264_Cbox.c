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
 *	functionality of the Cbox.
 *
 *	Revision History:
 *
 *	V01.000		26-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Cbox.h"

/*
 * AXP_VictimBuffer
 *	This function is responsible for managing the Victim Buffer (VB).  The VB
 *	is comprised of the Victim Address File (VAF) and Victim Data File (VDF).
 *	There are a total of eight(8) entries in the VB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_VictimBuffer()
{
	return;
}

/*
 * AXP_IOWriteBuffer
 *	This function is responsible for managing the I/O Write Buffer (IOWB).
 *	There are a total of four(4) 64-byte entries in the IOWB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IOWriteBuffer()
{
	return;
}

/*
 * AXP_ProbeQueue
 *	This function is responsible for managing the Probe Queue (PQ).
 *	There are a total of eight(8) entries in the PQ.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_ProbeQueue()
{
	return;
}

/*
 * AXP_DuplicateDcacheTagArray
 *	This function is responsible for managing the duplicate Dcache Tag (DTAG)
 *	array and is used by the Cbox when processing Dcache fills, Icache fills
 *	and system port probes.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_DuplicateDcacheTagArray()
{
	return;
}

/*
 * AXP_IQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Integer Issue Queue (IQ).  Architecturally, this is defined in the Cbox
 *	as the "Arbiter".  According to the architecture, there are two(2) arbiters
 *	for the IQ, one for the upper subclusters and one for the lower.  Each will
 *	select two instructions from the 20 possible queued integer instructions.
 *	Priority is given to older requests over newer ones.  If one instruction
 *	requests both lower subclusters and there is no other requesting a lower
 *	subcluster, then L0 is selected.  The same, but for the upperclusters will
 *	select U1.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IQArbiter()
{
	return;
}

/*
 * AXP_FQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Floatin-point Issue Queue (FQ).  Architecturally, this is defined in
 *	the Cbox as the "Arbiter".  According to the architecture, there are
 *	three(3) arbiters for the FQ, one for each add, multiply, or store
 *	pipelines.  The add and multiply arbiters will pick one request, while the
 *	store arbiter will pick two (one for each store pipeline).  Priority is
 *	given to older requests over newer ones.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_FQArbiter()
{
	return;
}
