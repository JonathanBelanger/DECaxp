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
#include "AXP_21274_21264_Common.h"
#include "AXP_21274_Registers.h"
#include "AXP_21274_Cchip.h"
#include "AXP_21274_Dchip.h"
#include "AXP_21274_Pchip.h"

/*
 * The following structure contains the information needed to be able to
 * communication with a CPU.
 */
typedef struct
{
	pthread_mutex_t		*mutex;
	pthread_cond_t		*cond;
	AXP_21274_CBOX_PQ	*pq;
	u8					*pqTop;
	u8					*pqBottom;
	u8					*irq_H;
} AXP_21274_CPU;

#define AXP_21274_MAX_CPUS		4
#define AXP_21274_MAX_ARRAYS	4

/*
 * HRM 2.1 System Building Block Variables
 *
 * The parameters that may be varied are as follows:
 *		- Number of CPUs (one or two)
 *		- Number of memory data buses (one or two)
 *		- Number of Dchips (two, four, or eight)
 *		- Number of Pchips (one or two)
 *		- Number of main memory DRAM arrays (one, two, three, or four)
 *		- Width of the memory data buses (16 bytes or 32 bytes each)
 *		- Type of DRAM SIMMs (synchronous 16MB or 64MB, with various timing
 *		  parameters)
 *
 * The combinations for possible system configurations are listed in Table 2?1.
 *
 *	Table 2?1 System Configurations
 *	--------------------------------------------------------------------------------------
 *										Pchip-to-
 *	Number of	Number of	Number of	Dchip Bus	Number		Number of		Memory Bus
 *	Cchips		Dchips		Pchips		Width		of CPUs		Memory Busses	Width
 *	--------------------------------------------------------------------------------------
 *		1			2		  1			4 bytes		  1			  1				16 bytes
 *		1			4		1 or 2		4 bytes		1 or 2		  1(1)			32 bytes
 *		1			4		1 or 2		4 bytes		1 or 2		  2(2)			16 bytes
 *		1			8		1 or 2		4 bytes		1 or 2		1 or 2(3)		32 bytes
 *		1			8		1 or 2		4 bytes		  4			1 or 2(3)		32 bytes
 *	--------------------------------------------------------------------------------------
 *	(1) Preferable for uniprocessors.
 *	(2) Preferable for dual processors.
 *	(3) Two memory buses are recommended when using two or four CPUs.
 *
 * The following notes also apply to Table 2?1.
 *		- A 32-byte memory bus can be half-populated, in which case, it
 *		  operates as a 16-byte memory bus. The difference is that the maximum
 *		  number of arrays on the bus is still four.
 *		- Using SDRAMs and a system clock speed of 83 MHz, 16-byte memory buses
 *		  each deliver 1.35-GB/s and 32-byte memory buses each deliver 2.7-GB/s
 *		  effective bandwidth.
 *		- The data path from the CPU to the Dchip is always 8 bytes, and can
 *		  run at 3 ns using clock forwarding for an effective bandwidth of
 *		  2.7-GB/s.
 *		- The PADbus (Pchip-to-Dchip) can run at 83 MHz for a raw bandwidth of
 *		  400-MB/s, ignoring turnaround cycles.
 *		- In a system with eight Dchips, each Dchip transfers 1 check bit, but
 *		  only ? byte per cycle. So the Pchip transfers 8 bytes with check bits
 *		  every two cycles over a 40-wire interface.
 *		- In a system with eight Dchips, the Dchips support up to four CPUs,
 *		  but the Cchip only supports one or two CPUs.
 *		- In a system with two memory buses, memory arrays 0 and 2 must be
 *		  attached to bus 0, while memory arrays 1 and 3 must be attached to
 *		  bus 1. The memory array number is determined by the set of DRAM
 *		  control signals from the Cchip. The memory bus number is determined
 *		  by the set of data signals on the Dchip slices (see Section 7.4).
 */

/*
 * System Data Structure.
 */
typedef struct
{

	/*
	 * This field needs to be at the top of all data blocks/structures
	 * that need to be specifically allocated by the Blocks module.
	 */
	AXP_BLOCK_DSC 			header;

	/*************************************************************************
	 * Cchip Data and Information											 *
	 *************************************************************************/
	pthread_t			cChipThreadID;
	pthread_mutex_t		cChipMutex;
	pthread_cond_t		cChipCond;
	AXP_QUEUE_HDR		skidBufferQ;
	AXP_21274_RQ_ENTRY	skidBuffers[AXP_21274_CCHIP_RQ_LEN * AXP_21274_MAX_CPUS];
	u32					skidLastUsed;
	AXP_21274_CPU		cpu[AXP_21274_MAX_CPUS];

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

	/*************************************************************************
	 * Dchip Data and Information											 *
	 *************************************************************************/
	pthread_t			dChipThreadID;
	pthread_mutex_t		dChipMutex;
	pthread_cond_t		dChipCond;
	u32					arrayCount;
	u64					*array[AXP_21274_MAX_ARRAYS];
	u64					arraySizes;

	/*
	 * Dchip Registers
	 */
	AXP_21274_DSC		dsc;		/* Address: 801.b000.0800 */
	AXP_21274_STR		str;		/* Address: 801.b000.0840 */
	AXP_21274_DREV		dRev;		/* Address: 801.b000.0880 */
	AXP_21274_DSC2		dsc2;		/* Address: 801.b000.08c0 */

	/*************************************************************************
	 * Pchip Data and Information											 *
	 *************************************************************************/
	pthread_t			p0ThreadID;
	pthread_mutex_t		p0Mutex;
	pthread_cond_t		p0Cond;
	pthread_t			p1ThreadID;
	pthread_mutex_t		p1Mutex;
	pthread_cond_t		p1Cond;

	/*
	 * Pchip Registers
	 */
	AXP_21274_WSBAn		p0Wsba0;	/* Address: 801.8000.0000 */
	AXP_21274_WSBAn		p0Wsba1;	/* Address: 801.8000.0040 */
	AXP_21274_WSBAn		p0Wsba2;	/* Address: 801.8000.0080 */
	AXP_21274_WSBA3		p0Wsba3;	/* Address: 801.8000.00c0 */
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
	AXP_21274_PERRMASK	p0PerrMask;	/* Address: 801.8000.0400 */
	AXP_21274_PERRSET	p0PerrSet;	/* Address: 801.8000.0440 */
	AXP_21274_TLBIV		p0Tlbiv;	/* Address: 801.8000.0480 */
	AXP_21274_PMONCTL	p0MonCtl;	/* Address: 801.8000.0500 */
	AXP_21274_PMONCNT	p0MonCnt;	/* Address: 801.8000.0540 */
	AXP_21274_SPRST		p0SprSt;	/* Address: 801.8000.0800 */
	AXP_21274_WSBAn		p1Wsba0;	/* Address: 803.8000.0000 */
	AXP_21274_WSBAn		p1Wsba1;	/* Address: 803.8000.0040 */
	AXP_21274_WSBAn		p1Wsba2;	/* Address: 803.8000.0080 */
	AXP_21274_WSBA3		p1Wsba3;	/* Address: 803.8000.00c0 */
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
	AXP_21274_PERRMASK	p1PerrMask;	/* Address: 803.8000.0400 */
	AXP_21274_PERRSET	p1PerrSet;	/* Address: 803.8000.0440 */
	AXP_21274_TLBIV		p1Tlbiv;	/* Address: 803.8000.0480 */
	AXP_21274_PMONCTL	p1MonCtl;	/* Address: 803.8000.0500 */
	AXP_21274_PMONCNT	p1MonCnt;	/* Address: 803.8000.0540 */
	AXP_21274_SPRST		p1SprSt;	/* Address: 803.8000.0800 */
} AXP_21274_SYSTEM;

#endif	/* _AXP_SYSTEM_DEFS_ */
