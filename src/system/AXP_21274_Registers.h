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
 *	This header file contains the definitions for the various registers defined
 *	in the Tsunami (21272) and Typhoon (21274) chipsets.
 *
 * Revision History:
 *
 *	V01.000		18-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_REGISTERS_H_
#define _AXP_21274_REGISTERS_H_

/*
 * Definitions for bits that are either on or off, but do not mean any thing
 * else, really.
 */
#define AXP_BIT_OFF					0
#define AXP_BIT_ON					1

/****************************************************************************
 *							HRM 10.2.2 Cchip CSRs							*
 ****************************************************************************/

/*
 * HRM 10.2.2.1 - Cchip Configuration Register (CSC - RW) (page 10-19)
 *
 *	Note:	Follow the rules listed in Chapter 12 when writing to this CSR.
 *			After writing to this register, a delay is required to ensure that
 *			subsequent accesses to the 21272 will succeed.
 *
 * All fields in CSC are read/write except for those in the two low-order
 * bytes, which are read-only. Bits <7:0> are initialized from the pins of the
 * Cchip on power-up. Bits <13:8> are written whenever the Dchip register STR
 * is written. This ensures that the Cchip and Dchip versions of these fields
 * are always synchronized, because the system will not function correctly if
 * they are not the same. Table 10?9 describes the Tsunami Cchip configuration
 * register (CSC). Table 10?10 describes the Typhoon Cchip configuration
 * register (CSC).  Below is the Typhoon CSC, as this is a superset of the
 * Tsunami CSC.
 *
 *	Table 10-10 Cchip System Configuration Register (CSC) (Typhoon Only)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES 		<63> 		MBZ,RAZ	0		Reserved.
 *	RES			<62>		MBZ,RAZ	0		Reserved.
 *	P1W(1)		<61>		RO		1		= Wide PADbus 1. (Typhoon only)
 *	P0W(1)		<60>		RO		1		= Wide PADbus 0. (Typhoon only)
 *	RES			<59>		MBZ,RAZ	0		Reserved.
 *	PBQMAX(2)	<58:56>		RW		1		CPU probe queue maximum - 0
 *											indicates 8 entries.
 *	RES			<55>		MBZ,RAZ	0		Reserved.
 *	PRQMAX		<54:52>		RW		2		Maximum requests to one Pchip until
 *											ACK, modulo 8 - the value 1 is
 *											illegal. Pchip Rev. 0 allows up to
 *											4.
 *	RES			<51>		MBZ,RAZ	0		Reserved.
 *	PDTMAX		<50:48>		RW		1		Maximum data transfers to one Pchip
 *											until ACK, modulo 8. Pchip Rev. 0
 *											allows up to 2.
 *	RES			<47>		MBZ,RAZ	0		Reserved.
 *	FPQPMAX		<46:44>		RW		1		Maximum entries in FPQ on Dchips
 *											known to Pchips, modulo 8. Dchip
 *											Rev. 0 allows up to 4. Must be the
 *											same as Pchip CSR PCTL <CDQMAX>.
 *	RES			<43>		MBZ,RAZ	0		Reserved.
 *	FPQCMAX		<42:40>		RW		1		True maximum entries in Dchip FPQ,
 *											modulo 8. Must be same as Pchip CSR
 *											PCTL<CDQMAX>.
 *	AXD			<39>		RW		0		Disable memory XOR. (Typhoon only)
 *	TPQMMAX		<38:36>		RW		1		Maximum entries in TPQM on Dchips,
 *											modulo 8 (2 Dchips = 4, 4 or 8
 *											Dchips = 8).
 *	B3D			<35>		RW		0		Bypass 3 issue path disable
 *	B2D			<34>		RW		0		Bypass 2 issue path disable
 *	B1D			<33>		RW		0		Bypass 1 issue path disable
 *	FTI(2)		<32>		RW		0		Force throttle issue
 *	EFT			<31>		RW		1		Extract to fill turnaround cycles.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		0 cycles
 *											1		1 cycle
 *											-----------------------------------
 *	QDI			<30:28>		RW		0		Queue drain interval
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		Disable draining
 *											1		1024 cycles
 *											2		256 cycles
 *											3		64 cycles
 *											4		16 cycles
 *											5		1 cycles
 *											6		Reserved
 *											7		Reserved
 *											-----------------------------------
 *	FET			<27:26>		RW		2		Fill to extract turnaround cycles.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		1 cycle - SED must be 2 or
 *													3 cycles.
 *											1		2 cycles - SED must be 3,
 *													4, or 5 cycles.
 *											2		3 cycles - SED must be 3,
 *													4, or 5 cycles.
 *											3		Reserved.
 *											-----------------------------------
 *	QPM			<25>		RW		0		Queue priority mode
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		Round robin
 *											1		Modified round robin
 *											-----------------------------------
 *	PME			<24>		RW		0		Page mode enable.
 *	RES			<23:22>		RO		0		Reserved.
 *	DRTP		<21:20>		RW		3		Minimum delay through Dchip from
 *											memory bus to PADbus.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2 cycles (rev 0 Dchip)
 *											1		3 cycles
 *											2		4 cycles
 *											3		5 cycles
 *											-----------------------------------
 *	DWFP		<19:18>		RW		3		Minimum delay through Dchip from
 *											PADbus to CPU or memory bus.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2 cycles
 *											1		3 cycles (rev 0 Dchip)
 *											2		4 cycles
 *											3		5 cycles
 *											-----------------------------------
 *	DWTP		<17:16>		RW		3		Minimum delay through Dchip from
 *											CPU bus to PADbus.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2 cycles
 *											1		3 cycles
 *											2		4 cycles (rev 0 Dchip)
 *											3		5 cycles
 *											-----------------------------------
 *	RES			<15>		MBZ,RAZ	0		Reserved.
 *	P1P(3)		<14>		RO				Pchip 1 present.
 *	IDDW		<13:12>		RO(4)	3		Issue to data delay for all
 *											transactions except memory reads
 *											(see Table 7-5).
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		3 cycles
 *											1		4 cycles
 *											2		5 cycles
 *											3		6 cycles
 *											-----------------------------------
 *	IDDR		<11:9>		RO(4)	4		Issue to data delay for memory
 *											reads (see Table 7-5).
 *											-----------------------------------
 *											Value Cycles
 *											-----------------------------------
 *											0		5 cycles
 *											1		6 cycles
 *											2		7 cycles
 *											3		8 cycles
 *											4		9 cycles
 *											5		10 cycles
 *											6		11 cycles
 *											7		Reserved
 *											-----------------------------------
 *	AW			<8>			RO(4)	0		Array width.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		16 bytes
 *											1		32 bytes
 *											-----------------------------------
 *	FW(5)		<7>			RO				Available for firmware.
 *	SFD(5)		<6>			RO				SysDC fill delay. The number of
 *											cycles from the SysDC cycle to the
 *											first CPU data cycle when moving
 *											data into the CPU.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		2 cycles
 *											1		3 cycles
 *											-----------------------------------
 *	SED(5)		<5:4>		RO				SysDC extract delay - The number of
 *											cycles from the SysDC cycle to the
 *											first CPU data cycle when moving
 *											data out of the CPU.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2 cycles
 *											1		3 cycles
 *											2		4 cycles
 *											3		5 cycles
 *											-----------------------------------
 *	C1CFP(5)	<3>			RO				CPU1 clock forward preset (see
 *											Chapter 11).
 *	C0CFP(5)	<2>			RO				CPU0 clock forward preset (see
 *											Chapter 11).
 *	BC5			<1:0>		RO				Base configuration.
 *											-----------------------------------
 *											Value	Configuration
 *											-----------------------------------
 *											0		2 Dchips, 1 memory bus
 *											1		4 Dchips, 1 memory bus
 *											2		4 Dchips, 2 memory buses
 *											3		8 Dchips, 2 memory buses
 *	---------------------------------------------------------------------------
 *	(1) Loaded from CAPbus<13:12> during reset.
 *	(2) The combination of PBQMAX = 1 and FTI = 0 is illegal.
 *	(3) Powers up to the value present on the CAPREQ<1> pin.
 *	(4) These fields are updated when the Dchip STR register is written.
 *	(5) Byte 0 powers up to the value present on bits <7:0> of the TIGbus.
 */
typedef struct
{
	u64	bc		: 2;	/* Base configuration */
	u64	c0cfp	: 1;	/* CPU0 clock forward preset (see Chapter 11) */
	u64	c1cfp	: 1;	/* CPU1 clock forward preset (see Chapter 11) */
	u64	sed		: 2;	/* SysDC extract delay */
	u64	sfd		: 1;	/* SysDC fill delay */
	u64	fw		: 1;	/* Available for firmware */
	u64	aw		: 1;	/* Array width */
	u64	iddr	: 3;	/* Issue to data delay for memory reads */
	u64	iddw	: 2;	/* Issue to data delay for all but memory reads */
	u64	pip		: 1;	/* Pchip 1 preset */
	u64	res_15	: 1;	/* Reserved at bit 15 */
	u64	dwtp	: 2;	/* Minimum delay through Dchip from CPU bus to PADbus */
	u64	dwfp	: 2;	/* Minimum delay through Dchip from PADbus to CPU bus */
	u64	drtp	: 2;	/* Minimum delay through Dchip from Memory to PADbus */
	u64	res_22	: 2;	/* Reserved at bits 23:22 */
	u64	pme		: 1;	/* Page mode enable */
	u64	qpm		: 1;	/* Queue priority mode */
	u64	fet		: 2;	/* Fill to extract turnaround cycles */
	u64	qdi		: 3;	/* Queue drain interval */
	u64	eft		: 1;	/* Extract to fill turnaround cycles */
	u64	fti		: 1;	/* Force throttle issue */
	u64	b1d		: 1;	/* Bypass 1 issue path disable */
	u64	b2d		: 1;	/* Bypass 2 issue path disable */
	u64	b3d		: 1;	/* Bypass 3 issue path disable */
	u64	tpqmmax	: 3;	/* Maximum entries in TPQM on Dchips, modulo 8 */
	u64	axd		: 1;	/* Disable memory XOR (Typhoon only) */
	u64	fpqcmax	: 3;	/* Maximum entries in Dchip FPQ, modulo 8 */
	u64	res_43	: 1;	/* Reserved at bit 43 */
	u64	fpqpmax	: 3;	/* Maximum entries in FPQ on Dchips known to Pchips, modulo 8 */
	u64	res_47	: 1;	/* Reserved at bit 47 */
	u64	pdtmax	: 3;	/* Maximum data transfers to one Pchip until ACK, modulo 8 */
	u64	res_51	: 1;	/* Reserved at bit 51 */
	u64	prqmax	: 3;	/* Maximum requests to one Pchip until ACK, modulo 8 */
	u64	res_55	: 1;	/* Reserved at bit 55 */
	u64	pbqmax	: 3;	/* CPU probe queue maximum - 0 indicates 8 entries */
	u64	res_59	: 1;	/* Reserved at bit 59 */
	u64	p0w		: 1;	/* Wide PADbus 0 (Typhoon only) */
	u64	p1w		: 1;	/* Wide PADbus 1 (Typhoon only) */
	u64	res_62	: 1;	/* Reserved at bit 62 */
	u64	res_63	: 1;	/* Reserved at bit 63 */
} AXP_21274_CSC;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_CSC_RMASK	0x377777FFFF3F7FFF
#define AXP_21274_CSC_WMASK	0x077777FFFF3F0000

/*
 * Definitions for the values in various fields in the CSC.
 */
#define AXP_EFT_0_CYCLES			0
#define AXP_EFT_1_CYCLES			1
#define AXP_QDI_DISABLE_DRAINING	0
#define AXP_QDI_1024_CYCLES			1
#define AXP_QDI_256_CYCLES			2
#define AXP_QDI_64_CYCLES			3
#define AXP_QDI_16_CYCLES			4
#define AXP_QDI_1_CYCLES			5
#define AXP_FET_1_CYCLE				0
#define AXP_FET_2_CYCLE				1
#define AXP_FET_3_CYCLE				2
#define AXP_QPM_ROUND_ROBIN			0
#define AXP_QPM_MODIFIED_RR			1
#define AXP_DRTP_2_CYCLES			0	/* (rev 0 Dchip) */
#define AXP_DRTP_3_CYCLES			1
#define AXP_DRTP_4_CYCLES			2
#define AXP_DRTP_5_CYCLES			3
#define AXP_DWFP_2_CYCLES			0
#define AXP_DWFP_3_CYCLES			1	/* (rev 0 Dchip) */
#define AXP_DWFP_4_CYCLES			2
#define AXP_DWFP_5_CYCLES			3
#define AXP_DWTP_2_CYCLES			0
#define AXP_DWTP_3_CYCLES			1
#define AXP_DWTP_4_CYCLES			2	/* (rev 0 Dchip) */
#define AXP_DWTP_5_CYCLES			3
#define AXP_IDDW_3_CYCLES			0
#define AXP_IDDW_4_CYCLES			1
#define AXP_IDDW_5_CYCLES			2
#define AXP_IDDW_6_CYCLES			3
#define AXP_IDDR_5_CYCLES			0
#define AXP_IDDR_6_CYCLES			1
#define AXP_IDDR_7_CYCLES			2
#define AXP_IDDR_8_CYCLES			3
#define AXP_IDDR_9_CYCLES			4
#define AXP_IDDR_10_CYCLES			5
#define AXP_IDDR_11_CYCLES			6
#define AXP_AW_16_BYTES				0
#define AXP_AW_32_BYTES				1
#define AXP_SFD_2_CYCLES			0
#define AXP_SFD_3_CYCLES			1
#define AXP_SED_2_CYCLES			0
#define AXP_SED_3_CYCLES			1
#define AXP_SED_4_CYCLES			2
#define AXP_SED_5_CYCLES			3
#define AXP_BC_2D_1M				0	/* 2 Dchips, 1 memory bus */
#define AXP_BC_4D_1M				1	/* 4 Dchips, 1 memory bus */
#define AXP_BC_4D_2M				2	/* 4 Dchips, 2 memory buses */
#define AXP_BC_8D_2M				3	/* 8 Dchips, 1 memory buses */

/*
 * HRM 10.2.2.2 - Memory Timing Register(MTR - RW) (page 10-26)
 *
 *	Table 10-11 Memory Timing Register (MTR) (Continued)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:46>		MBX,RAZ	0		Reserved.
 *	MPH			<45:40>		RW		0		Maximum page hits - The most page
 *											hits the memory controller allows
 *											before forcing a page to be closed.
 *											The issue unit can, under some
 *											circumstances, sneak in one more
 *											page hit than this parameter
 *											allows.
 *	PHCW		<39:36>		RW		14		Page hit cycles for writes - The
 *											number of cycles that the memory
 *											controller must wait after a write
 *											is issued until it attempts to
 *											close the page.
 *
 *											Note: Must be greater than or equal
 *											to the greater of (IRD + RPW - 2)
 *											and (IRD + RCD + bl + tRWL - 3),
 *											where bl is 4 for 16-byte buses and
 *											2 for 32-byte buses.
 *
 *											Note: Initializes as 14 in the Rev.
 *											C Cchip; initializes as 15 in the
 *											Rev. B Cchip.
 *	PHCR		<35:32>		RW		15		Page hit cycles for reads - The
 *											number of cycles that the memory
 *											controller must wait after a read
 *											is issued until it attempts to
 *											close the page.
 *
 *											Note: Must be greater than or equal
 *											to the greater of (RPW - 2) and
 *											(RCD + bl - 2) where bl is 4 for
 *											16-byte buses and 2 for 32-byte
 *											buses.
 *	RES			<31:30>		MBZ,RAZ	0		Reserved.
 *	RI			<29:24>		RW		0		Refresh interval - The number of
 *											cycles per refresh interval divided
 *											by 64. Each DRAM is refreshed once
 *											per refresh interval. A value of 0
 *											disables refreshing.  The values 1,
 *											2, and 3 are illegal.
 *	RES			<23:21>		MBZ,RAZ	0		Reserved.
 *	MPD			<20>		RW		0		Mask pipeline delay - The
 *											b_mndqm<1:0> signals to the SDRAMs
 *											may need to be buffered. Setting
 *											this bit causes the memory
 *											controllers to signal the DQM masks
 *											one cycle earlier to compensate.
 *	RES			<19:17>		MBZ,RAZ	0		Reserved.
 *	RRD			<16>		RW		0		Minimum same-array different-bank
 *											RAS-to-RAS delay - The minimum
 *											number of cycles from the Row
 *											Activate or Refresh command to the
 *											next Row Activate or Refresh
 *											command to the other bank of the
 *											same array.
 *											-----------------------------------
 *											Value	Delay
 *											-----------------------------------
 *											0		2
 *											1		3
 *											-----------------------------------
 *	RES			<15:14>		MBZ,RAZ	0		Reserved.
 *	RPT			<13:12>		RW		0		Minimum RAS precharge time - The
 *											minimum number of cycles from a
 *											Precharge command to the next Row
 *											Activate or Refresh command to the
 *											same bank.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2
 *											1		3
 *											2		4
 *											3		Reserved
 *											-----------------------------------
 *	RES			<11:10>		MBZ,RAZ	0		Reserved.
 *	RPW			<9:8>		RW		0		Minimum RAS pulse width (tRAS) -
 *											The minimum number of cycles from a
 *											Row Activate or Refresh command to
 *											the next Precharge command to the
 *											same bank. Used by the refresh
 *											logic only. PHCW and PHCR are used
 *											by the page-hit logic.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		4
 *											1		5
 *											2		6
 *											3		7
 *											-----------------------------------
 *	RES			<7>			MBZ,RAZ	0		Reserved.
 *	IRD			<6:4>		RW		0		Issue to RAS delay - For memory
 *											writes this is the number of cycles
 *											from when the arbitrator issues the
 *											request, to when the DRAM row is
 *											activated. See Section 10.2.4.3 for
 *											the correct value to use.
 *											-----------------------------------\
 *											Value	Cycles
 *											-----------------------------------
 *											0		0
 *											1		1
 *											2		2
 *											3		3
 *											4		4
 *											5		5
 *											6		Reserved
 *											7		Reserved
 *											-----------------------------------
 *	RES			<3>			MBZ,RAZ	0		Reserved.
 *	CAT			<2>			RW		0		CAS access time - The number of
 *											cycles from the CAS command to when
 *											data appears on the DRAM outputs.
 *											Must be greater than or equal to
 *											RCD.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2
 *											1		3
 *											-----------------------------------
 *	RES			<1>			MBZ,RAZ	0		Reserved.
 *	RCD			<0>			RW		0		RAS-to-CAS delay - The number of
 *											cycles from the Row Activate
 *											command to the next Read or Write
 *											command to the same bank. Must be
 *											less than or equal to CAT.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		2
 *											1		3
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	rcd		: 1;	/* RAS-to-CAS delay */
	u64	res_1	: 1;	/* Reserved at bit 1 */
	u64	cat		: 1;	/* CAS access time */
	u64	res_3	: 1;	/* Reserved at bit 3 */
	u64	ird		: 3;	/* Issue to RAS delay */
	u64	res_7	: 1;	/* Reserved at bit 7 */
	u64	rpw		: 2;	/* Minimum RAS plus width (tRAS) */
	u64	res_10	: 2;	/* Reserved at bits 11:10 */
	u64	rpt		: 2;	/* Minimum RAS precharge time */
	u64	res_14	: 2;	/* Reserved at bits 14:15 */
	u64	rrd		: 1;	/* Minimum same-array different-bank RAS-to-RAS delay */
	u64	res_17	: 3;	/* Reserved at bits 19:17 */
	u64	mpd		: 1;	/* Mask pipeline delay */
	u64	res_21	: 3;	/* Reserved at bits 23:21 */
	u64	ri		: 6;	/* Refresh interval */
	u64	res_30	: 2;	/* Reserved at bits 31:30 */
	u64	phcr	: 4;	/* Page hit cycles for reads */
	u64	phcw	: 4;	/* Page hit cycles for writes */
	u64	mph		: 6;	/* Maximum page hits */
	u64 res_46	: 18;	/* Reserved at bits 63:46 */
} AXP_21274_MTR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_MTR_RMASK	0x00003FFF3F113375
#define AXP_21274_MTR_WMASK	0x00003FFF3F113375

/*
 * Definitions for the values in various fields in the MTR.
 */
#define AXP_MPD__NO_DELAY			0
#define AXP_MPD_ONE_PIPELINE_STAGE	1
#define AXP_RRD_2_CYCLES			0
#define AXP_RRD_3_CYCLES			1
#define AXP_RPT_2_CYCLES			0
#define AXP_RPT_3_CYCLES			1
#define AXP_RPT_4_CYCLES			2
#define AXP_RPW_4_CYCLES			0
#define AXP_RPW_5_CYCLES			1
#define AXP_RPW_6_CYCLES			2
#define AXP_RPW_7_CYCLES			3
#define AXP_IRD_0_CYCLES			0
#define AXP_IRD_1_CYCLES			1
#define AXP_IRD_2_CYCLES			2
#define AXP_IRD_3_CYCLES			3
#define AXP_IRD_4_CYCLES			4
#define AXP_IRD_5_CYCLES			5
#define AXP_CAT_2_CYCLES			0
#define AXP_CAT_3_CYCLES			1
#define AXP_RCD_2_CYCLES			0
#define AXP_RCD_3_CYCLES			1

/*
 * HRM 10.2.2.3 Miscellaneous Register (MISC - RW) (page 10-29)
 *
 * This register is designed so that there are no read side effects, and that
 * writing a 0 to any bit has no effect. Therefore, when software wants to
 * write a 1 to any bit in the register, it need not be concerned with
 * read-modify-write or the status of any other bits in the register.  Once NXM
 * is set, the NXS field is locked so that initial NXM error information is not
 * overwritten by subsequent errors. It is unlocked when software clears the
 * NXM field. The ABW (arbitration won) field is locked if either ABW bit is
 * set, so the first CPU to write it locks out the other CPU. Writing a 1 to
 * ACL (arbitration clear) clears both ABW bits and both ABT (arbitration try)
 * bits and unlocks the ABW field. Table 10-12 describes the miscellaneous
 * register (MISC).
 *
 * 	Table 10-12 Miscellaneous Register (MISC)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:44>		MBZ,RAZ	0		Reserved.
 *	DEVSUP		<43:40>		WO		0		Suppress IRQ1 (device) interrupts
 *											to the CPU corresponding to a 1 in
 *											this field until the interrupt
 *											polling machine has completed a
 *											poll of all PCI devices.
 *											(<43:42> are used in Typhoon only)
 *	REV			<39:32>		RO		-		Latest revision of Cchip:
 *									1		21272 (Tsunami)
 *									8		21274 (Typhoon)
 *	NXS			<31:29>		RO		0		NXM source - Device that caused the
 *											NXM - UNPREDICTABLE if NXM is not
 *											set.
 *											-----------------------------------
 *											Value	Source
 *											-----------------------------------
 *											0		CPU 0
 *											1		CPU 1
 *											2		Reserved
 *													CPU2 - Typhoon only
 *											3		Reserved
 *													CPU3 - Typhoon only
 *											4		Pchip 0
 *											5		Pchip 1
 *											6, 7	Reserved
 *											-----------------------------------
 *	NXM			<28>		R,W1C	0		Nonexistent memory address
 *											detected.  Sets DRIR<63> and locks
 *											the NXS field until it is cleared.
 *	RES			<27:25>		MBZ,RAZ	0		Reserved.
 *	ACL			<24>		WO		0		Arbitration clear - writing a 1 to
 *											this bit clears the ABT and ABW
 *											fields.
 *	ABT			<23:20>		R,W1S	0		Arbitration try - writing a 1 to
 *											these bits sets them. (<23:22> are
 *											used in Typhoon only)
 *	ABW			<19:16>		R,W1S	0		Arbitration won - writing a 1 to
 *											these bits sets them unless one is
 *											already set, in which case the
 *											write is ignored.
 *											(<19:18> are used in Typhoon only)
 *	IPREQ		<15:12>		WO		0		Interprocessor interrupt request -
 *											write a 1 to the bit corresponding
 *											to the CPU you want to interrupt.
 *											Writing a 1 here sets the
 *											corresponding bit in the IPINTR.
 *											(<15:14> are used in Typhoon only)
 *	IPINTR		<11:8>		R,W1C	0		Interprocessor interrupt pending -
 *											one bit per CPU. Pin irq<3> is
 *											asserted to the CPU corresponding
 *											to a 1 in this field.
 *											(<11:10> are used in Typhoon only)
 *	ITINTR		<7:4>		R,W1C	0		Interval timer interrupt pending -
 *											one bit per CPU. Pin IRQ2 is
 *											asserted to the CPU corresponding
 *											to a 1 in this field.
 *											(<7:6> are used in Typhoon only)
 *	RES			<3:2>		MBZ,RAZ	0		Reserved.
 *	CPUID		<1:0>		RO		-		ID of the CPU performing the read.
 *											(<1> are used in Typhoon only)
 */
typedef struct
{
	u64	cpuID	: 2;	/* ID of the CPU performing the read */
	u64	res_2	: 2;	/* Reserved at bits 3:2 */
	u64	itintr	: 4;	/* Interval timer interrupt pending - one bit per CPU */
	u64	ipintr	: 4;	/* Interprocessor interrupt pending - one bit per CPU */
	u64	ipreq	: 4;	/* Interprocessor interrupt request - one bit per CPU */
	u64	abw		: 4;	/* Arbitration won - one bit per CPU */
	u64	abt		: 4;	/* Arbitration try - one bit per CPU */
	u64	acl		: 1;	/* Arbitration clear */
	u64	res_25	: 3;	/* Reserved at bits 27:25 */
	u64	nxm		: 1;	/* Nonexistent memory address detected */
	u64	nxs		: 3;	/* NXM source */
	u64	rev		: 8;	/* Latest revision of Cchip */
	u64	devSup	: 4;	/* Suppress IRQ 1 (device) interrupts to the CPU */
	u64	res_44	: 20;	/* Reserved at bits 63:44 */
} AXP_21274_MISC;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, Read-WriteOneClear, and Read-WriteOneSet.  Reserved is masked
 * out for both reads and writes.  Read-Only are masked out for writes.
 * Write-only are masked out for reads.  Read-WriteOneClear are masked in for
 * reads and certain writes where writing a 1 will actually clear the bit.
 * Read-WriteOneSet are masked in for reads and certain writes where writing
 * a 1 will set the bit.
 */
#define AXP_21274_MISC_RMASK	0x000000FFF0FF0FF3
#define AXP_21274_MISC_WOMASK	0x00000F000100F000
#define AXP_21274_MISC_W1SMASK	0x0000000000FF0000
#define AXP_21274_MISC_W1CMASK	0x0000000010000FF0

/*
 * Definitions for the values in various fields in the MISC.
 */
#define AXP_DEVSUP_CPU0				0x1	/*  0  0  0  1 */
#define AXP_DEVSUP_CPU1				0x2	/*  0  0  1  0 */
#define AXP_DEVSUP_CPU2				0x4	/*  0  1  0  0 */
#define AXP_DEVSUP_CPU3				0x8	/*  1  0  0  0 */
#define AXP_REV_TSUNAMI				1
#define AXP_REV_TYPHOON				8
#define AXP_NXS_CPU0				0
#define AXP_NXS_CPU1				1
#define AXP_NXS_CPU2				2
#define AXP_NXS_CPU3				3
#define AXP_NXS_PCHIP0				4
#define AXP_NXS_PCHIP1				5
#define AXP_ABT_CPU0				0x1	/*  0  0  0  1 */
#define AXP_ABT_CPU1				0x2	/*  0  0  0  1 */
#define AXP_ABT_CPU2				0x4	/*  0  0  0  1 */
#define AXP_ABT_CPU3				0x8	/*  0  0  0  1 */
#define AXP_ABW_CPU0				0x1	/*  0  0  0  1 */
#define AXP_ABW_CPU1				0x2	/*  0  0  0  1 */
#define AXP_ABW_CPU2				0x4	/*  0  0  0  1 */
#define AXP_ABW_CPU3				0x8	/*  0  0  0  1 */
#define AXP_IPREQ_CPU0				0x1	/*  0  0  0  1 */
#define AXP_IPREQ_CPU1				0x2	/*  0  0  0  1 */
#define AXP_IPREQ_CPU2				0x4	/*  0  0  0  1 */
#define AXP_IPREQ_CPU3				0x8	/*  0  0  0  1 */
#define AXP_IPINTR_CPU0				0x1	/*  0  0  0  1 */
#define AXP_IPINTR_CPU1				0x2	/*  0  0  0  1 */
#define AXP_IPINTR_CPU2				0x4	/*  0  0  0  1 */
#define AXP_IPINTR_CPU3				0x8	/*  0  0  0  1 */
#define AXP_ITINTR_CPU0				0x1	/*  0  0  0  1 */
#define AXP_ITINTR_CPU1				0x2	/*  0  0  0  1 */
#define AXP_ITINTR_CPU2				0x4	/*  0  0  0  1 */
#define AXP_ITINTR_CPU3				0x8	/*  0  0  0  1 */
#define AXP_CPUID_CPU0				0
#define AXP_CPUID_CPU1				1
#define AXP_CPUID_CPU2				2
#define AXP_CPUID_CPU3				3

/*
 * HRM 10.2.2.4 Memory Presence Detect Register (MPD - RW)
 *
 * The memory presence detect register is connected to two open-drain pins on
 * the Cchip.  These pins can be used by software to implement the I2C protocol
 * to read the serial presence detect pins on the SDRAM DIMMs. Table 10-13
 * describes the memory presence detect register (MPD).
 *
 * Table 10-13 Memory Presence Detect Register (MPD)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:4>		MBZ,RAZ	0		Reserved
 *	DR			<3>			RO		1		Data receive
 *	CKR			<2>			RO		1		Clock receive
 *	DS			<1>			WO		1		Data send - Must be a 1 to receive
 *	CKS			<0>			WO		1		Clock send
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	cks		: 1;	/* Clock send */
	u64	ds		: 1;	/* Data send - Must be a 1 to receive */
	u64	ckr		: 1;	/* Clock receive */
	u64	dr		: 1;	/* Data receive */
	u64	res_4	: 60;	/* Reserved at bits 63:4 */
} AXP_21274_MPD;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_MPD_RMASK	0x000000000000000C
#define AXP_21274_MPD_WMASK	0x0000000000000003

/*
 * Definitions for the values in various fields in the MPD.
 */
#define AXP_MPD_CLEAR				0
#define AXP_MPD_SET					1

/*
 * HRM 10.2.2.5 Array Address Register (AAR0, AAR1, AAR2, AAR3 - RW)
 *
 *	Table 10-15 Array Address Register (AAR0, AAR1, AAR2, AAR3) (Typhoon Only)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:35>		MBZ,RAZ	0		Reserved.
 *	ADDR		<34:24>		RW		0		Base address - Bits <34:24> of the
 *											physical byte address of the first
 *											byte in the array.
 *											(<27:24> are used in Tsunami only;
 *											<31:24> are valid.)
 *											(<34:32> are used in Typhoon only;
 *											<34:28> are valid)
 *	RES			<23:17>		MBZ,RAZ	0		Reserved.
 *	DBG			<16>		RW		0		Enables this memory port to be used
 *											as a debug interface.
 *	ASIZ		<15:12>		RW		0		Array size (<15> is used in Typhoon
 *											only).
 *											-----------------------------------
 *											Value	Size
 *											-----------------------------------
 *											0000	0 (bank disabled)
 *											0001	16MB
 *											0010	32MB
 *											0011	64MB
 *											0100	128MB
 *											0101	256MB
 *											0110	512MB
 *											0111	1GB
 *											1000	2GB (Reserved Tsunami;
 *														Typhoon only)
 *											1001	4GB (Reserved Tsunami;
 *														Typhoon only)
 *											1010	8GB (Reserved Tsunami;
 *														Typhoon only)
 *											1011-	Reserved.
 *											 1111
 *											-----------------------------------
 *	RES			<11:10>		MBZ,RAZ	0		Reserved.
 *	TSA			<9>			RW		0		Twice-split array (Typhoon only)
 *	SA			<8>			RW		0		Split array.
 *	RES			<7:4>		MBZ,RAZ	0		Reserved.
 *	ROWS		<3:2>		RW		0		Number of row bits in the SDRAMs.
 *											-----------------------------------
 *											Value	Number of Bits
 *											-----------------------------------
 *											0		11
 *											1		12
 *											2		13
 *											3		Reserved
 *											-----------------------------------
 *	BNKS		<1:0>		RW		0		Number of bank bits in the SDRAMs
 *											-----------------------------------
 *											Value	Number of Bits
 *											-----------------------------------
 *											0		1
 *											1		2
 *											2		3 (Typhoon only)
 *											3		Reserved
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	bnks	: 2;	/* Number of bank bits in the SDRAMs */
	u64	rows	: 2;	/* Number of row bits in the SDRAM */
	u64	res_4	: 4;	/* Reserved at bits 7:4 */
	u64	sa		: 1;	/* Split array */
	u64	tsa		: 1;	/* Twice-split array (Typhoon only).  Reserved (Tsunami) */
	u64	res_10	: 2;	/* Reserved at bits 11:10 */
	u64	asiz	: 4;	/* Array size */
	u64	dbg		: 1;	/* Enables this memory port to be used as a debug interface */
	u64	res_17	: 7;	/* Reserved at bits 23:17 */
	u64	addr	: 11;	/* Base address */
	u64	res_35	: 29;	/* Reserved at bits 63:35 */
} AXP_21274_AARx;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_ARRx_RMASK	0x00000007FF01F30F
#define AXP_21274_ARRx_WMASK	0x00000007FF01F30F

/*
 * Definitions for the values in various fields in the AARx.
 */
#define AXP_ADDR_MASK_TSUNAMI	0x00000000ff000000ll
#define AXP_ADDR_MASK_TYPHOON	0x00000007f0000000ll
#define AXP_ASIZ_DISABLED		0
#define AXP_ASIZ_16MB			1
#define AXP_ASIZ_32MB			2
#define AXP_ASIZ_64MB			3
#define AXP_ASIZ_128MB			4
#define AXP_ASIZ_256MB			5
#define AXP_ASIZ_512MB			6
#define AXP_ASIZ_1GB			7
#define AXP_ASIZ_2GB			8
#define AXP_ASIZ_4GB			9
#define AXP_ASIZ_8GB			10
#define AXP_TSA_DISABLED		0
#define AXP_TSA_ENABLED			1
#define AXP_SA_DISABLED			0
#define AXP_SA_ENABLED			1
#define AXP_ROWS_11_BITS		0
#define AXP_ROWS_12_BITS		1
#define AXP_ROWS_13_BITS		2
#define AXP_BNKS_1_BITS			0
#define AXP_BNKS_2_BITS			1
#define AXP_BNKS_3_BITS			2

/*
 * HRM 10.2.2.6 Device Interrupt Mask Register (DIMn, n=0,3 - RW)
 *
 * Register n applies to CPUn. (Typhoon only: n=2,3.)
 *
 * These four mask registers control which interrupts are allowed to go through
 * to the CPUs. No interrupt in DRIR will get through to the masked interrupt
 * registers (and on to interrupt the CPUs) unless the corresponding mask bit
 * is set in DIMn. All bits are initialized to 0 at reset. Table 10-16
 * describes the device interrupt mask registers.
 *
 *	Table 10-16 Device Interrupt Mask Register (DIMn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	DIM 		<63:0>		RW		0		Interrupts allowed through to the
 *											CPU
 *	---------------------------------------------------------------------------
 */
typedef u64 AXP_21274_DIMn;

/*
 * Definitions for the values in various fields in the DIMx.
 */
#define AXP_DIM_INTR_NONE	0x0000000000000000ll
#define AXP_DIM_INTR_0		0x0000000000000001ll
#define AXP_DIM_INTR_1		0x0000000000000002ll
#define AXP_DIM_INTR_2		0x0000000000000004ll
#define AXP_DIM_INTR_3		0x0000000000000008ll
#define AXP_DIM_INTR_4		0x0000000000000010ll
#define AXP_DIM_INTR_5		0x0000000000000020ll
#define AXP_DIM_INTR_6		0x0000000000000040ll
#define AXP_DIM_INTR_7		0x0000000000000080ll
#define AXP_DIM_INTR_8		0x0000000000000100ll
#define AXP_DIM_INTR_9		0x0000000000000200ll
#define AXP_DIM_INTR_10		0x0000000000000400ll
#define AXP_DIM_INTR_11		0x0000000000000800ll
#define AXP_DIM_INTR_12		0x0000000000001000ll
#define AXP_DIM_INTR_13		0x0000000000002000ll
#define AXP_DIM_INTR_14		0x0000000000004000ll
#define AXP_DIM_INTR_15		0x0000000000008000ll
#define AXP_DIM_INTR_16		0x0000000000010000ll
#define AXP_DIM_INTR_17		0x0000000000020000ll
#define AXP_DIM_INTR_18		0x0000000000040000ll
#define AXP_DIM_INTR_19		0x0000000000080000ll
#define AXP_DIM_INTR_20		0x0000000000100000ll
#define AXP_DIM_INTR_21		0x0000000000200000ll
#define AXP_DIM_INTR_22		0x0000000000400000ll
#define AXP_DIM_INTR_23		0x0000000000800000ll
#define AXP_DIM_INTR_24		0x0000000001000000ll
#define AXP_DIM_INTR_25		0x0000000002000000ll
#define AXP_DIM_INTR_26		0x0000000004000000ll
#define AXP_DIM_INTR_27		0x0000000008000000ll
#define AXP_DIM_INTR_28		0x0000000010000000ll
#define AXP_DIM_INTR_29		0x0000000020000000ll
#define AXP_DIM_INTR_30		0x0000000040000000ll
#define AXP_DIM_INTR_31		0x0000000080000000ll
#define AXP_DIM_INTR_32		0x0000000100000000ll
#define AXP_DIM_INTR_33		0x0000000200000000ll
#define AXP_DIM_INTR_34		0x0000000400000000ll
#define AXP_DIM_INTR_35		0x0000000800000000ll
#define AXP_DIM_INTR_36		0x0000001000000000ll
#define AXP_DIM_INTR_37		0x0000002000000000ll
#define AXP_DIM_INTR_38		0x0000004000000000ll
#define AXP_DIM_INTR_39		0x0000008000000000ll
#define AXP_DIM_INTR_40		0x0000010000000000ll
#define AXP_DIM_INTR_41		0x0000020000000000ll
#define AXP_DIM_INTR_42		0x0000040000000000ll
#define AXP_DIM_INTR_43		0x0000080000000000ll
#define AXP_DIM_INTR_44		0x0000100000000000ll
#define AXP_DIM_INTR_45		0x0000200000000000ll
#define AXP_DIM_INTR_46		0x0000400000000000ll
#define AXP_DIM_INTR_47		0x0000800000000000ll
#define AXP_DIM_INTR_48		0x0001000000000000ll
#define AXP_DIM_INTR_49		0x0002000000000000ll
#define AXP_DIM_INTR_50		0x0004000000000000ll
#define AXP_DIM_INTR_51		0x0008000000000000ll
#define AXP_DIM_INTR_52		0x0010000000000000ll
#define AXP_DIM_INTR_53		0x0020000000000000ll
#define AXP_DIM_INTR_54		0x0040000000000000ll
#define AXP_DIM_INTR_55		0x0080000000000000ll
#define AXP_DIM_INTR_56		0x0100000000000000ll
#define AXP_DIM_INTR_57		0x0200000000000000ll
#define AXP_DIM_INTR_58		0x0400000000000000ll
#define AXP_DIM_INTR_59		0x0800000000000000ll
#define AXP_DIM_INTR_60		0x1000000000000000ll
#define AXP_DIM_INTR_61		0x2000000000000000ll
#define AXP_DIM_INTR_62		0x4000000000000000ll
#define AXP_DIM_INTR_63		0x8000000000000000ll

/*
 * HRM 10.2.2.7 Device Interrupt Request Register (DIRn, n=0,3 - RO)
 *
 * Register n applies to CPUn. (Typhoon only: n=2,3.)
 *
 * These two registers indicate which interrupts are pending to the CPUs. If a
 * raw request bit is set and the corresponding mask bit is set, then the
 * corresponding bit in this register will be set and the appropriate CPU will
 * be interrupted. Table 10-17 describes the device interrupt request
 * registers.
 *
 *	Table 10-17 Device Interrupt Request Register (DIRn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	ERR			<63:58>		RO		0		IRQ0 error interrupts
 *											<63> Chip detected MISC<NXM>
 *											<62> recommended hookup to Pchip0
 *											error
 *											<61> recommended hookup to Pchip1
 *											error
 *											Others per module designer-s choice
 *	RES			<57:56>		RO		0		Reserved
 *	DEV			<55:0>		RO		0		IRQ1 PCI interrupts pending to the
 *											CPU
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	dev		: 56;	/* IRQ1 PCI interrupts pending to the CPU */
	u64	res_56	: 2;	/* Reserved at bits 57:56 */
	u64	err		: 6;	/* IRQ0 error interrupts */
} AXP_21274_DIRn;

/*
 * The following macros mask in bits that are Read-Only.
 */
#define AXP_21274_DIRn_RMASK	0xFCFFFFFFFFFFFFFF

/*
 * Definitions for the values in various fields in the DIRx.
 */
#define AXP_ERR_NXM					0x20
#define AXP_ERR_PCHIP0				0x10
#define AXP_ERR_PCHIP1				0x80
/* TODO: What are we supposed to do with the DEV field */

/*
 * HRM 10.2.2.8 Device Raw Interrupt Request Register (DRIR - RO)
 *
 * DRIR indicates which of the 64 possible device interrupts is asserted. Table
 * 10-18 describes the device raw interrupt request register (DRIR).
 *
 *	Table 10-18 Device Interrupt Mask Register (DIMn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	DRIR 		<63:0>		RW		0		Interrupts pending from devices
 *	---------------------------------------------------------------------------
 */
typedef u64 AXP_21274_DRIR;

/*
 * Definitions for the values in various fields in the DRIR.
 */
#define AXP_DRIR_INTR_NONE	0x0000000000000000ll
#define AXP_DRIR_INTR_0		0x0000000000000001ll
#define AXP_DRIR_INTR_1		0x0000000000000002ll
#define AXP_DRIR_INTR_2		0x0000000000000004ll
#define AXP_DRIR_INTR_3		0x0000000000000008ll
#define AXP_DRIR_INTR_4		0x0000000000000010ll
#define AXP_DRIR_INTR_5		0x0000000000000020ll
#define AXP_DRIR_INTR_6		0x0000000000000040ll
#define AXP_DRIR_INTR_7		0x0000000000000080ll
#define AXP_DRIR_INTR_8		0x0000000000000100ll
#define AXP_DRIR_INTR_9		0x0000000000000200ll
#define AXP_DRIR_INTR_10	0x0000000000000400ll
#define AXP_DRIR_INTR_11	0x0000000000000800ll
#define AXP_DRIR_INTR_12	0x0000000000001000ll
#define AXP_DRIR_INTR_13	0x0000000000002000ll
#define AXP_DRIR_INTR_14	0x0000000000004000ll
#define AXP_DRIR_INTR_15	0x0000000000008000ll
#define AXP_DRIR_INTR_16	0x0000000000010000ll
#define AXP_DRIR_INTR_17	0x0000000000020000ll
#define AXP_DRIR_INTR_18	0x0000000000040000ll
#define AXP_DRIR_INTR_19	0x0000000000080000ll
#define AXP_DRIR_INTR_20	0x0000000000100000ll
#define AXP_DRIR_INTR_21	0x0000000000200000ll
#define AXP_DRIR_INTR_22	0x0000000000400000ll
#define AXP_DRIR_INTR_23	0x0000000000800000ll
#define AXP_DRIR_INTR_24	0x0000000001000000ll
#define AXP_DRIR_INTR_25	0x0000000002000000ll
#define AXP_DRIR_INTR_26	0x0000000004000000ll
#define AXP_DRIR_INTR_27	0x0000000008000000ll
#define AXP_DRIR_INTR_28	0x0000000010000000ll
#define AXP_DRIR_INTR_29	0x0000000020000000ll
#define AXP_DRIR_INTR_30	0x0000000040000000ll
#define AXP_DRIR_INTR_31	0x0000000080000000ll
#define AXP_DRIR_INTR_32	0x0000000100000000ll
#define AXP_DRIR_INTR_33	0x0000000200000000ll
#define AXP_DRIR_INTR_34	0x0000000400000000ll
#define AXP_DRIR_INTR_35	0x0000000800000000ll
#define AXP_DRIR_INTR_36	0x0000001000000000ll
#define AXP_DRIR_INTR_37	0x0000002000000000ll
#define AXP_DRIR_INTR_38	0x0000004000000000ll
#define AXP_DRIR_INTR_39	0x0000008000000000ll
#define AXP_DRIR_INTR_40	0x0000010000000000ll
#define AXP_DRIR_INTR_41	0x0000020000000000ll
#define AXP_DRIR_INTR_42	0x0000040000000000ll
#define AXP_DRIR_INTR_43	0x0000080000000000ll
#define AXP_DRIR_INTR_44	0x0000100000000000ll
#define AXP_DRIR_INTR_45	0x0000200000000000ll
#define AXP_DRIR_INTR_46	0x0000400000000000ll
#define AXP_DRIR_INTR_47	0x0000800000000000ll
#define AXP_DRIR_INTR_48	0x0001000000000000ll
#define AXP_DRIR_INTR_49	0x0002000000000000ll
#define AXP_DRIR_INTR_50	0x0004000000000000ll
#define AXP_DRIR_INTR_51	0x0008000000000000ll
#define AXP_DRIR_INTR_52	0x0010000000000000ll
#define AXP_DRIR_INTR_53	0x0020000000000000ll
#define AXP_DRIR_INTR_54	0x0040000000000000ll
#define AXP_DRIR_INTR_55	0x0080000000000000ll
#define AXP_DRIR_INTR_56	0x0100000000000000ll
#define AXP_DRIR_INTR_57	0x0200000000000000ll
#define AXP_DRIR_INTR_58	0x0400000000000000ll
#define AXP_DRIR_INTR_59	0x0800000000000000ll
#define AXP_DRIR_INTR_60	0x1000000000000000ll
#define AXP_DRIR_INTR_61	0x2000000000000000ll
#define AXP_DRIR_INTR_62	0x4000000000000000ll
#define AXP_DRIR_INTR_63	0x8000000000000000ll

/*
 * HRM 10.2.2.9 Probe Enable Register (PRBEN - RW)
 *
 * This register is special in that reads do not return the value of the
 * register, but rather cause the probe enable bit for the requesting CPU to be
 * cleared. The return data is UNPREDICTABLE. Writing to this register causes
 * the probe enable bit for the requesting CPU to be set, regardless of the
 * value written. Table 10-19 describes the probe enable register (PRBEN).
 *
 *	Table 10-19 Probe Enable Register (PRBEN)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:4>		MBZ		0		Reserved
 *	PRBEN3		<3>			RTC,WTS	0		Probe enable bit for CPU3
 *	PRBEN2		<2>			RTC,WTS	0		Probe enable bit for CPU2
 *	PRBEN1		<1>			RTC,WTS	0		Probe enable bit for CPU1
 *	PRBEN0		<0>			RTC,WTS	0		Probe enable bit for CPU0
 *	---------------------------------------------------------------------------
 *	NOTE:	This register is read from to clean the bit and written to to set
 *			the bit.  It is never returned or anything.  So, to help things
 *			along, I introduced 3 more bit, so that there is now one bit for
 *			each CPU.  This is different than the HRM, but this is actually a
 *			pseudo-register and is never returned on reading.
 */
typedef struct
{
	u64	prben0	: 1;	/* Probe enable bit CPU0 */
	u64	prben1	: 1;	/* Probe enable bit CPU1 */
	u64	prben2	: 1;	/* Probe enable bit CPU2 */
	u64	prben3	: 1;	/* Probe enable bit CPU3 */
	u64	res_2	: 60;	/* Reserved at bits 63:4 */
} AXP_21274_PRBEN;

/*
 * Definitions for the values in various fields in the PRBEN.
 */
#define AXP_PRBEN_DISABLED			0
#define AXP_PRBEN_ENABLED			1

/*
 * HRM Interval Ignore Count Register (IICn, n=0,3 - RW)
 *
 * Register n applies to CPUn. (Typhoon only: n=2,3.)
 *
 * These registers are used for 21264 CPU sleep mode. They are written with a
 * count of how many interval timer interrupts to suppress and count down to 0
 * as subsequent interval timer interrupts are asserted. They can be read at
 * any time to find the remaining count. After the count has been decremented
 * to 0, the next interval timer interrupt will be sent through to the CPU.
 * After the wake-up tick is received, the count goes negative, and the OF bit
 * is set on the next timer interval tick. This allows the CPU to determine
 * exactly how many interval timer ticks were skipped. Table 10-20 describes
 * the interval ignore count register (IIC).
 *
 *	Table 10-20 Interval Ignore Count Register (IIC)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:25>		MBZ,RAZ	0		Reserved
 *	OF			<24>		RO		0		Overflow - Indicates negative count
 *	ICNT		<23:0>		RW		0		Count of remaining interrupts to
 *											ignore
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	iCnt	: 24;	/* Count of remaining interrupts to ignore */
	u64 of		: 1;	/* Overflow, indicates negative count */
	u64	res_25	: 39;
} AXP_21274_IICn;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_IICn_RMASK	0x0000000001FFFFFF
#define AXP_21274_IICn_WMASK	0x0000000000FFFFFF

/*
 * Definitions for the values in various fields in the IICn.
 */
#define AXP_OF_POSITIVE			0
#define AXP_OF_NEGATIVE			1

/*
 * HRM 10.2.2.11 Wake-Up Delay Register (WDR - RW)
 *
 * The WDR register determines how long (in system cycles) the chipset waits
 * after a reset, or after sending a wake-up interrupt to a sleeping CPU,
 * before deasserting b_cfrst<1:0>.
 *
 *	Table 10-21 Wake-Up Delay Register (WDR)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:25>		MBZ		0		Reserved
 *	WDR			<24:0>		RW		0		Wake-up delay (in cycles)
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	wdr		: 25;	/* Wake-up delay (in cycles) */
	u64	res_25	: 39;	/* Reserved at bits 63:25 */
} AXP_21274_WDR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_WDR_RMASK	0x0000000001FFFFFF
#define AXP_21274_WDR_WMASK	0x0000000001FFFFFF

/*
 * HRM 10.2.2.12 Memory Programming Register (MPR0, MPR1, MPR2, MPR3 - WO)
 *
 * A write to these registers causes a RAM program cycle (a mode register set
 * command) to the associated memory array using the data written to the MPRDAT
 * field. Table 10-22 describes the memory programming registers.
 *
 *	Table 10-22 Memory Programming Register (MPRn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:13>		MBZ		0		Reserved
 *	MPRDAT		<12:0>		WO		-		Data to be written on address lines
 *											<12:0>
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	mprdat	: 13;	/* Data to be written on address lines <12:0> */
	u64	res_13	: 51;	/* Reserved at bits 63:13 */
} AXP_21274_MPRn;

/*
 * The following macro mask in Reserved, and Write-Only.  Reserved is masked
 * out.
 */
#define AXP_21274_MPRn_WMASK	0x0000000000003FFF

/*
 * HRM 10.2.2.13 M-Port Control Register (MCTL - MBZ)
 *
 * The M-port control register controls chipset debug features. It must be 0
 * for normal operations. In Typhoon, this register is replaced by the CMONCTL
 * registers.
 */
typedef u64 AXP_21274_MCTL;

/*
 * HRM 10.2.2.14 TIGbus Timing Register (TTR - RW)
 *
 * The TIGbus timing register controls the nonaddress-specific timing of the
 * TIGbus.  Table 10-23 describes the TIGbus timing register (TTR). See Section
 * 6.3 for timing diagrams and information.
 *
 *	Table 10-23 TIGbus Timing Register (TTR - RW)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:15>		MBZ,RAZ	0		Reserved.
 *	ID			<14:12>		RW		7		Interrupt starting device - If
 *											there are fewer than eight
 *											interrupt buffers present on the
 *											module, this field determines the
 *											lowest-order byte number that gets
 *											read in - Devices <7:ID> all
 *											present is a requirement.
 *	RES			<11:10>		MBZ,RAZ	0		Reserved.
 *	IRT			<9:8>		RW		3		Interrupt read time - The number of
 *											cycles that the interrupt driver is
 *											enabled on the TIGbus before the
 *											interrupt data is latched in the
 *											Cchip.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		1 cycle
 *											1		2 cycles
 *											2		3 cycles
 *											3		4 cycles
 *											-----------------------------------
 *	RES			<7:6>		MBZ,RAZ	0		Reserved.
 *	IS			<5:4>		RW		3		Interrupt setup time - The number
 *											of cycles that the Cchip drives the
 *											IRQ data on the TIGbus before
 *											asserting b_tis to strobe the data
 *											into a register on the module.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		1 cycle
 *											1		2 cycles
 *											2		3 cycles
 *											3		4 cycles
 *											-----------------------------------
 *	RES			<3:2>		MBZ,RAZ	0		Reserved
 *	AH			<1>			RW		0		Address hold after as_l before
 *											cs_l.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		1 cycle
 *											1		2 cycles
 *											-----------------------------------
 *	AS			<0>			RW		0		Address setup to the address latch
 *											before as_l.
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		1 cycle
 *											1		2 cycles
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	as		: 1;	/* Address setup to the address latch before as_1 */
	u64	ah		: 1;	/* Address hold after as_1 before cs_1 */
	u64	res_2	: 2;	/* Reserved at bits 3:2 */
	u64	is		: 2;	/* Interrupt setup time */
	u64	res_6	: 2;	/* Reserved at bits 7:6 */
	u64	irt		: 2;	/* Interrupt read time */
	u64	res_10	: 2;	/* Reserved at bits 11:10 */
	u64	id		: 3;	/* Interrupt starting device */
	u64 res_15	: 49;	/* Reserved at bits 63:15 */
} AXP_21274_TTR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_TTR_RMASK	0x0000000000007333
#define AXP_21274_TTR_WMASK	0x0000000000007333

/*
 * Definitions for the values in various fields in the TTR.
 */
#define AXP_IRT_1_CYCLE			0
#define AXP_IRT_2_CYCLE			1
#define AXP_IRT_3_CYCLE			2
#define AXP_IRT_4_CYCLE			3
#define AXP_IS_1_CYCLE			0
#define AXP_IS_2_CYCLE			1
#define AXP_IS_3_CYCLE			2
#define AXP_IS_4_CYCLE			3
#define AXP_AH_1_CYCLE			0
#define AXP_AH_2_CYCLE			1
#define AXP_AS_1_CYCLE			0
#define AXP_AS_2_CYCLE			1

/*
 * HRM 10.2.2.15 TIGbus Device Timing Register (TDR - RW)
 *
 * One 16-bit field of this register is selected by TIG address bits <23:22> to\
 * allow up to four different timing domains on the TIGbus. All values in the
 * register are expressed in cycles. The state machine stays in each state one
 * cycle longer than the number in the register (that is, a value of 0 means
 * one cycle). See Section 6.3 for timing diagrams describing these fields.
 *
 * Table 10-24 describes the TIGbus device timing register (TDR).
 *
 *	Table 10-24 TIGbus Device Timing Register (TDR)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	WH3			<63>		RW		0		See bit WH0
 *	WP3			<62:60>		RW		0		See bit WP0
 *	RES			<59:58>		MBZ,RAZ	0		Reserved
 *	WS3			<57:56>		RW		0		See bit WS0
 *	RES			<55>		MBZ,RAZ	0		Reserved
 *	RD3			<54:52>		RW		0		See bit RD0
 *	RA3			<51:48>		RW		0		See bit RA0
 *	WH2			<47>		RW		0		See bit WH0
 *	WP2			<46:44>		RW		0		See bit WP0
 *	RES			<43:42>		MBZ,RAZ	0		Reserved
 *	WS2			<41:40>		RW		0		See bit WS0
 *	RES			<39>		MBZ,RAZ	0		Reserved
 *	RD2			<38:36>		RW		0		See bit RD0
 *	RA2			<35:32>		RW		0		See bit RA0
 *	WH1			<31>		RW		0		See bit WH0
 *	WP1			<30:28>		RW		0		See bit WP0
 *	RES			<27:26>		MBZ,RAZ	0		Reserved
 *	WS1			<25:24>		RW		0		See bit WS0
 *	RES			<23>		MBZ,RAZ	0		Reserved
 *	RD1			<22:20>		RW		0		See bit RD0
 *	RA1			<19:16>		RW		0		See bit RA0
 *	WH0			<15>		RW		0		Write hold time - The number of
 *											cycles that cs_l, the address, and
 *											the data are held once we_l is
 *											deasserted
 *	WP0			<14:12>		RW		0		Write pulse width - The number of
 *											cycles that we_l is held asserted
 *	RES			<11:10>		MBZ,RAZ	0		Reserved
 *	WS0			<9:8>		RW		0		Write setup time - The number of
 *											cycles that the address and data
 *											are held stable to the device
 *											before we_l is asserted
 *	RES			<7>			MBZ,RAZ	0		Reserved
 *	RD0			<6:4>		RW		0		Read output disable time - The
 *											number of cycles that the device
 *											takes to turn off its output
 *											drivers once oe_l is deasserted
 *	RA0			<3:0>		RW		0		Read access time - The number of
 *											cycles that cs_l and oe_l are
 *											asserted to the device before the
 *											data is latched in from the TIGbus
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	ra0		: 4;	/* Read access time */
	u64	rd0		: 3;	/* Read output disable time */
	u64	res_7	: 1;	/* Reserved at bit 7 */
	u64	ws0		: 2;	/* Write setup time */
	u64	res_10	: 2;	/* Reserved at bits 11:10 */
	u64	wp0		: 3;	/* Write pulse width */
	u64	wh0		: 1;	/* Write hold time */
	u64	ra1		: 4;	/* Read access time */
	u64	rd1		: 3;	/* Read output disable time */
	u64	res_23	: 1;	/* Reserved at bit 7 */
	u64	ws1		: 2;	/* Write setup time */
	u64	res_26	: 2;	/* Reserved at bits 11:10 */
	u64	wp1		: 3;	/* Write pulse width */
	u64	wh1		: 1;	/* Write hold time */
	u64	ra2		: 4;	/* Read access time */
	u64	rd2		: 3;	/* Read output disable time */
	u64	res_39	: 1;	/* Reserved at bit 7 */
	u64	ws2		: 2;	/* Write setup time */
	u64	res_42	: 2;	/* Reserved at bits 11:10 */
	u64	wp2		: 3;	/* Write pulse width */
	u64	wh2		: 1;	/* Write hold time */
	u64	ra3		: 4;	/* Read access time */
	u64	rd3		: 3;	/* Read output disable time */
	u64	res_55	: 1;	/* Reserved at bit 7 */
	u64	ws3		: 2;	/* Write setup time */
	u64	res_58	: 2;	/* Reserved at bits 11:10 */
	u64	wp3		: 3;	/* Write pulse width */
	u64	wh3		: 1;	/* Write hold time */
} AXP_21274_TDR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_TDR_RMASK	0xF37FF37FF37FF37F
#define AXP_21274_TDR_WMASK	0xF37FF37FF37FF37F

/*
 * HRM 10.2.2.16 Power Management Control (PWR - RW)
 *
 * This register controls chipset management features. To date, only SDRAM
 * self-refresh mode is implemented.
 *
 * Warning:	Software must ensure that there are no DRAM accesses active in the
 *			21272 when self-refresh mode is active.
 *
 * Table 10-25 describes the power management control register (PWR).
 *
 *	Table 10-25 Power Management Control Register (PWR - RW)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:1>		MBZ,RAZ	0		Reserved.
 *	SR			<0>			RW		0		-----------------------------------
 *											Value	Meaning
 *											-----------------------------------
 *											0		Normal operation.
 *											1		Self-refresh mode; standard
 *													refreshing is suppressed,
 *													regardless of MTR<RI>.
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	sr		: 1;	/* Self Refresh */
	u64	res_1	: 63;	/* Reserved at bits 63:1 */
} AXP_21274_PWR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PWR_RMASK	0x0000000000000001
#define AXP_21274_PWR_WMASK	0x0000000000000001

/*
 * Definitions for the values in various fields in the PWR.
 */
#define AXP_SR_NORMAL			0
#define AXP_SR_SELF_REFRESH		1

/*
 * HRM 10.2.3 Cchip Monitor Control (CMONCTLA, CMONCTLB - RW) - Typhoon only
 *
 * All fields in the CMONCTLA and CMONCTLB registers are RW. They are cleared
 * by reset. Some monitor signals are hardwired to select a specific CPU. The
 * mask and match/entry fields provide wide flexibility in the selection of
 * events to count.
 *
 * Table 10-26 describes the Cchip monitor control register CMONCTLA and Table
 * 10-27 describes register CMONCTLB.
 *
 *	Table 10-26 Cchip Monitor Control Register (CMONCTLA)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:62>		MBZ,RAZ	0		Reserved.
 *	MSK23		<61:52>		RW		0		Mask field - For ECNT2 and ECNT3,
 *											the match/entry fields can be used
 *											to qualify the value in the <SLCTn>
 *											field.
 *	RES			<51:50>		MBZ,RAZ	0		Reserved.
 *	MSK01		<49:40>		RW		0		Mask field - For ECNT0 and ECNT1.
 *	STKDIS3		<39>		RW		0		ECNT3 stick disable.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		ECNT3 sticks at all ones
 *											1		ECNT3 wraps
 *											-----------------------------------
 *	STKDIS2		<38>		RW		0
 *	STKDIS1		<37>		RW		0
 *	STKDIS0		<36>		RW		0
 *	RES			<35:34>		MBZ,RAZ	0		Reserved.
 *	SLCTMBL		<33:32>		RW		0		Select memory bus monitor low bits.
 *											Note: Memory bus monitor bits
 *											<20:16> are fixed.
 *											-----------------------------------
 *											Value	Group
 *											-----------------------------------
 *											0		mem bus monitor <15:0> =
 *													mgroup0
 *											1		mem bus monitor <15:0> =
 *													mgroup1
 *											2		mem bus monitor <15:0> =
 *													mgroup2
 *											3		mem bus monitor <15:0> =
 *													mgroup3
 *											-----------------------------------
 *	SLCT3		<31:24>		RW		0		Select B MONITOR<3>; Select Event
 *											3.
 *	SLCT2		<23:16>		RW		0		Select B MONITOR<2>; Select Event
 *											2.
 *	SLCT1		<15:8>		RW		0		Select B MONITOR<1>; Select Event
 *											1.
 *	SLCT0		<7:0>		RW		0		Select B MONITOR<0>; Select Event
 *											0.
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	slct0	: 8;	/* Select B Minitor<0> */
	u64	slct1	: 8;	/* Select B Minitor<1> */
	u64	slct2	: 8;	/* Select B Minitor<2> */
	u64	slct3	: 8;	/* Select B Minitor<3> */
	u64	slctmbl	: 2;	/* Select memory bus monitor low bits */
	u64	res_34	: 2;	/* Reserved at bits 35:34 */
	u64	stkdis0	: 1;	/* ECNT0 stick disable */
	u64	stkdis1	: 1;	/* ECNT1 stick disable */
	u64	stkdis2	: 1;	/* ECNT2 stick disable */
	u64	stkdis3	: 1;	/* ECNT3 stick disable */
	u64	msk01	: 10;	/* Mask field for ECNT0 and ECNT1 */
	u64	res_50	: 2;	/* Reserverd at bits 51:50 */
	u64	msk23	: 10;	/* Mask field for ECNT2 and ECNT3 */
	u64	res_62	: 2;	/* Reserved at bits 63:52 */
} AXP_21274_CMONCTLA;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_CMONA_RMASK	0x3FF3FFF3FFFFFFFF
#define AXP_21274_CMONA_WMASK	0x3FF3FFF3FFFFFFFF

/*
 * Definitions for the values in various fields in the CMONCTLA.
 */
#define AXP_STKDIS_ALL_ONES			0
#define AXP_STKDIS_WRAPS			1
#define AXP_SLCTMBL_MGROUP0			0
#define AXP_SLCTMBL_MGROUP1			1
#define AXP_SLCTMBL_MGROUP2			2
#define AXP_SLCTMBL_MGROUP3			3

/*
 * HRM Table 10-27 Cchip Monitor Control Register (CMONCTLB)
 *
 *	Table 10-27 Cchip Monitor Control Register (CMONCTLB)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:62>		MBZ,RAZ	0		Reserved
 *	MTE3		<61:52>		RW		0		Match/entry field - for ECNT3
 *											The match/entry and mask fields can
 *											be used to qualify the value in the
 *											<SLCTn> field.
 *	RES			<51:50>		MBZ,RAZ	0		Reserved
 *	MTE2		<49:40>		RW		0		Match/entry field - for ECNT2
 *	RES			<39:38>		MBZ,RAZ	0		Reserved
 *	MTE1		<37:28>		RW		0		Match/entry field - for ECNT1
 *	RES			<27:26>		MBZ,RAZ	0		Reserved
 *	MTE0		<25:16>		RW		0		Match/entry field - for ECNT0
 *	RES			<15:1>		MBZ,RAZ	0		Reserved
 *	DIS			<0>			RW		0		Disable monitor output signals:
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		B_MONITOR outputs in use
 *													for monitor
 *											1		B_MONITOR outputs static at
 *													zero
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	dis		: 1;	/* Disable monitor output signals */
	u64	res_1	: 15;	/* Reserved at bits 15:1 */
	u64	mte0	: 10;	/* Match/entry field - for ECNT0 */
	u64	res_26	: 2;	/* Reserved at bits 27:26 */
	u64	mte1	: 10;	/* Match/entry field - for ECNT1 */
	u64	res_38	: 2;	/* Reserved at bits 39:38 */
	u64	mte2	: 10;	/* Match/entry field - for ECNT2 */
	u64	res_50	: 2;	/* Reserved at bits 51:50 */
	u64	mte3	: 10;	/* Match/entry field - for ECNT3 */
	u64	res_62	: 2;	/* Reserved at bits 63:62 */
} AXP_21274_CMONCTLB;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_CMONB_RMASK	0x3FF3FF3FF3FF0001
#define AXP_21274_CMONB_WMASK	0x3FF3FF3FF3FF0001

/*
 * Definitions for the values in various fields in the CMONCTLB.
 */
#define AXP_DIS_IN_USE				0
#define AXP_DIS_STATIC				1

/*
 * HRM 10.2.3.1 Cchip Monitor Counters (CMONCNT01, CMONCNT23 - R0)
 *
 * The 21272 has four 23-bit event counters. The event counted by counter n
 * (ECNTn) is selected by the CMONCTL<SLCTn> field. One of the possible events
 * selected is the carry-out of the previous counter, which allows both
 * counters to be used as two 64-bit counters. In this case, the
 * CMONCTL<STKDISn> bit must be set to ensure that the low-order 32-bit counter
 * does not stick at all ones.
 *
 * Both counters hold their values for four cycles
 * each time that a read to CMONCNTx is performed, so that a slight inaccuracy
 * can result if the events being counted continue to occur at the time of
 * reading.
 *
 * The <SLCTn> field may specify the use of the CMONCTL fields <MTEx> and
 * <MSKy> to further qualify the selection. In this case, a fixed
 * correspondence occurs between the ECNT field to be updated and the
 * combination MTE/MSK qualifier field used. Table 10-28 shows this
 * correspondence.
 *
 *	Table 10-28 Correspondence Between ECNT and MTE/MSK
 *	---------------------------------------------------------------------------
 *	Field to Increment		MTE Field Used		MSK Field Used
 *	---------------------------------------------------------------------------
 *	ECNT3					MTE3				MSK23
 *	ECNT2					MTE2				MSK23
 *	ECNT1					MTE1				MSK01
 *	ECNT0					MTE0				MSK01
 *	---------------------------------------------------------------------------
 *
 * CMONCNT01 Registers - Typhoon Only
 *
 * All fields in the CMONCNT01 registers are Read/Write; however, the write
 * feature is only for diagnostic purposes. Writing a value of all ones to any
 * field of CMONCNT is not supported due to implementation considerations (the
 * carry-out is pre-computed).
 *
 * All fields of CMONCNT are cleared by reset and when CMONCTLA or CMONCTLB is
 * written. The expected usage is to write CMONCTL, wait for a while, read
 * CMONCNT, and repeat. Table 10-29 shows the CMONCNT01 registers.
 *
 *	Table 10-29 CMONCNT01 Registers
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Values	Description
 *	---------------------------------------------------------------------------
 *	ECNT1		<63:32>		RW		0		-		Increments when Event 1 is
 *													true
 *	ECNT0		<31:0>		RW		0		-		Increments when Event 0 is
 *													true
 *	---------------------------------------------------------------------------
 */
typedef struct
{
		u32	ecnt0;			/* Increments when Event 0 is true */
		u32	ecnt1;			/* Increments when Event 1 is true */
} AXP_21274_CMONCNT01;

/*
 * CMONCNT23 Registers - Typhoon Only
 *
 * The operation of CMONCNT23 is the same as that for CMONCNT01. Table 10-30
 * shows the CMONCNT23 registers.
 *
 *	Table 10-30 CMONCNT23 Registers
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Values	Description
 *	---------------------------------------------------------------------------
 *	ECNT3		<63:32>		RW		0		-		Increments when Event 3 is
 *													true
 *	ECNT2		<31:0>		RW		0		-		Increments when Event 2 is
 *													true
 *	---------------------------------------------------------------------------
 */
typedef struct
{
		u32	ecnt2;			/* Increments when Event 0 is true */
		u32	ecnt3;			/* Increments when Event 1 is true */
} AXP_21274_CMONCNT23;

/****************************************************************************
 *							HRM 10.2.3 Dchip CSRs							*
 ****************************************************************************/

/*
 * HRM 10.2.4.1 Dchip System Configuration Register (DSC - RO)
 *
 *	Table 10-31 Dchip System Configuration Register (DSC)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:8>		Special	0		-
 *							(1)
 *	RES			<7>			RAZ 	0 		Reserved
 *	P1P			<6>			RO 		-(2)	Pchip 1 present
 *	C3CFP		<5>			RO 		-(2)	CPU3 clock forward preset (see
 *											Chapter 11)
 *	C2CFP		<4>			RO 		-(2)	CPU2 clock forward preset (see
 *											Chapter 11)
 *	C1CFP		<3>			RO 		-(2)	CPU1 clock forward preset (see
 *											Chapter 11)
 *	C0CFP		<2>			RO 		-(2)	CPU0 clock forward preset (see
 *											Chapter 11)
 *	BC			<1:0>		RO 		-(2)	Base configuration
 *											-----------------------------------
 *											Value	Configuration
 *											-----------------------------------
 *											0		2 Dchips, 1 memory bus
 *											1		4 Dchips, 1 memory bus
 *											2		4 Dchips, 2 memory buses
 *											3		8 Dchips, 2 memory buses
 *	---------------------------------------------------------------------------
 *	(1)	This is an 8-bit register that mirrors some information in CSC. It is
 *		special, however, in that it is byte-sliced across eight Dchips.
 *		Therefore, it is read as a quadword with the same value repeated in all
 *		eight bytes.
 *	(2)	This register powers up to the value present on bits <6:0> of the CPM
 *		command from the Cchip.
 */
typedef struct
{
	union
	{
		struct
		{
			u8	bc		: 2;	/* Base configuration */
			u8	c0cfp	: 1;	/* CPU0 clock forward preset */
			u8	c1cfp	: 1;	/* CPU1 clock forward preset */
			u8	c2cfp	: 1;	/* CPU2 clock forward preset */
			u8	c3cfp	: 1;	/* CPU3 clock forward preset */
			u8	p1p		: 1;	/* Pchip 1 present */
			u8	res_7	: 1;	/* Reserved at bit 7 */
		};
		u8	dchip0;
	};
	u8	dchip1;
	u8	dchip2;
	u8	dchip3;
	u8	dchip4;
	u8	dchip5;
	u8	dchip6;
	u8	dchip7;
} AXP_21274_DSC;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only.
 */
#define AXP_21274_DSC_RMASK	0x7F7F7F7F7F7F7F7F

/*
 * HRM 10.2.4.2 Dchip System Configuration Register 2 (DSC2 - R0)
 *
 * These registers are for future use, for a Dchip that implements wide PADbus
 * support.  Table 10-32 describes the Dchip system configuration register 2.
 *
 *	Table 10-32 Dchip System Configuration Register 2 (DSC2)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:5>		RO		0		Reserved
 *	RES			<4:2>		RO		-(1)	Reserved
 *	P1W			<1>			RO		-(1)	Reserved
 *											0 = Wide PADbus1 (Typhoon only)
 *	P0W			<0>			RO		-(1)	Reserved
 *											0 = Wide PADbus0 (Typhoon only)
 *	---------------------------------------------------------------------------
 *	(1)	This register powers up to the value present on bits <4:0> of the
 *		PADCMD bus from the Cchip.
 */
typedef struct
{
		u64	p0w		: 1;	/* Reserved */
		u64	p1w		: 1;	/* Reserved */
		u64	res_2	: 3;	/* Reserved at bits 4:2 */
		u64	res_5	: 59;	/* Reserved at bits 63:5 */
} AXP_21274_DSC2;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only.
 */
#define AXP_21274_DSC2_RMASK	0x0000000000000003

/*
 * HRM 10.2.4.3 System Timing Register (STR - RW)
 *
 * When the system timing register is written, all Dchips, as well as the
 * corresponding fields in the CSC register, are updated at the same time. The
 * corresponding fields in the CSC register are read-only, so the only way to
 * update them is to write this register.
 *
 * Note:	Follow the rules listed in Chapter 12 when writing to this CSR.
 * 			After writing to this register, a delay is required to ensure that
 * 			subsequent accesses to the 21272 will succeed.
 *
 * IDDR, IDDW, and IRD (set in CSC) must be set as follows before accessing
 * memory:
 *
 * - RCD is the RAS-to-CAS delay in the DRAMs (set in CSC).
 * - CAT is the CAS access time in the DRAMs (set in CSC).
 * - SED is the SysDC extract delay (set in the CSC).
 * - b is the burst length (2 for 32-byte memories and 4 for 16-byte memories).
 * - p is the number of pipeline stages on the control signals between the
 * 	 Cchip and the SDRAMs (0, 1, or 2).
 * 	 IDDR = RCD + CAT + p + b - 1
 * 	 IDDW = MAX (RCD + p - 1, SED + 1, IDDR - 2b + 1)
 * 	 IRD = IDDW - RCD - p + 1
 *
 * If software wishes to set IDDW to a value other than the power-up default,
 * and knows that memory will not yet be accessed, then the restriction is
 * relaxed to:
 *
 * 		IDDW > SED
 *
 * Table 10-33 describes the system timing register (STR).
 *
 *	Table 10-33 System Timing Register (STR)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:8>		Special	0 		-(1)
 *							(1)
 *	RES			<7:6>		MBZ,RAZ	0		Reserved
 *	IDDW		<5:4>		RW		2		Issue to data delay for all
 *											transactions except memory reads
 *											(see Table 7-5)
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		3 cycles
 *											1		4 cycles
 *											2		5 cycles
 *											3		6 cycles
 *											-----------------------------------
 *	IDDR		<3:1>		RW		4		Issue to data delay for memory
 *											reads (see Table 7-5)
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		5 cycles
 *											1		6 cycles
 *											2		7 cycles
 *											3		8 cycles
 *											4		9 cycles
 *											5		10 cycles
 *											6		11 cycles
 *											7		Reserved
 *											-----------------------------------
 *	AW			<0>			RW		0		Array width
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		16 bytes
 *											1		32 bytes
 *	---------------------------------------------------------------------------
 *	(1)	This is an 8-bit register corresponding to bits CSC<13:8>. It is
 *		special, however, in that it must be written to up to eight Dchips
 *		simultaneously. Therefore, it is written as a quadword with the same
 *		value repeated in all eight bytes. That way, all Dchips are configured
 *		properly regardless of system configuration.
 */
typedef struct
{
	union
	{
		struct
		{
			u8	aw		: 1;	/* Array width */
			u8	iddr	: 3;	/* Issue to data delay for memory reads */
			u8	iddw	: 2;	/* issue to data delay for non reads */
			u8	res_7	: 2;	/* Reserved at bit 7 */
		};
		u8	dchip0;
	};
	u8	dchip1;
	u8	dchip2;
	u8	dchip3;
	u8	dchip4;
	u8	dchip5;
	u8	dchip6;
	u8	dchip7;
} AXP_21274_STR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_STR_RMASK	0x3F3F3F3F3F3F3F3F
#define AXP_21274_STR_WMASK	0x000000000000003F

/*
 * HRM 10.2.4.4 Dchip Revision Register (DREV - RO)
 *
 *	Table 10-34 Dchip Revision Register (DREV)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:60>		RAZ		0		Reserved
 *	REV7		<59:56>		RO		1		Dchip 7 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<55:52>		RAZ		0		Reserved
 *	REV6		<51:48>		RO		1		Dchip 6 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<47:44>		RAZ		0		Reserved
 *	REV5		<43:40>		RO		1		Dchip 5 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<39:36>		RAZ		0		Reserved
 *	REV4		<35:32>		RO		1		Dchip 4 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<31:28>		RAZ		0		Reserved
 *	REV3		<27:24>		RO		1		Dchip 3 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<23:20>		RAZ		0		Reserved
 *	REV2		<19:16>		RO		1		Dchip 2 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<15:12>		RAZ		0		Reserved
 *	REV1		<11:8>		RO		1		Dchip 1 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	RES			<7:4>		RAZ		0		Reserved
 *	REV0		<3:0>		RO		1		Dchip 0 revision. This field
 *											indicates the latest revision of
 *											the Dchip.
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	rev0	: 4;	/* Dchip 0 revision */
	u64 res_4	: 4;	/* Reserved at bits 7:4 */
	u64	rev1	: 4;	/* Dchip 1 revision */
	u64 res_12	: 4;	/* Reserved at bits 7:4 */
	u64	rev2	: 4;	/* Dchip 2 revision */
	u64 res_20	: 4;	/* Reserved at bits 7:4 */
	u64	rev3	: 4;	/* Dchip 3 revision */
	u64 res_28	: 4;	/* Reserved at bits 7:4 */
	u64	rev4	: 4;	/* Dchip 4 revision */
	u64 res_36	: 4;	/* Reserved at bits 7:4 */
	u64	rev5	: 4;	/* Dchip 5 revision */
	u64 res_44	: 4;	/* Reserved at bits 7:4 */
	u64	rev6	: 4;	/* Dchip 6 revision */
	u64 res_52	: 4;	/* Reserved at bits 7:4 */
	u64	rev7	: 4;	/* Dchip 7 revision */
	u64 res_60	: 4;	/* Reserved at bits 7:4 */
} AXP_21274_DREV;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only.
 */
#define AXP_21274_DREV_RMASK	0x0303030303030303

/****************************************************************************
 *							HRM 10.2.5 Pchip CSRs							*
 ****************************************************************************/

/*
 * HRM 10.2.5.1 Window Space Base Address Register (WSBAn - RW)
 *
 * Because the information in the WSBAn registers and WSMn registers (Section
 * 10.2.5.2) is used to compare against the PCI address, a clock-domain
 * crossing (from i_sysclk to i_pclko<7:0>) is made when these registers are
 * written. Therefore, for a period of several clock cycles, a window is
 * disabled when its contents are disabled. If PCI bus activity, which accesses
 * the window in question, is not stopped before updating that window, the
 * Pchip might fail to respond with b_devsel_l when it should. This would
 * result in a master abort condition on the PCI bus. Therefore, before a
 * window (base or mask) is updated, all PCI activity accessing that window
 * must be stopped, even if only some activity is being added or deleted.
 *
 * The contents of the window may be read back to confirm that the update has
 * taken place. Then PCI activity through that window can be resumed.
 *
 * Table 10-35 describes the window space base address registers WSBA0, 1, and
 * 2.
 * Table 10-36 describes WSBA3.
 *
 *	Table 10-35 Window Space Base Address Register (WSBA0, 1, 2)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:32>		MBZ,RAZ	0		Reserved
 *	ADDR		<31:20>		RW		0		Base address
 *	RES			<19:2>		MBZ,RAZ	0		Reserved
 *	SG			<1>			RW		0		Scatter-gather
 *	ENA			<0>			RW		0		Enable
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	ena		: 1;	/* Enable */
	u64	sg		: 1;	/* Scatter-gather */
	u64	res_2	: 18;	/* Reserved at bits 19:2 */
	u64	addr	: 12;	/* Base address */
	u64	res_32	: 32;	/* Reserved at bits 63:32 */
} AXP_21274_WSBAn;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_WSBAn_RMASK	0x00000000FFF00003
#define AXP_21274_WSBAn_WMASK	0x00000000FFF00003

/*
 * Definitions for the values in various fields in the WSBAn and WSBA3.
 */
#define AXP_ENA_DISABLE				0
#define AXP_ENA_ENABLE				1
#define AXP_SG_DISABLE				0
#define AXP_SG_ENABLE				1

/*	Table 10-36 Window Space Base Address Register (WSBA3)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:40>		MBZ,RAZ	0		Reserved
 *	DAC			<39>		RW		0		DAC enable
 *	RES			<38:32>		MBZ,RAZ	0		Reserved
 *	ADDR		<31:20>		RW		0		Base address if DAC enable = 0
 *											Not used if DAC enable = 1
 *	RES			<19:2>		MBZ,RAZ	0		Reserved
 *	SG			<1>			RO		1		Scatter-gather always enabled
 *	ENA			<0>			RW		0		Enable
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	ena		: 1;	/* Enable */
	u64	sg		: 1;	/* Scatter-gather always enabled */
	u64	res_2	: 18;	/* Reserved at bits 19:2 */
	u64	addr	: 12;	/* Base address id DAC enable = 0, not used otherwise */
	u64	res_32	: 7;	/* Reserved at bits 38:32 */
	u64	dac		: 1;	/* DAC enable */
	u64	res_40	: 24;	/* Reserved at bits 63:40 */
} AXP_21274_WSBA3;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_WSBA3_RMASK	0x00000080FFF00003
#define AXP_21274_WSBA3_WMASK	0x00000080FFF00001

/*
 * Definitions for the values in various fields in the WSBA3.
 */
#define AXP_DAC_DISABLE				0
#define AXP_DAC_ENABLE				1

/*
 * HRM 10.2.5.2 Window Space Mask Register (WSM0, WSM1, WSM2, WSM3 - RW)
 *
 * Table 10-37 describes the window space mask registers. Refer to the WSBAn
 * register description (Section 10.2.5.1) for a brief description of the
 * window space mask register.
 *
 *	Table 10-37 Window Space Mask Register (WSMn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:32>		MBZ,RAZ	0		Reserved
 *	AM			<31:20>		RW		0		Address mask
 *	RES			<19:0>		MBZ,RAZ	0		Reserved
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	res_0	: 20;	/* Reserved at bits 19:0 */
	u64	am		: 12;	/* Address mask */
	u64 res_32	: 32;	/* Reserved at bits 63-32 */
} AXP_21274_WSMn;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_WSMn_RMASK	0x00000000FFF00000
#define AXP_21274_WSMn_WMASK	0x00000000FFF00000

/*
 * HRM 10.2.5.3 Translated Base Address Register (TBAn - RW)
 *
 * Table 10-38 describes the translated base address registers TBA0, 1, and 2.
 * Table 10-39 describes TBA3.
 *
 *	Table 10-38 Translated Base Address Registers (TBA0, 1, and 2)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:35>		MBZ,RAZ	0		Reserved
 *	ADDR		<34:10>		RW		0		Translated address base
 *	RES			<9:0>		MBZ,RAZ	0		Reserved
 *	---------------------------------------------------------------------------
 *
 * 	Table 10-39 Translated Base Address Registers (TBA3)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:35>		MBZ,RAZ	0		Reserved
 *	ADDR		<34:10>		RW		0		If DAC enable = 1, bits <34:22> are
 *											the Page Table Origin address
 *											<34:22> and bits <21:10> are
 *											ignored.
 *											If DAC enable = 0, this is the
 *											translated address base.
 *	RES			<9:0>		MBZ,RAZ	0		Reserved
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	res_0	: 10;	/* Reserved at bits 9:0 */
	u64	addr	: 25;	/* See above TBAn and TBA3 descriptions */
	u64	res_35	: 29;	/* Reserved at bits 63:35 */
} AXP_21274_TBAn;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_TBAn_RMASK	0x00000007FFFFFC00
#define AXP_21274_TBAn_WMASK	0x00000007FFFFFC00

/*
 * HRM 10.2.5.4 Pchip Control Register (PCTL - RW)
 *
 *	Table 10-40 Pchip Control Register (PCTL)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:48>		MBZ,RAZ	0		Reserved.
 *	PID			<47:46>		RO 		-(1)	Pchip ID.
 *	RPP			<45>		RO 		-(2)	Remote Pchip present.
 *	PTEVRFY		<44>		RW 		-		PTE verify for DMA read.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		If TLB miss, then make DMA
 *													read request as soon as
 *													possible and discard data
 *													if PTE was not valid -
 *													could cause Cchip
 *													nonexistent memory error.
 *											1		If TLB miss, then delay
 *													read request until PTE is
 *													verified as valid - no
 *													request if not valid.
 *	FDWDIS		<43>		RW		-		Fast DMA read cache block wrap
 *											request disable.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		Normal operation
 *											1		Reserved for testing
 *													purposes only
 *	FDSDIS		<42>		RW		-		Fast DMA start and SGTE request
 *											disable.
 *											-----------------------------------
 *											Value	Description
 *											-----------------------------------
 *											0		Normal operation
 *											1		Reserved for testing
 *													purposes only
 *	PCLKX		<41:40>		RO		-(3)	PCI clock frequency multiplier
 *											-----------------------------------
 *											Value	Multiplier
 *											-----------------------------------
 *											0		x6
 *											1		x4
 *											2		x5
 *											3		Reserved
 *	PTPMAX		<39:36>		RW		2		Maximum PTP requests to Cchip from
 *											both Pchips until returned on
 *											CAPbus, modulo 16 (minimum = 2)
 *											(use 4 for pass 1 Cchip and Dchip).
 *	CRQMAX		<35:32>		RW		1		Maximum requests to Cchip from both
 *											Pchips until Ack, modulo 16 (use 4
 *											for Cchip).
 *											(Use 3 or less for Typhoon because
 *											there is one less skid buffer in
 *											the C4 chip.)
 *	REV			<31:24>		RO		0		In conjunction with the state of
 *											PMONCTL<0>, this field indicates
 *											the revision of the Pchip (see
 *											Section 8.10).
 *	CDQMAX		<23:20>		RW		1		Maximum data transfers to Dchips
 *											from both Pchips until Ack, modulo
 *											16 (use 4 for Dchip). Must be same
 *											as Cchip CSR CSC<FPQPMAX>.
 *	PADM		<19>		RW		-(4)	PADbus mode.
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		8-nibble, 8-check bit mode
 *											1		4-byte, 4-check bit mode
 *	ECCEN		<18>		RW		0		ECC enable for DMA and SGTE
 *											accesses.
 *	RES			<17:16>		MBZ,RAZ	0		Reserved.
 *	PPRI		<15>		-		0		Arbiter priority group for the
 *											Pchip.
 *	PRIGRP		<14:8>		RW		0		Arbiter priority group; one bit per
 *											PCI slot with bits <14:8>
 *											corresponding to input
 *											b_req_l<6:0>.
 *											-----------------------------------
 *											Value	Group
 *											-----------------------------------
 *											0		Low-priority group
 *											1		High-priority group
 *	ARBENA		<7>			RW		0		Internal arbiter enable.
 *	MWIN		<6>			RW		0		Monster window enable.
 *	HOLE		<5>			RW		0		512KB-to-1MB window hole enable.
 *	TGTLAT		<4>			RW		0		Target latency timers enable.
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		Retry/disconnect after 128
 *													PCI clocks without data.
 *											1		Retry initial request after
 *													32 PCI clocks without data;
 *													disconnect subsequent
 *													transfers after 8 PCI
 *													clocks without data.
 *	CHAINDIS	<3>			RW		0		Disable chaining.
 *	THDIS		<2>			RW		0		Disable antithrash mechanism for
 *											TLB.
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		Normal operation
 *											1		Testing purposes only
 *	FBTB		<1>			RW		0		Fast back-to-back enable.
 *	FDSC		<0>			RW		0		Fast discard enable.
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		Discard data if no retry
 *													after 215 PCI clocks.
 *											1	Discard data if no retry after
 *												210 PCI clocks.
 *	---------------------------------------------------------------------------
 *	(1)	This field is initialized from the PID pins.
 *	(2)	This field is initialized from the assertion of CREQRMT_L pin at system
 *		reset.
 *	(3)	This field is initialized from the PCI i_pclkdiv<1:0> pins.
 *	(4)	This field is initialized from a decode of the b_cap<1:0> pins.
 */
typedef struct
{
	u64	fdsc	: 1;	/* Fast discard enable */
	u64	fbtb	: 1;	/* Fast back-to-back enable */
	u64	thdis	: 1;	/* Disable antithrash mechanism for TLB */
	u64	chaindis : 1;	/* Disable chaining */
	u64 tgtlat	: 1;	/* Target latency timers enable */
	u64	hole	: 1;	/* 512KB-to-1MB window hole enable */
	u64	mwin	: 1;	/* Monster window enable */
	u64	arbena	: 1;	/* Internal arbiter enable */
	u64	prigrp	: 7;	/* Arbiter priority group */
	u64	ppri	: 1;	/* Arbiter priority group for the Pchip */
	u64	res_16	: 2;	/* Reserved at bits 17:16 */
	u64	eccen	: 1;	/* ECC ebavle for DMA and SGTE accesses */
	u64	padm	: 1;	/* PADbus mode */
	u64	cdqmax	: 4;	/* Maximum data transfers to Dchips from both Pchips */
	u64	rev		: 8;	/* Revision of the Pchip (see Section 8.10) */
	u64	crqmax	: 4;	/* Maximum requests to Cchip from both Pchips */
	u64	ptpmax	: 4;	/* Maximum PTP requests to Cchip from both Pchips */
	u64	pclkx	: 2;	/* PCI clock frequency multiplier */
	u64	fdsdis	: 1;	/* Fast DMA start and SGTE request disable */
	u64	fdwdis	: 1;	/* Fast DMA read cache block wrap request disable */
	u64	ptevrfy	: 1;	/* PTE verify for DMA read */
	u64	rpp		: 1;	/* Remote Pchip present */
	u64	pid		: 2;	/* Pchip ID */
	u64	res_48	: 16;	/* Reserved at bits 63:48 */
} AXP_21274_PCTL;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PCTL_RMASK	0x0000FFFFFFFCFFFF
#define AXP_21274_PCTL_WMASK	0x00001CFF00FCFFFF

/*
 * Definitions for the values in various fields in the PCTL.
 */
#define AXP_RPP_NOT_PRESENT			0
#define AXP_RPP_PRESENT				1
#define AXP_PTEVRFY_DISABLE			0
#define AXP_PTEVRFY_ENABLE			1
#define AXP_FDWDIS_NORMAL			0
#define AXP_FDWDIS_TEST				1
#define AXP_FDSDIS_NORMAL			0
#define AXP_FDSDIS_TEST				1
#define AXP_PCLKX_6_TIMES			0
#define AXP_PCLKX_4_TIMES			1
#define AXP_PCLKX_5_TIMES			2
#define AXP_PADM_8_8				0
#define AXP_PADM_4_4				1
#define AXP_ECCEN_DISABLE			0
#define AXP_ECCEN_ENABLE			1
#define AXP_PPRI_LOW				0
#define AXP_PPRI_HIGH				1
#define AXP_PRIGRP_PICx_LOW			0x00	/* x000 0000 */
#define AXP_PRIGRP_PCI0_HIGH		0x01	/* x000 0001 */
#define AXP_PRIGRP_PCI1_HIGH		0x02	/* x000 0010 */
#define AXP_PRIGRP_PCI2_HIGH		0x04	/* x000 0100 */
#define AXP_PRIGRP_PCI3_HIGH		0x08	/* x000 1000 */
#define AXP_PRIGRP_PCI4_HIGH		0x10	/* x001 0000 */
#define AXP_PRIGRP_PCI5_HIGH		0x20	/* x010 0000 */
#define AXP_PRIGRP_PCI6_HIGH		0x40	/* x100 0000 */
#define AXP_ARBENA_DISABLE			0
#define AXP_ARBENA_ENABLE			1
#define AXP_MWIN_DISABLE			0
#define AXP_MWIN_ENABLE				1
#define AXP_HOLE_DISABLE			0
#define AXP_HOLE_ENABLE				1
#define AXP_TGTLAT_DISABLE			0
#define AXP_TGTLAT_ENABLE			1
#define AXP_CHAINDIS_DISABLE		0
#define AXP_CHAINDIS_ENABLE			1
#define AXP_THDIS_NORMAL			0
#define AXP_THDIS_TEST				1
#define AXP_FBTB_DISABLE			0
#define AXP_FBTB_ENABLE				1
#define AXP_FDSC_DISABLE			0
#define AXP_FDSC_ENABLE				1

/*
 * HRM 10.2.5.5 Pchip Master Latency Register (PLAT - RW)
 *
 * Table 10-41 describes the Pchip master latency register (PLAT).
 *
 *	Table 10-41 Pchip Master Latency Register (PLAT)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:16>		MBZ,RAZ	0		Reserved
 *	LAT			<15:8>		RW		0		Master latency timer
 *	RES			<7:0>		MBZ,RAZ	0		Reserved
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u8	res_0;			/* Reserved at bits 7:0 */
	u8	lat;			/* Master latency timer */
	u16	res_16;			/* Reserved at bits 31-16 */
	u32	res_32;			/* Reserved at bits 63-32 */
} AXP_21274_PLAT;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PLAT_RMASK	0x000000000000FF00
#define AXP_21274_PLAT_WMASK	0x000000000000FF00

/*
 * HRM 10.2.5.6 Pchip Error Register (PERROR - RW)
 *
 * If any of bits <11:0> are set, then this entire register is frozen and the
 * Pchip output signal b_error is asserted. Only bit <0> can be set after that.
 * All other values will be held until all of bits <11:0> are clear. When an
 * error is detected and one of bits <11:0> becomes set, the associated
 * information is captured in bits <63:16> of this register. After the
 * information is captured, the INV bit is cleared, but the information is not
 * valid and should not be used if INV is set.
 *
 * In rare circumstances involving more than one error, INV may remain set
 * because the Pchip cannot correctly capture the SYN, CMD, or ADDR field.
 *
 * Furthermore, if software reads PERROR in a polling loop, or reads PERROR
 * before the Pchip-s error signal is reflected in the Cchip-s DRIR CSR, the
 * INV bit may also be set.  To avoid the latter condition, read PERROR only
 * after receiving an IRQ0 interrupt, then read the Cchip DIR CSR to determine
 * that this Pchip has detected an error.
 *
 * Table 10-42 describes the Pchip error register (PERROR).
 *
 *	Table 10-42 Pchip Error Register (PERROR)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	SYN			<63:56>		RO		0		ECC syndrome of error if CRE or
 *											UECC.
 *	CMD			<55:52>		RO		0		PCI command of transaction when
 *											error detected if not CRE and not
 *											UECC.
 *											If CRE or UECC, then:
 *											-----------------------------------
 *											Value	Command
 *											-----------------------------------
 *											0000	DMA read
 *											0001	DMA RMW
 *											0011	SGTE read
 *											Others	Reserved
 *											-----------------------------------
 *	INV			<51>		RO Rev1	0		Info Not Valid - only meaningful
 *							RAZ Rev0		when one of bits <11:0> is set.
 *						   					Indicates validity of <SYN>, <CMD>,
 *											and <ADDR> fields.
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		Info fields are valid.
 *											1		Info fields are not valid.
 *											-----------------------------------
 *	ADDR		<50:16>		RO		0		If CRE or UECC, then ADDR<50:19> =
 *											system address <34:3> of erroneous
 *											quadword and ADDR<18:16> = 0.
 *											If not CRE and not UECC, then
 *											ADDR<50:48> = 0;
 *											ADDR<47:18> = starting PCI address
 *											<31:2> of transaction when error
 *											was detected;
 *											ADDR<17:16> = 00 --> not a DAC
 *											operation;
 *											ADDR<17:16> = 01 --> via DAC SG
 *											Window 3;
 *											ADDR<17> = 1 --> via Monster Window
 *	RES			<15:12>		MBZ,RAZ	0		Reserved.
 *	CRE			<11>		R,W1C	0		Correctable ECC error.
 *	UECC		<10>		R,W1C	0		Uncorrectable ECC error.
 *	RES			<9>			MBZ,RAZ	0		Reserved.
 *	NDS			<8>			R,W1C	0		No b_devsel_l as PCI master.
 *	RDPE		<7>			R,W1C	0		PCI read data parity error as PCI
 *											master.
 *	TA			<6>			R,W1C	0		Target abort as PCI master.
 *	APE			<5>			R,W1C	0		Address parity error detected as
 *											potential PCI target.
 *	SGE			<4>			R,W1C	0		Scatter-gather had invalid page
 *											table entry.
 *	DCRTO		<3>			R,W1C	0		Delayed completion retry timeout as
 *											PCI target.
 *	PERR		<2>			R,W1C	0		b_perr_l sampled asserted.
 *	SERR		<1>			R,W1C	0		b_serr_l sampled asserted.
 *	LOST		<0>			R,W1C	0		Lost an error because it was
 *											detected after this register was
 *											frozen, or while in the process of
 *											clearing this register.
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	lost	: 1;	/* Lost and error */
	u64	serr	: 1;	/* b_serr_1 sampled asserted */
	u64	perr	: 1;	/* b_perr_1 sampled asserted */
	u64	dcrto	: 1;	/* Delayed completion retry timeout as PCI target */
	u64	sge		: 1;	/* Scatter-gather had invalid page table entry */
	u64	ape		: 1;	/* Address parity error detected as potential PCI target */
	u64	ta		: 1;	/* Target abort as PCI master */
	u64	rdpe	: 1;	/* PCI read data parity error as PCI master */
	u64	nds		: 1;	/* No b-devsel_1 as PCI master */
	u64	res_9	: 1;	/* Reserved at bit 9 */
	u64	uecc	: 1;	/* Uncorrectable ECC error */
	u64	cre		: 1;	/* Correctable ECC error */
	u64	res_12	: 4;	/* Reserved at bits 15:12 */
	u64	addr	: 35;	/* CRE or UECC address */
	u64	inv		: 1;	/* Info Not Valid */
	u64 cmd		: 4;	/* PCI command of transaction when error detected */
	u64	syn		: 8;	/* ECC syndrome of error if CRE or UECC */
} AXP_21274_PERROR;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PERROR_RMASK	0xFFFFFFFFFFFF0DFF
#define AXP_21274_PERROR_WMASK	0x0000000000000DFF

/*
 * Definitions for the values in various fields in the PERROR.
 */
#define AXP_CMD_DMA_READ			0	/* 0000 */
#define AXP_CMD_DMA_RMW				1	/* 0001 */
#define AXP_CMD_SGTE_READ			3	/* 0011	*/
#define AXP_INFO_VALID				0
#define AXP_INFO_NOT_VALID			1
#define AXP_LOST_NOT_LOST			0
#define AXP_LOST_LOST				1

/*
 * HRM 10.2.5.7 Pchip Error Mask Register (PERRMASK - RW)
 *
 * If any of the MASK bits have the value 0, they prevent the setting of the
 * corresponding bit in the PERROR register, regardless of the detection of
 * errors or writing to PERRSET.  The default is for all errors to be disabled.
 *
 * Beside masking the reporting of errors in PERROR, certain bits of PERRMASK
 * have the following additional effects:
 * 	- If PERROR<RDPE> = 0, the Pchip ignores read data parity as the PCI master.
 * 	- If PERROR<PERR> = 0, the Pchip ignores write data parity as the PCI target.
 * 	- If PERROR<APE> = 0, the Pchip ignores address parity.
 *
 * Table 10-43 describes the Pchip error mask register (PERRMASK).
 *
 *	Table 10-43 Pchip Error Mask Register (PERRMASK)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:12>		MBZ,RAZ	0		Reserved
 *	MASK		<11:0>		RW		0		PERROR register bit enables (see
 *											the text in this section and in
 *											Section 10.2.5.6)
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	mask	: 12;	/* PERROR register bit enables */
	u64	res_12	: 52;	/* Reserved at bits 63:12 */
} AXP_21274_PERRMASK;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PERRMASK_RMASK	0x0000000000000FFF
#define AXP_21274_PERRMASK_WMASK	0x0000000000000FFF

/*
 * HRM 10.2.5.8 Pchip Error Set Register (PERRSET - WO)
 *
 * If any of the SET bits = 1, and the corresponding MASK bits in PERRMASK also
 * = 1, they cause the setting of the corresponding bits in the PERROR
 * register, the capture of the INFO into the corresponding bits in the PERROR
 * register, and the freezing of the PERROR register. Zero (0) values in the
 * PERRMASK register override one (1) values in the PERRSET register. If the
 * PERROR register is already frozen when PERRSET is written, only the LOST bit
 * will be additionally set in PERROR.
 *
 * Table 10-44 describes the Pchip error set register (PERRSET).
 *
 *	Table 10-44 Pchip Error Set Register (PERRSET)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	INFO		<63:16>		WO		0		PERROR register information (see
 *											the text in this section)
 *	RES			<15:12>		MBZ		0		Reserved
 *	SET			<11:0>		WO		0		PERROR register bit set (see the
 *											text in this section and in Section
 *											10.2.5.6)
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	set		: 12;	/* PERROR register bit set */
	u64	res_12	: 4;	/* Reserved at bits 15:12 */
	u64	info	: 48;	/* PERROR register information */
} AXP_21274_PERRSET;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only.
 */
#define AXP_21274_PERRSET_WMASK	0xFFFFFFFFFFFF0FFF

/*
 * HRM 10.2.5.9 Translation Buffer Invalidate Virtual Register (TLBIV - WO)
 *
 * A write to this register invalidates all scatter-gather TLB entries that
 * correspond to PCI addresses whose bits <31:16> and bit 39 match the value
 * written in bits <19:4> and 27 respectively. This invalidates up to eight
 * PTEs at a time, which are the number that can be defined in one 21264 cache
 * block (64 bytes). Because a single TLB PCI tag covers four entries, at most
 * two tags are actually invalidated. PTE bits <22:4> correspond to system
 * address bits <34:16> - where PCI<34:32> must be zeros for scatter-gather
 * window hits - in generating the resulting system address, providing 8-page
 * (8KB) granularity.
 *
 * Table 10-45 describes the translation buffer invalidate virtual register (TLBIV).
 *
 *	Table 10-45 Translation Buffer Invalidate Virtual Register (TLBIV)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:28>		WO,MBZ	0		Reserved
 *	DAC			<27>		WO		0		Only invalidate if match PCI
 *											address <39>
 *	RES			<26:20>		WO,MBZ	0		Reserved
 *	ADDR		<19:4>		WO		0		Only invalidate if match against
 *											PCI address <31:16>
 *	RES			<3:0>		WO,MBZ	0		Reserved
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	res_0	: 4;	/* Reserved at bits 3:0 */
	u64	addr	: 16;	/* Only invalidate if match against PCI address 31:16 */
	u64	res_20	: 7;	/* Reserved at bits 26:20 */
	u64	dac		: 1;	/* Only invalidate if match PCI address bit 39 */
	u64	res_28	: 36;	/* Reserved at bits 63:28 */
} AXP_21274_TLBIV;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only.
 */
#define AXP_21274_TLBIV_WMASK	0x00000000080FFFF0

/*
 * HRM 10.2.5.11 Pchip Monitor Control Register (PMONCTL - RW)
 *
 * This register has two fields - one each for selecting among a set of
 * internal signals. The set of selectable signals is identical for each field.
 * SLCT0 selects the signal that is brought to the chip output b_monitor<0>.
 * SLCT1 selects the signal that is brought to the chip output b_monitor<1>.
 * The chip monitor outputs are two i_sysclk cycles later than the defined
 * signal. All of the defined signals are synchronized to the system clock (not
 * the PCI clock). Also, some of the signals derived from PCI clocked signals
 * are gated with UREN_D1_R (see Section 8.1.2.3) so that they can be used to
 * count events that occur in the PCI clock domain, regardless of the PCI clock
 * frequency. Others are not gated this way, so that durations can be measured
 * in terms of system clocks.
 *
 * In addition, b_monitor<0> is used as the input to the CNT0 field in PMONCNT,
 * and b_monitor<1> is used as the input to the least significant bit in the
 * CNT1 field in PMONCNT.
 *
 * Writing any value to PMONCTL clears both fields of PMONCNT.
 *
 * In normal operation, the two counters in PMONCNT stick at the value of all
 * 1s (so that overflow can be detected). The two STKDIS control bits can
 * disable this behavior for either counter, so that the associated counter
 * wraps back to all 0s and continues counting. This is useful if one counter-s
 * carry-out is used as the input to the other counter.
 *
 * Table 10-47 describes the Pchip monitor control register (PMONCTL).
 *
 *	Table 10-47 Pchip Monitor Control (PMONCTL)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:18>		MBZ,RAZ	0		Reserved
 *	STKDIS1		<17>		RW		0		Sticky count1 disable
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		PMONCNT<CNT1> sticks at all
 *													1s.
 *											1		PMONCNT<CNT1> wraps at all
 *													1s.
 *											-----------------------------------
 *	STKDIS0		<16>		RW		0		Sticky count0 disable
 *											-----------------------------------
 *											Value	Mode
 *											-----------------------------------
 *											0		PMONCNT<CNT0> sticks at all
 *													1s.
 *											1		PMONCNT<CNT0> wraps at all
 *													1s.
 *											-----------------------------------
 *	SLCT1		<15:8>		RW		0		Selects chip output b_monitor<1>,
 *											which is also the input to the
 *											least significant bit of
 *											PMONCNT<CNT1>.
 *	SLCT0		<7:0>		RW		1		Selects chip output b_monitor<0> on
 *											reset; used to differentiate
 *											between current and previous
 *											revisions of the Pchip. Also input
 *											to the least significant bit of
 *											PMONCNT<CNT0>.
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	slct0	: 8;	/* Selects chip output b_monitor<0> on reset */
	u64	slct1	: 8;	/* Selects chip output b_monitor<1> on reset */
	u64	stkdis0	: 1;	/* Sticky count0 disable */
	u64	stkdis1	: 1;	/* Sticky count1 disable */
	u64	res_18	: 46;	/* Reserved at bits 63:18 */
} AXP_21274_PMONCTL;

/*
 * The following macros mask in or out bits that are Reserved, Read-Only,
 * Write-Only, and Read-Write.  Reserved is masked out for both reads and
 * writes.  Read-Only are masked out for writes.  Write-only are masked out for
 * Reads.  Read-Write are masked in for both reads and writes.
 */
#define AXP_21274_PMONC_RMASK	0x000000000003FFFF
#define AXP_21274_PMONC_WMASK	0x000000000003FFFF

/*
 * Definitions for the values in various fields in the PMONCTL.
 */
#define AXP_STKDIS_STICKS_1S	0
#define AXP_STKDIS_WRAPS_1S		1

/*
 * HRM 10.2.5.12 Pchip Monitor Counters (PMONCNT - RO)
 *
 * The two fields CNT0 and CNT1 count the system clock cycles during which the
 * b_monitor<0> and b_monitor<1> signals respectively (selected by
 * PMONCTL<SLCT0> and PMONCTL<SLCT1>) are asserted.
 *
 * Both fields are cleared when any value is written to PMONCTL.
 *
 * Each of the counters sticks at the value of all 1s, unless the associated
 * STKDIS bit is set in PMONCTL.
 *
 * The counters both hold their values for four cycles each time that a read to
 * PMONCNT is performed. A slight inaccuracy can result if the events being
 * counted continue to occur at the time of the reading.
 *
 * Table 10-48 describes the Pchip monitor counters (PMONCNT).
 *
 *	Table 10-48 Pchip Monitor Counters (PMONCNT)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	CNT1		<63:32>		RO		0		Counts i_sysclk cycles that
 *											b_monitor<1> is asserted
 *	CNT0		<31:0>		RO		0		Counts sysclk cycles that
 *											monitor<0> is asserted
 *	---------------------------------------------------------------------------
 */
typedef struct
{
		u32	cnt0;			/* Counts sysclk cycles that monitor<0> is asserted */
		u32	cnt1;			/* Counts i_sysclk cycles that monitor<0> is asserted */
} AXP_21274_PMONCNT;

/*
 * This register is not completely documented, but referred to in a number of
 * places in the HRM.  The CSR for this is used as the Soft PCI Reset Register.
 */
typedef u64 AXP_21274_SPRST;

#endif /* _AXP_21274_REGISTERS_H_ */
