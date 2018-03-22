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
#include "AXP_21274_Cchip.h"

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

	/*
	 * Initialization for CSC (HRM Table 10-10)
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
	sys->csc.eft = 1;
	sys->csc.qdi = 0;
	sys->csc.fet = 2;
	sys->csc.qpm = 0;
	sys->csc.pme = 0;
	sys->csc.res_22 = 0;
	sys->csc.drtp = 3;
	sys->csc.dwfp = 3;
	sys->csc.dwtp = 3;
	sys->csc.res_15 = 0;
	sys->csc.pip = -1;	/* Powers up to the value present on the CAPREQ<1> pin */
	sys->csc.iddw = 3;
	sys->csc.iddr = 4;
	sys->csc.aw = 0;
	sys->csc.fw = -1;	/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */
	sys->csc.sfd = -1;	/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */
	sys->csc.sed = -1;	/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */
	sys->csc.c1cfp = -1;/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */
	sys->csc.c0cfp = -1;/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */
	sys->csc.bc = -1;	/* Byte 0 powers up to the value present on bits <7:0> of the TIGbus */

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
	sys->mtr.rrd = 0;
	sys->mtr.res_14 = 0;
	sys->mtr.rpt = 0;
	sys->mtr.res_10 = 0;
	sys->mtr.rpw = 0;
	sys->mtr.res_7 = 0;
	sys->mtr.ird = 0;
	sys->mtr.res_3 = 0;
	sys->mtr.cat = 0;
	sys->mtr.res_1 = 0;
	sys->mtr.rcd = 0;

	/*
	 * Initialization for MISC (HRM Table 10-12)
	 */
	sys->misc.res_44 = 0;
	sys->misc.devSup = 0;
	sys->misc.rev = 8;	/* Tsunami (21272) = 1, Typhoon (21274) = 8 */
	sys->misc.nxs = 0;
	sys->misc.nxm = 0;
	sys->misc.res_25 = 0;
	sys->misc.acl = 0;
	sys->misc.abt = 0;
	sys->misc.abw = 0;
	sys->misc.ipreq = 0;
	sys->misc.ipintr = 0;
	sys->misc.itintr = 0;
	sys->misc.res_2 = 0;
	sys->misc.cpuID = 0;

	/*
	 * Initialization for MPD (HRM Table 10-13)
	 */
	sys->mpd.res_4 = 0;
	sys->mpd.dr = 1;
	sys->mpd.ckr = 1;
	sys->mpd.ds = 1;
	sys->mpd.cks = 1;

	/*
	 * Initialization for AAR0, AAR1, AAR2, AAR3 (HRM Table 10-15)
	 */
	sys->aar0.res_35 = 0;
	sys->aar0.addr = 0;
	sys->aar0.res_17 = 0;
	sys->aar0.dbg = 0;
	sys->aar0.asiz = 0;
	sys->aar0.res_10 = 0;
	sys->aar0.tsa = 0;
	sys->aar0.sa = 0;
	sys->aar0.res_4 = 0;
	sys->aar0.rows = 0;
	sys->aar0.bnks = 0;

	sys->aar1.res_35 = 0;
	sys->aar1.addr = 0;
	sys->aar1.res_17 = 0;
	sys->aar1.dbg = 0;
	sys->aar1.asiz = 0;
	sys->aar1.res_10 = 0;
	sys->aar1.tsa = 0;
	sys->aar1.sa = 0;
	sys->aar1.res_4 = 0;
	sys->aar1.rows = 0;
	sys->aar1.bnks = 0;

	sys->aar2.res_35 = 0;
	sys->aar2.addr = 0;
	sys->aar2.res_17 = 0;
	sys->aar2.dbg = 0;
	sys->aar2.asiz = 0;
	sys->aar2.res_10 = 0;
	sys->aar2.tsa = 0;
	sys->aar2.sa = 0;
	sys->aar2.res_4 = 0;
	sys->aar2.rows = 0;
	sys->aar2.bnks = 0;

	sys->aar3.res_35 = 0;
	sys->aar3.addr = 0;
	sys->aar3.res_17 = 0;
	sys->aar3.dbg = 0;
	sys->aar3.asiz = 0;
	sys->aar3.res_10 = 0;
	sys->aar3.tsa = 0;
	sys->aar3.sa = 0;
	sys->aar3.res_4 = 0;
	sys->aar3.rows = 0;
	sys->aar3.bnks = 0;

	/*
	 * Initialization for DIM0, DIM1, DIM2, DIM3 (HRM Table 10-16)
	 */
	sys->dim0 = 0;
	sys->dim1 = 0;
	sys->dim2 = 0;
	sys->dim3 = 0;

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
	sys->drir = 0;

	/*
	 * Initialization for PRBEN (HRM Table 10-19)
	 */
	sys->prbEn.res_2 = 0;
	sys->prbEn.prben = 0;

	/*
	 * Initialization for IIC0, IIC1, IIC2, IIC3 (HRM Table 10-20)
	 */
	sys->iic0.res_25 = 0;
	sys->iic0.of = 0;
	sys->iic0.iCnt = 0;

	sys->iic1.res_25 = 0;
	sys->iic1.of = 0;
	sys->iic1.iCnt = 0;

	sys->iic2.res_25 = 0;
	sys->iic2.of = 0;
	sys->iic2.iCnt = 0;

	sys->iic3.res_25 = 0;
	sys->iic3.of = 0;
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
	sys->ttr.irt = 3;
	sys->ttr.res_6 = 0;
	sys->ttr.is = 3;
	sys->ttr.res_2 = 0;
	sys->ttr.ah = 0;
	sys->ttr.as = 0;

	/*
	 * Return back to the caller.
	 */
	return;
}
