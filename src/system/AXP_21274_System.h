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
 *	This header file contains the function prototypes for the System interface.
 *
 *	Revision History:
 *
 *	V01.000		31-Dec-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_SYSTEM_DEFS_
#define _AXP_SYSTEM_DEFS_	1

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_21274_SystemDefs.h"
#include "AXP_21274_Registers.h"

/*
 * System Data Structure.
 */
typedef struct
{

	/*
	 * Cchip Registers
	 */
	AXP_21274_CSC		csc;		/* Address: 801.a000.0000 */
	AXP_21274_MTR		mtr;		/* Address: 801.a000.0040 */
	AXP_21274_MISC		misc;		/* Address: 801.a000.0080 */
	AXP_21274_MPD		mpd;		/* Address: 801.a000.00c0 */
	AXP_21274_AARx		aar0;		/* Address: 801.a000.0100 */
	AXP_21274_AARx		aar1;		/* Address: 801.a000.0140 */
	AXP_21274_AARx		aar2;		/* Address: 801.a000.0180 */
	AXP_21274_AARx		aar3;		/* Address: 801.a000.01c0 */
	AXP_21274_DIMn		dim0;		/* Address: 801.a000.0200 */
	AXP_21274_DIMn		dim1;		/* Address: 801.a000.0240 */
	AXP_21274_DIRn		dir0;		/* Address: 801.a000.0280 */
	AXP_21274_DIRn		dir1;		/* Address: 801.a000.02c0 */
	AXP_21274_DRIR		drir;		/* Address: 801.a000.0300 */
	AXP_21274_PRBEN		prbEn;		/* Address: 801.a000.0340 */
	AXP_21274_IICn		iic0;		/* Address: 801.a000.0380 */
	AXP_21274_IICn		iic1;		/* Address: 801.a000.03c0 */
	AXP_21274_MPRn		mpr0;		/* Address: 801.a000.0400 */
	AXP_21274_MPRn		mpr1;		/* Address: 801.a000.0440 */
	AXP_21274_MPRn		mpr2;		/* Address: 801.a000.0480 */
	AXP_21274_MPRn		mpr3;		/* Address: 801.a000.04c0 */
	AXP_21274_TTR		ttr;		/* Address: 801.a000.0580 */
	AXP_21274_TDR		tdr;		/* Address: 801.a000.05c0 */
	AXP_21274_DIMn		dim2;		/* Address: 801.a000.0600 */
	AXP_21274_DIMn		dim3;		/* Address: 801.a000.0640 */
	AXP_21274_DIRn		dir2;		/* Address: 801.a000.0680 */
	AXP_21274_DIRn		dir3;		/* Address: 801.a000.06c0 */
	AXP_21274_IICn		iic2;		/* Address: 801.a000.0700 */
	AXP_21274_IICn		iic3;		/* Address: 801.a000.0740 */
	AXP_21274_PWR		pwr;		/* Address: 801.a000.0780 */
	AXP_21274_CMONCTLA	cmonctla;	/* Address" 801.a000.0c00 */
	AXP_21274_CMONCTLB	cmonctlb;	/* Address" 801.a000.0c40 */
	AXP_21274_CMONCNT01	cmoncnt01;	/* Address" 801.a000.0c80 */
	AXP_21274_CMONCNT23	cmoncnt23;	/* Address" 801.a000.0cc0 */

	/*
	 * Dchip Registers
	 */
	AXP_21274_DSC		dsc;		/* Address: 801.b000.0800 */
	AXP_21274_STR		str;		/* Address: 801.b000.0840 */
	AXP_21274_DREV		dRev;		/* Address: 801.b000.0880 */
	AXP_21274_DSC2		dsc2;		/* Address: 801.b000.08c0 */

	/*
	 * Pchip Registers
	 */
	AXP_21274_WSBAn		p0Wsba0;	/* Address: 801.8000.0000 */
	AXP_21274_WSBAn		p0Wsba1;	/* Address: 801.8000.0040 */
	AXP_21274_WSBAn		p0Wsba2;	/* Address: 801.8000.0080 */
	AXP_21274_WSBA3		p0Wsba4;	/* Address: 801.8000.00c0 */
	AXP_21274_WSMn		p0Wsm0;		/* Address: 801.8000.0100 */
	AXP_21274_WSMn		p0Wsm1;		/* Address: 801.8000.0140 */
	AXP_21274_WSMn		p0Wsm2;		/* Address: 801.8000.0180 */
	AXP_21274_WSMn		p0Wsm3;		/* Address: 801.8000.01c0 */
	AXP_21274_TBAn		p0Tba0;		/* Address: 801.8000.0200 */
	AXP_21274_TBAn		p0Tba1;		/* Address: 801.8000.0240 */
	AXP_21274_TBAn		p0Tba2;		/* Address: 801.8000.0280 */
	AXP_21274_TBAn		p0Tba3;		/* Address: 801.8000.02c0 */
	AXP_21274_PCTL		p0Pctl;		/* Address: 801.8000.0300 */
	AXP_21274_PLAT		p0Plat;		/* Address: 801.8000.0340 */
	/* AXP_21274_RES	p0Res;		 * Address: 801.8000.0380 */
	AXP_21274_PERROR	p0Perror;	/* Address: 801.8000.03c0 */
	AXP_21274_PERRMASK	p0PerMask;	/* Address: 801.8000.0400 */
	AXP_21274_PERRSET	p0PerrSet;	/* Address: 801.8000.0440 */
	AXP_21274_TLBIV		p0Tlbiv;	/* Address: 801.8000.0480 */
	AXP_21274_PMONCTL	p0MonCtl;	/* Address: 801.8000.0500 */
	AXP_21274_PMONCNT	p0MonCnt;	/* Address: 801.8000.0540 */
	AXP_21274_SPRST		p0SprSt;	/* Address: 801.8000.0800 */
	AXP_21274_WSBAn		p1Wsba0;	/* Address: 803.8000.0000 */
	AXP_21274_WSBAn		p1Wsba1;	/* Address: 803.8000.0040 */
	AXP_21274_WSBAn		p1Wsba2;	/* Address: 803.8000.0080 */
	AXP_21274_WSBA3		p1Wsba4;	/* Address: 803.8000.00c0 */
	AXP_21274_WSMn		p1Wsm0;		/* Address: 803.8000.0100 */
	AXP_21274_WSMn		p1Wsm1;		/* Address: 803.8000.0140 */
	AXP_21274_WSMn		p1Wsm2;		/* Address: 803.8000.0180 */
	AXP_21274_WSMn		p1Wsm3;		/* Address: 803.8000.01c0 */
	AXP_21274_TBAn		p1Tba0;		/* Address: 803.8000.0200 */
	AXP_21274_TBAn		p1Tba1;		/* Address: 803.8000.0240 */
	AXP_21274_TBAn		p1Tba2;		/* Address: 803.8000.0280 */
	AXP_21274_TBAn		p1Tba3;		/* Address: 803.8000.02c0 */
	AXP_21274_PCTL		p1Pctl;		/* Address: 803.8000.0300 */
	AXP_21274_PLAT		p1Plat;		/* Address: 803.8000.0340 */
	/* AXP_21274_RES	p1Res;		 * Address: 803.8000.0380 */
	AXP_21274_PERROR	p1Perror;	/* Address: 803.8000.03c0 */
	AXP_21274_PERRMASK	p1PerMask;	/* Address: 803.8000.0400 */
	AXP_21274_PERRSET	p1PerrSet;	/* Address: 803.8000.0440 */
	AXP_21274_TLBIV		p1Tlbiv;	/* Address: 803.8000.0480 */
	AXP_21274_PMONCTL	p1MonCtl;	/* Address: 803.8000.0500 */
	AXP_21274_PMONCNT	p1MonCnt;	/* Address: 803.8000.0540 */
	AXP_21274_SPRST		p1SprSt;	/* Address: 803.8000.0800 */
} AXP_21274_SYSTEM;

void AXP_System_CommandSend(
					AXP_21264_TO_SYS_CMD, bool, int, bool,
					u64, bool, u64, u8 *, int);
void AXP_System_ProbeResponse(bool, bool, u8, bool, u8, AXP_21264_PROBE_STAT);

#endif	/* _AXP_SYSTEM_DEFS_ */
