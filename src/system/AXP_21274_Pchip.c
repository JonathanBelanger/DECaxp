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
 */
#include "AXP_21274_Pchip.h"

/*
 * AXP_21274_PchipInit
 *	This function is called to initialize the Pchip CSRs as documented in HRM
 *	10.2 Chipset Registers.
 *
 * Input Parameters:
 *	sys:
 *		A pointer to the system data structure from which the emulation
 *		information is maintained.
 *
 * Output Parameters:
 *	sys:
 *		A pointer to the system data structure, with the Pchip CSRs initialized
 *		based on the HRM for Tsunami/Typhoon chip set.
 *
 * Return Values:
 *	None.
 */
void AXP_21274_PchipInit(AXP_21274_SYSTEM *sys)
{

	/*
	 * Initialization for WSBA0, WSBA1,and WSBA2 (HRM Table 10-35), for both
	 * Pchip0 and Pchip1.
	 */
	sys->p0Wsba0.res_32 = 0;
	sys->p0Wsba0.addr = 0;
	sys->p0Wsba0.res_2 = 0;
	sys->p0Wsba0.sg = AXP_SG_DISABLE;
	sys->p0Wsba0.ena = AXP_ENA_DISABLE;

	sys->p0Wsba1.res_32 = 0;
	sys->p0Wsba1.addr = 0;
	sys->p0Wsba1.res_2 = 0;
	sys->p0Wsba1.sg = AXP_SG_DISABLE;
	sys->p0Wsba1.ena = AXP_ENA_DISABLE;

	sys->p0Wsba2.res_32 = 0;
	sys->p0Wsba2.addr = 0;
	sys->p0Wsba2.res_2 = 0;
	sys->p0Wsba2.sg = AXP_SG_DISABLE;
	sys->p0Wsba2.ena = AXP_ENA_DISABLE;

	sys->p1Wsba0.res_32 = 0;
	sys->p1Wsba0.addr = 0;
	sys->p1Wsba0.res_2 = 0;
	sys->p1Wsba0.sg = AXP_SG_DISABLE;
	sys->p1Wsba0.ena = AXP_ENA_DISABLE;

	sys->p1Wsba1.res_32 = 0;
	sys->p1Wsba1.addr = 0;
	sys->p1Wsba1.res_2 = 0;
	sys->p1Wsba1.sg = AXP_SG_DISABLE;
	sys->p1Wsba1.ena = AXP_ENA_DISABLE;

	sys->p1Wsba2.res_32 = 0;
	sys->p1Wsba2.addr = 0;
	sys->p1Wsba2.res_2 = 0;
	sys->p1Wsba2.sg = AXP_SG_DISABLE;
	sys->p1Wsba2.ena = AXP_ENA_DISABLE;

	/*
	 * Initialization for WSBA3 (HRM Table 10-36), for both Pchip0 and Pchip1.
	 */
	sys->p0Wsba3.res_40 = 0;
	sys->p0Wsba3.dac = AXP_DAC_DISABLE;
	sys->p0Wsba3.res_32 = 0;
	sys->p0Wsba3.addr = 0;
	sys->p0Wsba3.res_2 = 0;
	sys->p0Wsba3.sg = AXP_SG_ENABLE;
	sys->p0Wsba3.ena = AXP_ENA_DISABLE;

	sys->p1Wsba3.res_40 = 0;
	sys->p1Wsba3.dac = AXP_DAC_DISABLE;
	sys->p1Wsba3.res_32 = 0;
	sys->p1Wsba3.addr = 0;
	sys->p1Wsba3.res_2 = 0;
	sys->p1Wsba3.sg = AXP_SG_ENABLE;
	sys->p1Wsba3.ena = AXP_ENA_DISABLE;

	/*
	 * Initialization for WSM0, WSM1, WSM2, and WSM3 (HRM Table 10-37), for
	 * both Pchip0 and Pchip1.
	 */
	sys->p0Wsm0.res_32 = 0;
	sys->p0Wsm0.am = 0;
	sys->p0Wsm0.res_0 = 0;

	sys->p0Wsm1.res_32 = 0;
	sys->p0Wsm1.am = 0;
	sys->p0Wsm1.res_0 = 0;

	sys->p0Wsm2.res_32 = 0;
	sys->p0Wsm2.am = 0;
	sys->p0Wsm2.res_0 = 0;

	sys->p0Wsm3.res_32 = 0;
	sys->p0Wsm3.am = 0;
	sys->p0Wsm3.res_0 = 0;

	sys->p1Wsm0.res_32 = 0;
	sys->p1Wsm0.am = 0;
	sys->p1Wsm0.res_0 = 0;

	sys->p1Wsm1.res_32 = 0;
	sys->p1Wsm1.am = 0;
	sys->p1Wsm1.res_0 = 0;

	sys->p1Wsm2.res_32 = 0;
	sys->p1Wsm2.am = 0;
	sys->p1Wsm2.res_0 = 0;

	sys->p1Wsm3.res_32 = 0;
	sys->p1Wsm3.am = 0;
	sys->p1Wsm3.res_0 = 0;

	/*
	 * Initialization for TBA0, TBA1, and TBA2 (HRM Table 10-38), for both
	 * Pchip0 and Pchip1.
	 */
	sys->p0Tba0.res_35 = 0;
	sys->p0Tba0.addr = 0;
	sys->p0Tba0.res_0 = 0;

	sys->p0Tba1.res_35 = 0;
	sys->p0Tba1.addr = 0;
	sys->p0Tba1.res_0 = 0;

	sys->p0Tba2.res_35 = 0;
	sys->p0Tba2.addr = 0;
	sys->p0Tba2.res_0 = 0;

	sys->p1Tba0.res_35 = 0;
	sys->p1Tba0.addr = 0;
	sys->p1Tba0.res_0 = 0;

	sys->p1Tba1.res_35 = 0;
	sys->p1Tba1.addr = 0;
	sys->p1Tba1.res_0 = 0;

	sys->p1Tba2.res_35 = 0;
	sys->p1Tba2.addr = 0;
	sys->p1Tba2.res_0 = 0;

	/*
	 * Initialization for TBA3 (HRM Table 10-39), for both Pchip0 and Pchip1.
	 */
	sys->p0Tba3.res_35 = 0;
	sys->p0Tba3.addr = 0;
	sys->p0Tba3.res_0 = 0;

	sys->p1Tba3.res_35 = 0;
	sys->p1Tba3.addr = 0;
	sys->p1Tba3.res_0 = 0;

	/*
	 * Initialization for PCTL (HRM Table 10-40), for both Pchip0 and Pchip1.
	 */
	sys->p0Pctl.res_48 = 0;
	sys->p0Pctl.pid = 0;	/* Initialized from the PID pins */
	sys->p0Pctl.rpp = AXP_RPP_NOT_PRESENT;	/* Initialized from the CREQRMT_L pin at sys reset */
	sys->p0Pctl.ptevrfy = AXP_PTEVRFY_DISABLE;
	sys->p0Pctl.fdwdis = AXP_FDWDIS_NORMAL;
	sys->p0Pctl.fdsdis = AXP_FDSDIS_NORMAL;
	sys->p0Pctl.pclkx = AXP_PCLKX_6_TIMES; /* Initialized from the PCI i_pclkdiv<1:0> pins */
	sys->p0Pctl.ptpmax = 2;
	sys->p0Pctl.crqmax = 1;
	sys->p0Pctl.rev = 0;
	sys->p0Pctl.cdqmax = 1;
	sys->p0Pctl.padm = 0;	/* initialized from a decode of the b_cap<1:0> pins */
	sys->p0Pctl.eccen = AXP_ECCEN_DISABLE;
	sys->p0Pctl.res_16 = 0;
	sys->p0Pctl.ppri = AXP_PPRI_LOW;
	sys->p0Pctl.prigrp = AXP_PRIGRP_PICx_LOW;
	sys->p0Pctl.arbena = AXP_ARBENA_DISABLE;
	sys->p0Pctl.mwin = AXP_MWIN_DISABLE;
	sys->p0Pctl.hole = AXP_HOLE_DISABLE;
	sys->p0Pctl.tgtlat = AXP_TGTLAT_DISABLE;
	sys->p0Pctl.chaindis = AXP_CHAINDIS_DISABLE;
	sys->p0Pctl.thdis = AXP_THDIS_NORMAL;
	sys->p0Pctl.fbtb = AXP_FBTB_DISABLE;
	sys->p0Pctl.fdsc = AXP_FDSC_ENABLE;

	sys->p1Pctl.res_48 = 0;
	sys->p1Pctl.pid = 0;	/* Initialized from the PID pins */
	sys->p1Pctl.rpp = AXP_RPP_NOT_PRESENT;	/* Initialized from the CREQRMT_L pin at sys reset */
	sys->p1Pctl.ptevrfy = AXP_PTEVRFY_DISABLE;
	sys->p1Pctl.fdwdis = AXP_FDWDIS_NORMAL;
	sys->p1Pctl.fdsdis = AXP_FDSDIS_NORMAL;
	sys->p1Pctl.pclkx = AXP_PCLKX_6_TIMES; /* Initialized from the PCI i_pclkdiv<1:0> pins */
	sys->p1Pctl.ptpmax = 2;
	sys->p1Pctl.crqmax = 1;
	sys->p1Pctl.rev = 0;
	sys->p1Pctl.cdqmax = 1;
	sys->p1Pctl.padm = 0;	/* initialized from a decode of the b_cap<1:0> pins */
	sys->p1Pctl.eccen = AXP_ECCEN_DISABLE;
	sys->p1Pctl.res_16 = 0;
	sys->p1Pctl.ppri = AXP_PPRI_LOW;
	sys->p1Pctl.prigrp = AXP_PRIGRP_PICx_LOW;
	sys->p1Pctl.arbena = AXP_ARBENA_DISABLE;
	sys->p1Pctl.mwin = AXP_MWIN_DISABLE;
	sys->p1Pctl.hole = AXP_HOLE_DISABLE;
	sys->p1Pctl.tgtlat = AXP_TGTLAT_DISABLE;
	sys->p1Pctl.chaindis = AXP_CHAINDIS_DISABLE;
	sys->p1Pctl.thdis = AXP_THDIS_NORMAL;
	sys->p1Pctl.fbtb = AXP_FBTB_DISABLE;
	sys->p1Pctl.fdsc = AXP_FDSC_ENABLE;

	/*
	 * Initialization for PLAT (HRM Table 10-41), for both Pchip0 and Pchip1.
	 */
	sys->p0Plat.res_32 = 0;
	sys->p0Plat.res_16 = 0;
	sys->p0Plat.lat = 0;
	sys->p0Plat.res_0 = 0;

	sys->p1Plat.res_32 = 0;
	sys->p1Plat.res_16 = 0;
	sys->p1Plat.lat = 0;
	sys->p1Plat.res_0 = 0;

	/*
	 * Return back to the caller.
	 */
	return;
}
