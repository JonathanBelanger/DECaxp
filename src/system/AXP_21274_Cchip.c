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
 *	This module contains the code for the Tsunami/Typhoon Cchip.  The Cchip
 *	performs the following functions:
 *		- Accepts requests from the Pchips and the CPUs
 *		- Orders the arriving requests as required
 *		- Selects among the requests to issue controls to the DRAMs
 *		- Issues probes to the CPUs as appropriate to the selected requests
 *		- Translates CPU PIO addresses to PCI and CSR addresses
 *		- Issues commands to the Pchip as appropriate to the selected (PIO or
 *		  PTP) requests
 *		- Issues responses to the Pchip and CPU as appropriate to the issued
 *		  requests
 *		- Issues controls to the Dchip as appropriate to the DRAM accesses, and
 *		  the probe and Pchip responses
 *		- Controls the TIGbus to manage interrupts, and maintains CSRs
 *		  including those that represent interrupt status
 *	The Tsunami supports up to 2 CPUs and the Typhoon supports up to 4 CPUs.
 *
 * Revision History:
 *
 *	V01.000		18-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21274_System.h"
#include "AXP_21274_Cchip.h"
#include "AXP_21274_21264_Common.h"

/*
 * AXP_21274_CchipInit
 *	This function is called to initialize the Cchip CSRs as documented in HRM
 *	10.2 Chipset Registers.
 *
 * Input Parameters:
 *	sys:
 *		A pointer to the system data structure from which the emulation
 *		information is maintained.
 *
 * Output Parameters:
 *	sys:
 *		A pointer to the system data structure, with the Cchip CSRs initialized
 *		based on the HRM for Tsunami/Typhoon chip set.
 *
 * Return Values:
 *	None.
 */
void AXP_21274_CchipInit(AXP_21274_SYSTEM *sys)
{
	u32		hh, ii, jj;

	/*
	 * Initialization for CSC (HRM Table 10-10)
	 *
	 * TODO:	iddw, iddr, and aw are updated when the Dchip STR register is
	 *			written.
	 * TODO:	Byte 0 powers up to the value present on bits <7:0> of the
	 * 			TIGbus.  This includes fw, sed, c1cfp, c0cfp, and bc fields.
	 */
	sys->csc.res_63 = 0;
	sys->csc.res_62 = 0;
	sys->csc.p1w = 1;
	sys->csc.p0w = 1;
	sys->csc.res_59 = 0;
	sys->csc.pbqmax = 1;
	sys->csc.res_55 = 0;
	sys->csc.prqmax = 2;
	sys->csc.res_51 = 0;
	sys->csc.pdtmax = 1;
	sys->csc.res_47 = 0;
	sys->csc.fpqpmax = 0;
	sys->csc.res_43 = 0;
	sys->csc.fpqcmax = 1;
	sys->csc.axd = 0;
	sys->csc.tpqmmax = 1;
	sys->csc.b3d = 0;
	sys->csc.b2d = 0;
	sys->csc.b1d = 0;
	sys->csc.fti = 0;
	sys->csc.eft = AXP_EFT_1_CYCLES;
	sys->csc.qdi = AXP_QDI_DISABLE_DRAINING;
	sys->csc.fet = AXP_FET_3_CYCLE;
	sys->csc.qpm = AXP_QPM_ROUND_ROBIN;
	sys->csc.pme = 0;
	sys->csc.res_22 = 0;
	sys->csc.drtp = AXP_DRTP_5_CYCLES;
	sys->csc.dwfp = AXP_DWFP_5_CYCLES;
	sys->csc.dwtp = AXP_DWTP_5_CYCLES;
	sys->csc.res_15 = 0;
	sys->csc.pip = 1;	/* in this implementation P1 is always present */
	sys->csc.iddw = AXP_IDDW_6_CYCLES;
	sys->csc.iddr = AXP_IDDR_9_CYCLES;
	sys->csc.aw = AXP_AW_16_BYTES;
	sys->csc.fw = 0;
	sys->csc.sfd = AXP_SFD_2_CYCLES;
	sys->csc.sed = AXP_SED_2_CYCLES;
	sys->csc.c1cfp = 0;
	sys->csc.c0cfp = 0;
	sys->csc.bc = 0;

	/*
	 * Initialization for MTR (HRM Table 10-11)
	 */
	sys->mtr.res_46 = 0;
	sys->mtr.mph = 0;
	sys->mtr.phcw = 14;
	sys->mtr.phcr = 15;
	sys->mtr.res_30 = 0;
	sys->mtr.ri = 0;
	sys->mtr.mpd = 0;
	sys->mtr.res_17 = 0;
	sys->mtr.rrd = AXP_RRD_2_CYCLES;
	sys->mtr.res_14 = 0;
	sys->mtr.rpt = AXP_RPT_2_CYCLES;
	sys->mtr.res_10 = 0;
	sys->mtr.rpw = AXP_RPW_4_CYCLES;
	sys->mtr.res_7 = 0;
	sys->mtr.ird = AXP_IRD_0_CYCLES;
	sys->mtr.res_3 = 0;
	sys->mtr.cat = AXP_CAT_2_CYCLES;
	sys->mtr.res_1 = 0;
	sys->mtr.rcd = AXP_RCD_2_CYCLES;

	/*
	 * Initialization for MISC (HRM Table 10-12)
	 */
	sys->misc.res_44 = 0;
	sys->misc.devSup = 0;
	sys->misc.rev = AXP_REV_TYPHOON;
	sys->misc.nxs = AXP_NXS_CPU0;
	sys->misc.nxm = 0;
	sys->misc.res_25 = 0;
	sys->misc.acl = 0;
	sys->misc.abt = 0;
	sys->misc.abw = 0;
	sys->misc.ipreq = 0;
	sys->misc.ipintr = 0;
	sys->misc.itintr = 0;
	sys->misc.res_2 = 0;
	sys->misc.cpuID = AXP_CPUID_CPU0;

	/*
	 * Initialization for MPD (HRM Table 10-13)
	 */
	sys->mpd.res_4 = 0;
	sys->mpd.dr = AXP_MPD_SET;
	sys->mpd.ckr = AXP_MPD_SET;
	sys->mpd.ds = AXP_MPD_SET;
	sys->mpd.cks = AXP_MPD_SET;

	/*
	 * Initialization for AAR0, AAR1, AAR2, AAR3 (HRM Table 10-15)
	 */
	sys->aar0.res_35 = 0;
	sys->aar0.addr = 0;
	sys->aar0.res_17 = 0;
	sys->aar0.dbg = 0;
	sys->aar0.asiz = AXP_ASIZ_DISABLED;
	sys->aar0.res_10 = 0;
	sys->aar0.tsa = AXP_TSA_DISABLED;
	sys->aar0.sa = AXP_SA_DISABLED;
	sys->aar0.res_4 = 0;
	sys->aar0.rows = AXP_ROWS_11_BITS;
	sys->aar0.bnks = AXP_BNKS_1_BITS;

	sys->aar1.res_35 = 0;
	sys->aar1.addr = 0;
	sys->aar1.res_17 = 0;
	sys->aar1.dbg = 0;
	sys->aar1.asiz = AXP_ASIZ_DISABLED;
	sys->aar1.res_10 = 0;
	sys->aar1.tsa = AXP_TSA_DISABLED;
	sys->aar1.sa = AXP_SA_DISABLED;
	sys->aar1.res_4 = 0;
	sys->aar1.rows = AXP_ROWS_11_BITS;
	sys->aar1.bnks = AXP_BNKS_1_BITS;

	sys->aar2.res_35 = 0;
	sys->aar2.addr = 0;
	sys->aar2.res_17 = 0;
	sys->aar2.dbg = 0;
	sys->aar2.asiz = AXP_ASIZ_DISABLED;
	sys->aar2.res_10 = 0;
	sys->aar2.tsa = AXP_TSA_DISABLED;
	sys->aar2.sa = AXP_SA_DISABLED;
	sys->aar2.res_4 = 0;
	sys->aar2.rows = AXP_ROWS_11_BITS;
	sys->aar2.bnks = AXP_BNKS_1_BITS;

	sys->aar3.res_35 = 0;
	sys->aar3.addr = 0;
	sys->aar3.res_17 = 0;
	sys->aar3.dbg = 0;
	sys->aar3.asiz = AXP_ASIZ_DISABLED;
	sys->aar3.res_10 = 0;
	sys->aar3.tsa = AXP_TSA_DISABLED;
	sys->aar3.sa = AXP_SA_DISABLED;
	sys->aar3.res_4 = 0;
	sys->aar3.rows = AXP_ROWS_11_BITS;
	sys->aar3.bnks = AXP_BNKS_1_BITS;

	/*
	 * Initialization for DIM0, DIM1, DIM2, DIM3 (HRM Table 10-16)
	 */
	sys->dim0 = AXP_DIM_INTR_NONE;
	sys->dim1 = AXP_DIM_INTR_NONE;
	sys->dim2 = AXP_DIM_INTR_NONE;
	sys->dim3 = AXP_DIM_INTR_NONE;

	/*
	 * Initialization for DIR0, DIR1, DIR2, DIR3 (HRM Table 10-17)
	 */
	sys->dir0.err = 0;
	sys->dir0.res_56 = 0;
	sys->dir0.dev = 0;

	sys->dir1.err = 0;
	sys->dir1.res_56 = 0;
	sys->dir1.dev = 0;

	sys->dir2.err = 0;
	sys->dir2.res_56 = 0;
	sys->dir2.dev = 0;

	sys->dir3.err = 0;
	sys->dir3.res_56 = 0;
	sys->dir3.dev = 0;

	/*
	 * Initialization for DRIR (HRM Table 10-18)
	 */
	sys->drir = AXP_DRIR_INTR_NONE;

	/*
	 * Initialization for PRBEN (HRM Table 10-19)
	 */
	sys->prbEn.res_2 = 0;
	sys->prbEn.prben = AXP_PRBEN_DISABLED;

	/*
	 * Initialization for IIC0, IIC1, IIC2, IIC3 (HRM Table 10-20)
	 */
	sys->iic0.res_25 = 0;
	sys->iic0.of = AXP_OF_POSITIVE;
	sys->iic0.iCnt = 0;

	sys->iic1.res_25 = 0;
	sys->iic1.of = AXP_OF_POSITIVE;
	sys->iic1.iCnt = 0;

	sys->iic2.res_25 = 0;
	sys->iic2.of = AXP_OF_POSITIVE;
	sys->iic2.iCnt = 0;

	sys->iic3.res_25 = 0;
	sys->iic3.of = AXP_OF_POSITIVE;
	sys->iic3.iCnt = 0;

	/*
	 * Initialization for MPR0, MPR1, MPR2, MPR3 (HRM Table 10-22)
	 */
	sys->mpr0.res_13 = 0;
	sys->mpr0.mprdat = 0;

	sys->mpr1.res_13 = 0;
	sys->mpr1.mprdat = 0;

	sys->mpr2.res_13 = 0;
	sys->mpr2.mprdat = 0;

	sys->mpr3.res_13 = 0;
	sys->mpr3.mprdat = 0;

	/*
	 * Initialization for TTR (HRM Table 10-23)
	 */
	sys->ttr.res_15 = 0;
	sys->ttr.id = 7;
	sys->ttr.res_10 = 0;
	sys->ttr.irt = AXP_IRT_4_CYCLE;
	sys->ttr.res_6 = 0;
	sys->ttr.is = AXP_IS_4_CYCLE;
	sys->ttr.res_2 = 0;
	sys->ttr.ah = AXP_AH_1_CYCLE;
	sys->ttr.as = AXP_AS_1_CYCLE;

	/*
	 * Initialization for TDR (HRM Table 10-24)
	 */
	sys->tdr.wh3 = 0;
	sys->tdr.wp3 = 0;
	sys->tdr.res_58 = 0;
	sys->tdr.ws3 = 0;
	sys->tdr.res_55 = 0;
	sys->tdr.ra3 = 0;

	sys->tdr.wh2 = 0;
	sys->tdr.wp2 = 0;
	sys->tdr.res_42 = 0;
	sys->tdr.ws2 = 0;
	sys->tdr.res_39 = 0;
	sys->tdr.ra2 = 0;

	sys->tdr.wh1 = 0;
	sys->tdr.wp1 = 0;
	sys->tdr.res_26 = 0;
	sys->tdr.ws1 = 0;
	sys->tdr.res_23 = 0;
	sys->tdr.ra1 = 0;

	sys->tdr.wh0 = 0;
	sys->tdr.wp0 = 0;
	sys->tdr.res_10 = 0;
	sys->tdr.ws0 = 0;
	sys->tdr.res_7 = 0;
	sys->tdr.ra0 = 0;

	/*
	 * Initialization for PWR (HRM Table 10-25)
	 */
	sys->pwr.res_1 = 0;
	sys->pwr.sr = AXP_SR_NORMAL;

	/*
	 * Initialization for CMONCTLA (HRM Table 10-26)
	 */
	sys->cmonctla.res_62 = 0;
	sys->cmonctla.msk23 = 0;
	sys->cmonctla.res_50 = 0;
	sys->cmonctla.msk01 = 0;
	sys->cmonctla.stkdis3 = AXP_STKDIS_ALL_ONES;
	sys->cmonctla.stkdis2 = AXP_STKDIS_ALL_ONES;
	sys->cmonctla.stkdis1 = AXP_STKDIS_ALL_ONES;
	sys->cmonctla.stkdis0 = AXP_STKDIS_ALL_ONES;
	sys->cmonctla.res_34 = 0;
	sys->cmonctla.slctmbl = AXP_SLCTMBL_MGROUP0;
	sys->cmonctla.slct3 = 0;
	sys->cmonctla.slct2 = 0;
	sys->cmonctla.slct1 = 0;
	sys->cmonctla.slct0 = 0;

	/*
	 * Initialization for CMONCTLB (HRM Table 10-27)
	 */
	sys->cmonctlb.res_62 = 0;
	sys->cmonctlb.mte3 = 0;
	sys->cmonctlb.res_50 = 0;
	sys->cmonctlb.mte2 = 0;
	sys->cmonctlb.res_38 = 0;
	sys->cmonctlb.mte1 = 0;
	sys->cmonctlb.res_26 = 0;
	sys->cmonctlb.mte0 = 0;
	sys->cmonctlb.res_1 = 0;
	sys->cmonctlb.dis = AXP_DIS_IN_USE;

	/*
	 * Initialization for CMONCNT01 (HRM Table 10-29)
	 *
	 *	Table 10–28 Correspondence Between ECNT and MTE/MSK
	 *	-----------------------------------------------------------------------
	 *	Field to Increment		MTE Field Used		MSK Field Used
	 *	-----------------------------------------------------------------------
	 *	ECNT3					MTE3				MSK23
	 *	ECNT2					MTE2				MSK23
	 *	ECNT1					MTE1				MSK01
	 *	ECNT0					MTE0				MSK01
	 *	-----------------------------------------------------------------------
	 */
	sys->cmoncnt01.ecnt1 = 0;
	sys->cmoncnt01.ecnt0 = 0;

	/*
	 * Initialization for CMONCNT23 (HRM Table 10-30)
	 *
	 *	Table 10–28 Correspondence Between ECNT and MTE/MSK
	 *	-----------------------------------------------------------------------
	 *	Field to Increment		MTE Field Used		MSK Field Used
	 *	-----------------------------------------------------------------------
	 *	ECNT3					MTE3				MSK23
	 *	ECNT2					MTE2				MSK23
	 *	ECNT1					MTE1				MSK01
	 *	ECNT0					MTE0				MSK01
	 *	-----------------------------------------------------------------------
	 */
	sys->cmoncnt23.ecnt3 = 0;
	sys->cmoncnt23.ecnt2 = 0;

	/*
	 * Initialize the request queue.
	 */
	for (hh = 0; hh < AXP_21274_MAX_CPUS; hh++)
	{
		for (ii = 0; ii < AXP_21274_CCHIP_RQ_LEN; ii++)
		{
			for (jj = 0; jj < AXP_21274_DATA_SIZE; jj++)
			{
				sys->skidBuffer[ii].sysData[jj] = 0;
			}
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].mask;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].pa;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].cmd = Sysbus_NOP;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].status = HitClean;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].phase = phase0;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].entry = 0;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].cpuID = 0;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].sysDataLen = 0;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].waitVector = 0;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].miss2 = false;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].rqValid = false;
			sys->skidBuffer[(hh * AXP_21274_CCHIP_RQ_LEN) + ii].cacheHit = false;
		}
		sys->skidStart = sys->skidEnd = 0;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21274_Cbox_Main
 * 	This is the main function for the Cchip.  It looks at its queues to
 * 	determine if there is anything that needs to be processed from the CPUs or
 * 	devices (PCI).
 *
 * Input Parameters:
 * 	sys:
 * 		A pointer to the System structure for the emulated DECchip 21272/21274
 * 		chipsets.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void *AXP_21274_CchipMain(void *voidPtr)
{
	AXP_21274_SYSTEM	*sys = (AXP_21274_SYSTEM *) voidPtr;

	/*
	 * Log that we are starting.
	 */
	if (AXP_SYS_CALL)
	{
		AXP_TRACE_BEGIN();
		AXP_TraceWrite("Cchip is starting");
		AXP_TRACE_END();
	}

	/*
	 * First lock the Cchips mutex so that we can make sure to coordinate
	 * access to the Cchip's queues.
	 */
	pthread_mutex_lock(&sys->cChipMutex);

	/*
	 * TODO: Need to determine what the end condition for this loop should be.
	 */
	while (true)
	{

		/*
		 * The Cchip performs the following functions:
		 * 	- Accepts requests from the Pchips and the CPUs
		 * 	- Orders the arriving requests as required
		 * 	- Selects among the requests to issue controls to the DRAMs
		 * 	- Issues probes to the CPUs as appropriate to the selected requests
		 * 	- Translates CPU PIO addresses to PCI and CSR addresses
		 * 	- Issues commands to the Pchip as appropriate to the selected (PIO
		 * 	  or PTP) requests
		 * 	- Issues responses to the Pchip and CPU as appropriate to the
		 * 	  issued requests
		 * 	- Issues controls to the Dchip as appropriate to the DRAM accesses,
		 * 	  and the probe and Pchip responses
		 * 	- Controls the TIGbus to manage interrupts, and maintains CSRs
		 * 	  including those that represent interrupt status
		 *
		 * This first thing we need to do is wait for something to arrive to be
		 * processed.
		 */
		while (sys->skidStart == sys->skidEnd)
			pthread_cond_wait(&sys->cChipCond, &sys->cChipMutex);

		/*
		 * HRM 6.1.3 Request, Probe, and Data Ordering.
		 */

	}

	/*
	 * We are shutting down.  Since we started everything, we need
	 * to clean ourself up.  The main function will be joining to
	 * all the threads it created and then freeing up the memory
	 * and exiting the image.
	 */
	pthread_exit(NULL);
	return(NULL);
}
