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
 *	This header file contains the definitions required for the Tsunami/Typhoon
 *	Cchip emulation.
 *
 * Revision History:
 *
 *	V01.000		18-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_CCHIP_H_
#define _AXP_21274_CCHIP_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_21274_Registers.h"

#define AXP_21274_SYSDATA_LEN	sizeof(u64)

typedef enum
{
	phase0,
	phase1,
	phase2,
	phase3
} AXP_21274_PHASES;

/*
 * The following definitions are used for the bit vectors, addrMatchWait,
 * pageHit, and olderRqs.  Each bit represents an entry in the queue of 6
 * entries.
 *
 *	Note:	Although there are conceptually 3-bit vectors, they can be combined
 *			into a 2-bit vector to represent the following four comparisons
 *			against each of the other requests in the array queue (address
 *			match wait includes and overrides page hit):
 *
 *				1. Not younger
 *				2. Younger and page hit match (but not address match wait)
 *				3. Younger and address match wait
 *				4. Younger and no match
 */
#define AXP_21274_NOT_YOUNG		0x00
#define AXP_21274_YOUNG_HIT		0x01
#define AXP_21274_YOUNG_WAIT	0x02
#define AXP_21274_YOUNG			0x03
#define AXP_21274_AGE_MASK		0x03

/*
 * This macro extracts the requested 2 bits used to determine the combination
 * of Address Match Wait, Page Hit, and Older Request vectors.
 */
#define AXP_21264_ENTRY(bitVector, entry)	\
	(((bitVecotr) >> ((entry * 2)) & AXP_21274_AGE_MASK)

/*
 * HRM 6.1.1 Memory Access Request Queues, Skid Buffers, and Dispatch Register
 *
 * Each new request that arrives from a CPU or Pchip is eventually dispatched
 * into one of four request queues. Request queues have the following
 * characteristics:
 *
 *	- Each queue corresponds to one of the memory arrays controlled by the Cchip.
 *	- Each queue has six entries.
 *
 * HRM 6.1.4 Request Queue Maintenance
 *
 * The request queue is a unified queue of all requests from the CPUs and the
 * Pchips. In an implementation-dependent manner, the relative ages of any set
 * of entries can be determined. Each queue entry contains the following
 * information:
 *
 *	- Command and other information, such as; CPU MAF/VAF id, number of QW for
 *	  DMA ops, and PIO mask
 *	- Address
 *	- Phase, Valid
 *	- Status (such as probe results)
 *	- Address match wait vector – A bit vector identifying the older requests
 *	  in this queue with (nearly) the same address, and for which this request
 *	  must wait
 *	- Page hit vector – A bit vector identifying the older requests in this
 *	  queue with the same DRAM page address, so that this request can issue
 *	  after a previous request without waiting for RAS precharge delay
 *	- Older request vector – A bit vector identifying all older requests in
 *	  this queue (used to arbitrate among otherwise equal ready requests)
 */
typedef struct
{
	u8						sysData[AXP_21274_SYSDATA_LEN];
	u64						mask;
	u64						pa;
	AXP_21264_TO_SYS_CMD	cmd;
	AXP_21264_PROBE_STAT	status;
	AXP_21274_PHASES		phase;
	int						entry;
	int						sysDataLen;
	u16						waitVector;
	bool					miss2;
	bool					rqValid;
	bool					cacheHit;
} AXP_21274_RQ_ENTRY;

/*
 * Function prototypes
 */
void AXP_21274_CchipInit(AXP_21274_SYSTEM *);

#endif /* _AXP_21274_CCHIP_H_ */
