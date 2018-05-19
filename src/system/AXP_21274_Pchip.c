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
 *	The Pchip is the interface chip between devices on the PCI bus and the rest
 *	of the system.  There can be one or two Pchips, and corresponding single or
 *	dual PCI buses, connected to the Cchip and Dchips. The Pchip performs the
 *	following functions:
 *		- Accepts requests from the Cchip by means of the CAPbus and enqueues
 *		  them
 *		- Issues commands to the PCI bus based on these requests
 *		- Accepts requests from the PCI bus and enqueues them
 *		- Issues commands to the Cchip by means of the CAPbus based on these
 *		  requests
 *		- Transfers data to and from the Dchips based on the above commands and
 *		  requests
 *		- Buffers the data when necessary
 *		- Reports errors to the Cchip, after recording the nature of the error
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		12-May-2018	Jonathan D. Belanger
 *	Moved the Pchip CSRs to this module.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_21274_Pchip.h"

/*
 * Local Prototypes
 */
static void AXP_21274_ReadPCSR(AXP_21274_PCHIP *, AXP_CAPbusMsg *);
static void AXP_21274_WritePCSR(AXP_21274_PCHIP *, AXP_CAPbusMsg *);

/*
 * AXP_21274_ReadPCSR
 *	This function is called when a read request has come in for a Pchip CSR.
 *
 * Input Parameters:
 *	p:
 *		A pointer to the Pchip data structure from which the emulation
 *		information is maintained.
 *	msg:
 *		A pointer to the request from the Cchuo that contains the CSR to be
 *		read.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	An unsigned  64-bit value read from the CSR.
 */
static u64 AXP_21274_ReadPCSR(AXP_21274_PCHIP *p, AXP_CAPbusMsg *msg)
{
    u64 retVal = 0;

    /*
     * Let's go do what we came here to do.
     */
    switch (msg->csr)
    {
	case 0x00: /* WSBA0 */
	    retVal = *((u64 *) &p->wsba0) & AXP_21274_WSBAn_RMASK;
	    break;

	case 0x01: /* WSBA1 */
	    retVal = *((u64 *) &p->wsba1) & AXP_21274_WSBAn_RMASK;
	    break;

	case 0x02: /* WSBA2 */
	    retVal = *((u64 *) &p->wsba2 & AXP_21274_WSBAn_RMASK);
	    break;

	case 0x03: /* WSBA3 */
	    retVal = *((u64 *) &p->wsba3) & AXP_21274_WSBA3_RMASK;
	    break;

	case 0x04: /* WSM0 */
	    retVal = *((u64 *) &p->wsm0) & AXP_21274_WSMn_RMASK;
	    break;

	case 0x05: /* WSM1 */
	    retVal = *((u64 *) &p->wsm1) & AXP_21274_WSMn_RMASK;
	    break;

	case 0x06: /* WSM2 */
	    retVal = *((u64 *) &p->wsm2) & AXP_21274_WSMn_RMASK;
	    break;

	case 0x07: /* WSM3 */
	    retVal = *((u64 *) &p->wsm3) & AXP_21274_WSMn_RMASK;
	    break;

	case 0x08: /* TBA0 */
	    retVal = *((u64 *) &p->tba0) & AXP_21274_TBAn_RMASK;
	    break;

	case 0x09: /* TBA1 */
	    retVal = *((u64 *) &p->tba1) & AXP_21274_TBAn_RMASK;
	    break;

	case 0x0a: /* TBA2 */
	    retVal = *((u64 *) &p->tba2) & AXP_21274_TBAn_RMASK;
	    break;

	case 0x0b: /* TBA3 */
	    retVal = *((u64 *) &p->tba3) & AXP_21274_TBAn_RMASK;
	    break;

	case 0x0c: /* PCTL */
	    retVal = *((u64 *) &p->pctl) & AXP_21274_PCTL_RMASK;
	    break;

	case 0x0d: /* PLAT */
	    retVal = *((u64 *) &p->plat) & AXP_21274_PLAT_RMASK;
	    break;

	case 0x0f: /* PERROR */
	    retVal = *((u64 *) &p->perror) & AXP_21274_PERROR_RMASK;
	    break;

	case 0x10: /* PERRMASK */
	    retVal = *((u64 *) &p->perrMask) & AXP_21274_PERRMASK_RMASK;
	    break;

	case 0x14: /* PMONCTL */
	    retVal = *((u64 *) &p->pMonCtl) & AXP_21274_PMONC_RMASK;
	    break;

	case 0x15: /* PMONCNT */
	    retVal = *((u64 *) &p->pMonCnt);
	    break;

	default:
	    /* TODO: non-existent memory */
	    break;
    }

    /*
     * Return the results back to the caller.
     */
    return (retVal);
}

/*
 *
 */
typedef union
{
    u64 value;
    AXP_21274_WSBAn Wsban;
    AXP_21274_WSBA3 Wsba3;
    AXP_21274_WSMn Wsmn;
    AXP_21274_TBAn Tban;
    AXP_21274_PCTL Pctl;
    AXP_21274_PLAT Plat;
    AXP_21274_PERROR Perror;
    AXP_21274_PERRMASK PerrMask;
    AXP_21274_PERRSET PerrSet;
    AXP_21274_TLBIV Tlbiv;
    AXP_21274_PMONCTL MonCtl;
    AXP_21274_SPRST SprSt;
} CSR_MAP;

/*
 * AXP_21274_WritePCSR
 *	This function is called when a write request has come in for a CSR.
 *
 * Input Parameters:
 *	sys:
 *		A pointer to the system data structure from which the emulation
 *		information is maintained.
 *	pa:
 *		The Physical Address that contains the CSR to be read.
 *	cpuID:
 *		An unsigned 32-bit value indicating the CPU requesting the read.  This
 *		is used when setting the correct ProbeEnable bit.  This can only be a
 *		value between 0 and 3.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	An unsigned  64-bit value read from the CSR.
 *
 * NOTE:	Writing to a CSR may not always cause the register to be updated.
 *			It is possible that some other action may be generated by simply
 *			executing the write operation.
 */
static void AXP_21274_WritePCSR(AXP_21274_PCHIP *p, AXP_CAPbusMsg *msg)
{
    CSR_MAP csrValue;

    /*
     * Let's go do what we came here to do.
     */
    switch (msg->csr)
    {
	case 0x00: /* WSBA0 */
	    csrValue.value = msg->data[0] & AXP_21274_WSBAn_WMASK;
	    p->wsba0 = csrValue.Wsban;
	    break;

	case 0x01: /* WSBA1 */
	    csrValue.value = msg->data[0] & AXP_21274_WSBAn_WMASK;
	    p->wsba1 = csrValue.Wsban;
	    break;

	case 0x02: /* WSBA2 */
	    csrValue.value = msg->data[0] & AXP_21274_WSBAn_WMASK;
	    p->wsba2 = csrValue.Wsban;
	    break;

	case 0x03: /* WSBA3 */
	    csrValue.value = msg->data[0] & AXP_21274_WSBA3_WMASK;
	    p->wsba3 = csrValue.Wsba3;
	    break;

	case 0x04: /* WSM0 */
	    csrValue.value = msg->data[0] & AXP_21274_WSMn_WMASK;
	    p->wsm0 = csrValue.Wsmn;
	    break;

	case 0x05: /* WSM1 */
	    csrValue.value = msg->data[0] & AXP_21274_WSMn_WMASK;
	    p->wsm1 = csrValue.Wsmn;
	    break;

	case 0x06: /* WSM2 */
	    csrValue.value = msg->data[0] & AXP_21274_WSMn_WMASK;
	    p->wsm2 = csrValue.Wsmn;
	    break;

	case 0x07: /* WSM3 */
	    csrValue.value = msg->data[0] & AXP_21274_WSMn_WMASK;
	    p->wsm3 = csrValue.Wsmn;
	    break;

	case 0x08: /* TBA0 */
	    csrValue.value = msg->data[0] & AXP_21274_TBAn_WMASK;
	    p->tba0 = csrValue.Tban;
	    break;

	case 0x09: /* TBA1 */
	    csrValue.value = msg->data[0] & AXP_21274_TBAn_WMASK;
	    p->tba1 = csrValue.Tban;
	    break;

	case 0x0a: /* TBA2 */
	    csrValue.value = msg->data[0] & AXP_21274_TBAn_WMASK;
	    p->tba2 = csrValue.Tban;
	    break;

	case 0x0b: /* TBA3 */
	    csrValue.value = msg->data[0] & AXP_21274_TBAn_WMASK;
	    p->tba3 = csrValue.Tban;
	    break;

	case 0x0c: /* PCTL */
	    csrValue.value = msg->data[0] & AXP_21274_PCTL_WMASK;
	    p->pctl = csrValue.Pctl;
	    break;

	case 0x0d: /* PLAT */
	    csrValue.value = msg->data[0] & AXP_21274_PLAT_WMASK;
	    p->plat = csrValue.Plat;
	    break;

	case 0x0f: /* PERROR */
	    /* TODO: HRM says RW, but I think it should be RO sys->p0Perror */
	    break;

	case 0x10: /* PERRMASK */
	    csrValue.value = msg->data[0] & AXP_21274_PERRMASK_WMASK;
	    p->perrMask = csrValue.PerrMask;
	    break;

	case 0x11: /* PERRSET */
	    csrValue.value = msg->data[0] & AXP_21274_PERRSET_WMASK;
	    p->perrSet = csrValue.PerrSet;
	    break;

	case 0x12: /* TLBIV */
	    csrValue.value = msg->data[0] & AXP_21274_TLBIV_WMASK;
	    p->tlbiv = csrValue.Tlbiv;
	    break;

	case 0x13: /* TLBIA */
	    /* TODO: Invalidate SG TLB sys->p0Tlbia; */
	    break;

	case 0x14: /* PMONCTL */
	    csrValue.value = msg->data[0] & AXP_21274_PMONC_WMASK;
	    p->pMonCtl = csrValue.MonCtl;
	    break;

	case 0x20: /* SPRST */
	    /* TODO: Soft PCI Reset sys->p0SprSt; */
	    break;

	default:
	    /* TODO: non-existent memory */
	    break;
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21274_PchipInit
 *	This function is called to initialize the Pchip CSRs as documented in HRM
 *	10.2 Chipset Registers.
 *
 * Input Parameters:
 *	p:
 *		A pointer to the Pchip data structure from which the emulation
 *		information is maintained.
 *	id:
 *		A value indicating the numeric identifier associated with this Pchip.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21274_PchipInit(AXP_21274_PCHIP *p, u32 id)
{
    int ii;

    p->pChipID = id; /* Save the ID for this Pchip */

    /*
     * Initialize the message queues.  We do not have the data queues, since
     * we do not have a separate thread reading and writing data to and from
     * memory.  The Cchip performs this function on behalf of the CPU and the
     * Pchip performs this function on behalf of itself.
     */
    AXP_INIT_QUE(p->tpr);
    AXP_INIT_QUE(p->fpr);
    for (ii = 0; ii < AXP_21274_CAPBUS_MQ_SIZE; ii++)
    {
	AXP_INIT_QUE(p->rq[ii].header);
    }

    /*
     * Initialization for WSBA0, WSBA1,and WSBA2 (HRM Table 10-35).
     */
    p->wsba0.res_32 = 0;
    p->wsba0.addr = 0;
    p->wsba0.res_2 = 0;
    p->wsba0.sg = AXP_SG_DISABLE;
    p->wsba0.ena = AXP_ENA_DISABLE;

    p->wsba1.res_32 = 0;
    p->wsba1.addr = 0;
    p->wsba1.res_2 = 0;
    p->wsba1.sg = AXP_SG_DISABLE;
    p->wsba1.ena = AXP_ENA_DISABLE;

    p->wsba2.res_32 = 0;
    p->wsba2.addr = 0;
    p->wsba2.res_2 = 0;
    p->wsba2.sg = AXP_SG_DISABLE;
    p->wsba2.ena = AXP_ENA_DISABLE;

    /*
     * Initialization for WSBA3 (HRM Table 10-36).
     */
    p->wsba3.res_40 = 0;
    p->wsba3.dac = AXP_DAC_DISABLE;
    p->wsba3.res_32 = 0;
    p->wsba3.addr = 0;
    p->wsba3.res_2 = 0;
    p->wsba3.sg = AXP_SG_ENABLE;
    p->wsba3.ena = AXP_ENA_DISABLE;

    /*
     * Initialization for WSM0, WSM1, WSM2, and WSM3 (HRM Table 10-37).
     */
    p->wsm0.res_32 = 0;
    p->wsm0.am = 0;
    p->wsm0.res_0 = 0;

    p->wsm1.res_32 = 0;
    p->wsm1.am = 0;
    p->wsm1.res_0 = 0;

    p->wsm2.res_32 = 0;
    p->wsm2.am = 0;
    p->wsm2.res_0 = 0;

    p->wsm3.res_32 = 0;
    p->wsm3.am = 0;
    p->wsm3.res_0 = 0;

    /*
     * Initialization for TBA0, TBA1, and TBA2 (HRM Table 10-38).
     */
    p->tba0.res_35 = 0;
    p->tba0.addr = 0;
    p->tba0.res_0 = 0;

    p->tba1.res_35 = 0;
    p->tba1.addr = 0;
    p->tba1.res_0 = 0;

    p->tba2.res_35 = 0;
    p->tba2.addr = 0;
    p->tba2.res_0 = 0;

    /*
     * Initialization for TBA3 (HRM Table 10-39).
     */
    p->tba3.res_35 = 0;
    p->tba3.addr = 0;
    p->tba3.res_0 = 0;

    /*
     * Initialization for PCTL (HRM Table 10-40).
     *
     * TODO:	Initialize pid from the PID pins.
     * TODO:	Initialize rpp from the CREQRMT_L pin at system reset.
     * TODO:	Initialize pclkx from the PCI i_pclkdiv<1:0> pins.
     * TODO:	Initialize padm from a decode of the b_cap<1:0> pins.
     */
    p->pctl.res_48 = 0;
    p->pctl.pid = 0;
    p->pctl.rpp = AXP_RPP_NOT_PRESENT;
    p->pctl.ptevrfy = AXP_PTEVRFY_DISABLE;
    p->pctl.fdwdis = AXP_FDWDIS_NORMAL;
    p->pctl.fdsdis = AXP_FDSDIS_NORMAL;
    p->pctl.pclkx = AXP_PCLKX_6_TIMES;
    p->pctl.ptpmax = 2;
    p->pctl.crqmax = 1;
    p->pctl.rev = 0;
    p->pctl.cdqmax = 1;
    p->pctl.padm = 0;
    p->pctl.eccen = AXP_ECCEN_DISABLE;
    p->pctl.res_16 = 0;
    p->pctl.ppri = AXP_PPRI_LOW;
    p->pctl.prigrp = AXP_PRIGRP_PICx_LOW;
    p->pctl.arbena = AXP_ARBENA_DISABLE;
    p->pctl.mwin = AXP_MWIN_DISABLE;
    p->pctl.hole = AXP_HOLE_DISABLE;
    p->pctl.tgtlat = AXP_TGTLAT_DISABLE;
    p->pctl.chaindis = AXP_CHAINDIS_DISABLE;
    p->pctl.thdis = AXP_THDIS_NORMAL;
    p->pctl.fbtb = AXP_FBTB_DISABLE;
    p->pctl.fdsc = AXP_FDSC_ENABLE;

    /*
     * Initialization for PLAT (HRM Table 10-41).
     */
    p->plat.res_32 = 0;
    p->plat.res_16 = 0;
    p->plat.lat = 0;
    p->plat.res_0 = 0;

    /*
     * Initialization for PERROR (HRM Table 10-42).
     */
    p->perror.syn = 0;
    p->perror.cmd = AXP_CMD_DMA_READ;
    p->perror.inv = AXP_INFO_VALID;
    p->perror.addr = 0;
    p->perror.res_12 = 0;
    p->perror.cre = 0;
    p->perror.uecc = 0;
    p->perror.res_9 = 0;
    p->perror.nds = 0;
    p->perror.rdpe = 0;
    p->perror.ta = 0;
    p->perror.ape = 0;
    p->perror.sge = 0;
    p->perror.dcrto = 0;
    p->perror.perr = 0;
    p->perror.serr = 0;
    p->perror.lost = AXP_LOST_NOT_LOST;

    /*
     * Initialization for PERRMASK (HRM Table 10-43).
     */
    p->perrMask.res_12 = 0;
    p->perrMask.mask = 0;

    /*
     * Initialization for PERRSET (HRM Table 10-44).
     */
    p->perrSet.info = 0;
    p->perrSet.res_12 = 0;
    p->perrSet.set = 0;

    /*
     * Initialization for TLBIV (HRM Table 10-45).
     */
    p->tlbiv.res_28 = 0;
    p->tlbiv.dac = 0;
    p->tlbiv.res_20 = 0;
    p->tlbiv.addr = 0;
    p->tlbiv.res_0 = 0;

    /*
     * The register for TLBIA (HRM Table 10-46) is really a pseudo register.
     * As such, writes to it is ignored.  A write to this register will simply
     * invalidate the scatter-gather TLB associated with that Pchip.
     * Therefore, there is no need to define the register or initialize it.
     */

    /*
     * Initialization for PMONCTL (HRM Table 10-47).
     */
    p->pMonCtl.res_18 = 0;
    p->pMonCtl.stkdis1 = AXP_STKDIS_STICKS_1S;
    p->pMonCtl.stkdis0 = AXP_STKDIS_STICKS_1S;
    p->pMonCtl.slct1 = 0;
    p->pMonCtl.slct0 = 1;

    /*
     * Initialization for PMONCNT (HRM Table 10-48).
     */
    p->pMonCnt.cnt1 = 0;
    p->pMonCnt.cnt0 = 0;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21274_Pchip_Main
 *	This is the main function for the Pchip.  It looks at its queues to
 * 	determine if there is anything that needs to be processed from the Cchip or
 * 	PCI devices.
 *
 * Input Parameters:
 * 	p:
 * 		A pointer to the Pchip structure for the emulated DECchip 21272/21274
 * 		chipsets.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void *AXP_21274_PchipMain(void *voidPtr)
{
    AXP_21274_PCHIP *p = (AXP_21274_PCHIP *) voidPtr;
    AXP_CAPbusMsg *msg;

    /*
     * Log that we are starting.
     */
    if (AXP_SYS_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("Pchip p%d is starting", p->pChipID);
	AXP_TRACE_END()
	;
    }

    /*
     * First lock the Pchip's mutex so that we can make sure to coordinate
     * access to the Pchip's queues.
     */
    pthread_mutex_lock(&p->mutex);

    /*
     * TODO: Need to determine what the end condition for this loop should be.
     */
    while (true)
    {

	/*
	 * The Pchip performs the following functions:
	 *
	 * This first thing we need to do is wait for something to arrive to be
	 * processed.
	 */
	whileAXP_QUE_EMPTY(p->tpr)
	pthread_cond_wait(&p->cond, &p->mutex);

	/*
	 * We have something to process.
	 */
	msg = (AXP_CAPbusMsg *) p->tpr.flink;
	AXP_REMQUE(&msg->header);

	/*
	 * At this point, we can unlock the Pchip mutex so that other threads
	 * can send requests to the Pchip.  We'll lock it before we mark the
	 * request to be processed as no longer in use.
	 */
	pthread_mutex_unlock(&p->mutex);

	/*
	 * Determine what has been requested and make the call needed to
	 * complete request.
	 */
	switch (msg->cmd)
	{
	    case PIO_IACK:
	    case PIO_SpecialCycle:
	    case PIO_Read:
	    case PIO_Write:
	    case PIO_MemoryWritePTP:
	    case PIO_MemoryRead:
	    case PTPMemoryRead:
	    case PIO_MemoryWriteCPU:
	    break;

	    case CSR_Read:
	    AXP_21274_ReadPCSR(p, msg);
	    break;

	    case CSR_Write:
	    AXP_21274_WritePCSR(p, msg);
	    break;

	    case PCI_ConfigRead:
	    case PCI_ConfigWrite:
	    case LoadPADbusDataDown:
	    case LoadPADbusDataUp:
	    case CAPbus_NoOp:
	    case DMAReadNQW:
	    case SGTEReadNQW:
	    case PTPMemoryRead:
	    case PTPMemoryWrite:
	    case DMARdModyWrQW:
	    case DMAWriteNQW:
	    case PTPMemoryWrite:
	    case PTPWrByteMaskByp:
	    break;
	}

	/*
	 * We are shutting down.  Since we started everything, we need to clean
	 * ourself up.  The main function will be joining to all the threads it
	 * created and then freeing up the memory and exiting the image.
	 */
	pthread_exit(NULL);
	return(NULL);
    }
