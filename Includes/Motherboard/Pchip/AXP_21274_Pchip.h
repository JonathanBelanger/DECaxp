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
 *	Pchip emulation.
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		12-May-2018	Jonathan D. Belanger
 *	Moved the Pchip CSRs and queues over here to be similar to the real thing.
 */
#ifndef _AXP_21274_PCHIP_H_
#define _AXP_21274_PCHIP_H_

#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Trace.h"
#include "Motherboard/AXP_21274_Registers.h"
#include "Motherboard/Cchip/AXP_21274_Cchip.h"
#include "Motherboard/Dchip/AXP_21274_Dchip.h"

/*
 * The following definition contains the fields and data structures required to
 * implement a single Pchip.  There is always at least one of these and as many
 * as two of them.
 */
#define AXP_21274_CAPBUS_MQ_SIZE	4+4	/* up to 4 messages in each direction */
typedef struct
{

    /*
     * There is one thread/mutex/condition variable per Pchip.
     */
    pthread_t threadID;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    /*
     * Pchip ID
     */
    u32 pChipID;

    /*
     * Interface queues.
     */
    AXP_QUEUE_HDR tpr; /* To Pchip Requests CSC<PRQMAX> */
    AXP_QUEUE_HDR fpr; /* From Pchip Requests PCTL<CRQMAX> */
    u32 tprCnt; /* Up to CSC<PRQMAX> */
    u32 fprCnt; /* Up to PCTL<CRQMAX> */
    AXP_CAPbusMsg rq[AXP_21274_CAPBUS_MQ_SIZE];
    u32 rqIdx;

    /*
     * The following are the Pchip CSRs.  The addresses for these Pchip CSRs
     * have n=1 for p0 and n=3 for p1.
     */
    AXP_21274_WSBAn wsba0; /* Address: 80n.8000.0000 */
    AXP_21274_WSBAn wsba1; /* Address: 80n.8000.0040 */
    AXP_21274_WSBAn wsba2; /* Address: 80n.8000.0080 */
    AXP_21274_WSBA3 wsba3; /* Address: 80n.8000.00c0 */
    AXP_21274_WSMn wsm0; /* Address: 80n.8000.0100 */
    AXP_21274_WSMn wsm1; /* Address: 80n.8000.0140 */
    AXP_21274_WSMn wsm2; /* Address: 80n.8000.0180 */
    AXP_21274_WSMn wsm3; /* Address: 80n.8000.01c0 */
    AXP_21274_TBAn tba0; /* Address: 80n.8000.0200 */
    AXP_21274_TBAn tba1; /* Address: 80n.8000.0240 */
    AXP_21274_TBAn tba2; /* Address: 80n.8000.0280 */
    AXP_21274_TBAn tba3; /* Address: 80n.8000.02c0 */
    AXP_21274_PCTL pctl; /* Address: 80n.8000.0300 */
    AXP_21274_PLAT plat; /* Address: 80n.8000.0340 */
    /* AXP_21274_RES		res;		 * Address: 80n.8000.0380 */
    AXP_21274_PERROR perror; /* Address: 80n.8000.03c0 */
    AXP_21274_PERRMASK perrMask; /* Address: 80n.8000.0400 */
    AXP_21274_PERRSET perrSet; /* Address: 80n.8000.0440 */
    AXP_21274_TLBIV tlbiv; /* Address: 80n.8000.0480 */
    AXP_21274_PMONCTL pMonCtl; /* Address: 80n.8000.0500 */
    AXP_21274_PMONCNT pMonCnt; /* Address: 80n.8000.0540 */
    AXP_21274_SPRST sprSt; /* Address: 80n.8000.0800 */
} AXP_21274_PCHIP;

#define AXP_21274_WHICH_PCHIP(addr) (((addr) & 0x0000000200000000) >> 33)

/*
 * Pchip Function Prototypes
 */
void *AXP_21274_PchipMain(void *);

#endif /* _AXP_21274_PCHIP_H_ */
