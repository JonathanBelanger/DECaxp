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
 * they are not the same. Table 10–9 describes the Tsunami Cchip configuration
 * register (CSC). Table 10–10 describes the Typhoon Cchip configuration
 * register (CSC).  Below is the Typhoon CSC, as this is a superset of the
 * Tsunami CSC.
 *
 *	Table 10–10 Cchip System Configuration Register (CSC) (Typhoon Only)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES 		<63> 		MBZ,RAZ	0		Reserved.
 *	RES			<62>		MBZ,RAZ	0		Reserved.
 *	P1W(1)		<61>		RO		1		= Wide PADbus 1. (Typhoon only)
 *	P0W(1)		<60>		RO		1		= Wide PADbus 0. (Typhoon only)
 *	RES			<59>		MBZ,RAZ	0		Reserved.
 *	PBQMAX(2)	<58:56>		RW		1		CPU probe queue maximum – 0
 *											indicates 8 entries.
 *	RES			<55>		MBZ,RAZ	0		Reserved.
 *	PRQMAX		<54:52>		RW		2		Maximum requests to one Pchip until
 *											ACK, modulo 8 – the value 1 is
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
 *											0		1 cycle – SED must be 2 or
 *													3 cycles.
 *											1		2 cycles – SED must be 3,
 *													4, or 5 cycles.
 *											2		3 cycles – SED must be 3,
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
 *											(see Table 7–5).
 *											-----------------------------------
 *											Value	Cycles
 *											-----------------------------------
 *											0		3 cycles
 *											1		4 cycles
 *											2		5 cycles
 *											3		6 cycles
 *											-----------------------------------
 *	IDDR		<11:9>		RO(4)	4		Issue to data delay for memory
 *											reads (see Table 7–5).
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
 *	SED(5)		<5:4>		RO				SysDC extract delay – The number of
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
#define AXP_QPM_MODIFIED_RR			0
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

/*
 * HRM 10.2.2.2 - Memory Timing Register(MTR - RW) (page 10-26)
 *
 *	Table 10–11 Memory Timing Register (MTR) (Continued)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:46>		MBX,RAZ	0		Reserved.
 *	MPH			<45:40>		RW		0		Maximum page hits – The most page
 *											hits the memory controller allows
 *											before forcing a page to be closed.
 *											The issue unit can, under some
 *											circumstances, sneak in one more
 *											page hit than this parameter
 *											allows.
 *	PHCW		<39:36>		RW		14		Page hit cycles for writes – The
 *											number of cycles that the memory
 *											controller must wait after a write
 *											is issued until it attempts to
 *											close the page.
 *
 *											Note: Must be greater than or equal
 *											to the greater of (IRD + RPW – 2)
 *											and (IRD + RCD + bl + tRWL – 3),
 *											where bl is 4 for 16-byte buses and
 *											2 for 32-byte buses.
 *
 *											Note: Initializes as 14 in the Rev.
 *											C Cchip; initializes as 15 in the
 *											Rev. B Cchip.
 *	PHCR		<35:32>		RW		15		Page hit cycles for reads – The
 *											number of cycles that the memory
 *											controller must wait after a read
 *											is issued until it attempts to
 *											close the page.
 *
 *											Note: Must be greater than or equal
 *											to the greater of (RPW – 2) and
 *											(RCD + bl – 2) where bl is 4 for
 *											16-byte buses and 2 for 32-byte
 *											buses.
 *	RES			<31:30>		MBZ,RAZ	0		Reserved.
 *	RI			<29:24>		RW		0		Refresh interval – The number of
 *											cycles per refresh interval divided
 *											by 64. Each DRAM is refreshed once
 *											per refresh interval. A value of 0
 *											disables refreshing.  The values 1,
 *											2, and 3 are illegal.
 *	RES			<23:21>		MBZ,RAZ	0		Reserved.
 *	MPD			<20>		RW		0		Mask pipeline delay – The
 *											b_mndqm<1:0> signals to the SDRAMs
 *											may need to be buffered. Setting
 *											this bit causes the memory
 *											controllers to signal the DQM masks
 *											one cycle earlier to compensate.
 *	RRD			<16>		RW		0		Minimum same-array different-bank
 *											RAS-to-RAS delay – The minimum
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
 *	RPT			<13:12>		RW		0		Minimum RAS precharge time – The
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
 *	RPW			<9:8>		RW		0		Minimum RAS pulse width (tRAS) –
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
 *	IRD			<6:4>		RW		0		Issue to RAS delay – For memory
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
 *	CAT			<2>			RW		0		CAS access time – The number of
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
 *	RCD			<0>			RW		0		RAS-to-CAS delay – The number of
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
 * HRM 10.2.2.3 Miscellaneous Register (MISC – RW) (page 10-29)
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
 * bits and unlocks the ABW field. Table 10–12 describes the miscellaneous
 * register (MISC).
 *
 * 	Table 10–12 Miscellaneous Register (MISC)
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
 *	REV			<39:32>		RO		—		Latest revision of Cchip:
 *									1		21272 (Tsunami)
 *									8		21274 (Typhoon)
 *	NXS			<31:29>		RO		0		NXM source – Device that caused the
 *											NXM – UNPREDICTABLE if NXM is not
 *											set.
 *											-----------------------------------
 *											Value	Source
 *											-----------------------------------
 *											0		CPU 0
 *											1		CPU 1
 *											2		Reserved
 *													CPU2 – Typhoon only
 *											3		Reserved
 *													CPU3 – Typhoon only
 *											4		Pchip 0
 *											5		Pchip 1
 *											6, 7	Reserved
 *											-----------------------------------
 *	NXM			<28>		R,W1C	0		Nonexistent memory address
 *											detected.  Sets DRIR<63> and locks
 *											the NXS field until it is cleared.
 *	RES			<27:25>		MBZ,RAZ	0		Reserved.
 *	ACL			<24>		WO		0		Arbitration clear – writing a 1 to
 *											this bit clears the ABT and ABW
 *											fields.
 *	ABT			<23:20>		R,W1S	0		Arbitration try – writing a 1 to
 *											these bits sets them. (<23:22> are
 *											used in Typhoon only)
 *	ABW			<19:16>		R,W1S	0		Arbitration won – writing a 1 to
 *											these bits sets them unless one is
 *											already set, in which case the
 *											write is ignored.
 *											(<19:18> are used in Typhoon only)
 *	IPREQ		<15:12>		WO		0		Interprocessor interrupt request –
 *											write a 1 to the bit corresponding
 *											to the CPU you want to interrupt.
 *											Writing a 1 here sets the
 *											corresponding bit in the IPINTR.
 *											(<15:14> are used in Typhoon only)
 *	IPINTR		<11:8>		R,W1C	0		Interprocessor interrupt pending –
 *											one bit per CPU. Pin irq<3> is
 *											asserted to the CPU corresponding
 *											to a 1 in this field.
 *											(<11:10> are used in Typhoon only)
 *	ITINTR		<7:4>		R,W1C	0		Interval timer interrupt pending –
 *											one bit per CPU. Pin irq<2> is
 *											asserted to the CPU corresponding
 *											to a 1 in this field.
 *											(<7:6> are used in Typhoon only)
 *	RES			<3:2>		MBZ,RAZ	0		Reserved.
 *	CPUID		<1:0>		RO		—		ID of the CPU performing the read.
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
 * HRM 10.2.2.4 Memory Presence Detect Register (MPD – RW)
 *
 * The memory presence detect register is connected to two open-drain pins on
 * the Cchip.  These pins can be used by software to implement the I2C protocol
 * to read the serial presence detect pins on the SDRAM DIMMs. Table 10–13
 * describes the memory presence detect register (MPD).
 *
 * Table 10–13 Memory Presence Detect Register (MPD)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:4>		MBZ,RAZ	0		Reserved
 *	DR			<3>			RO		1		Data receive
 *	CKR			<2>			RO		1		Clock receive
 *	DS			<1>			WO		1		Data send – Must be a 1 to receive
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
 * HRM 10.2.2.5 Array Address Register (AAR0, AAR1, AAR2, AAR3 – RW)
 *
 *	Table 10–15 Array Address Register (AAR0, AAR1, AAR2, AAR3) (Typhoon Only)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:35>		MBZ,RAZ	0		Reserved.
 *	ADDR		<34:24>		RW		0		Base address – Bits <34:24> of the
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
 *											1011–	Reserved.
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
} AXP_21274_ARRx;

/*
 * HRM 10.2.2.6 Device Interrupt Mask Register (DIMn, n=0,3 – RW)
 *
 * Register n applies to CPUn. (Typhoon only: n=2,3.)
 *
 * These two mask registers control which interrupts are allowed to go through
 * to the CPUs. No interrupt in DRIR will get through to the masked interrupt
 * registers (and on to interrupt the CPUs) unless the corresponding mask bit
 * is set in DIMn. All bits are initialized to 0 at reset. Table 10–16
 * describes the device interrupt mask registers.
 *
 *	Table 10–16 Device Interrupt Mask Register (DIMn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	DIM 		<63:0>		RW		0		Interrupts allowed through to the
 *											CPU
 *	---------------------------------------------------------------------------
 */
typedef u64 AXP_21274_DIMn;

/*
 * HRM 10.2.2.7 Device Interrupt Request Register (DIRn, n=0,3 – RO)
 *
 * Register n applies to CPUn. (Typhoon only: n=2,3.)
 *
 * These two registers indicate which interrupts are pending to the CPUs. If a
 * raw request bit is set and the corresponding mask bit is set, then the
 * corresponding bit in this register will be set and the appropriate CPU will
 * be interrupted. Table 10–17 describes the device interrupt request
 * registers.
 *
 *	Table 10–17 Device Interrupt Request Register (DIRn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	ERR			<63:58>		RO		0		IRQ0 error interrupts
 *											<63> Chip detected MISC<NXM>
 *											<62> recommended hookup to Pchip0
 *											error
 *											<61> recommended hookup to Pchip1
 *											error
 *											Others per module designer’s choice
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
 * HRM 10.2.2.8 Device Raw Interrupt Request Register (DRIR – RO)
 *
 * DRIR indicates which of the 64 possible device interrupts is asserted. Table
 * 10–18 describes the device raw interrupt request register (DRIR).
 *
 *	Table 10–18 Device Interrupt Mask Register (DIMn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	DRIR 		<63:0>		RW		0		Interrupts pending from devices
 *	---------------------------------------------------------------------------
 */
typedef u64 AXP_21274_DRIR;

/*
 * HRM 10.2.2.9 Probe Enable Register (PRBEN – RW)
 *
 * This register is special in that reads do not return the value of the
 * register, but rather cause the probe enable bit for the requesting CPU to be
 * cleared. The return data is UNPREDICTABLE. Writing to this register causes
 * the probe enable bit for the requesting CPU to be set, regardless of the
 * value written. Table 10–19 describes the probe enable register (PRBEN).
 *
 *	Table 10–19 Probe Enable Register (PRBEN)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:1>		MBZ		0		Reserved
 *	PRBEN		<0>			RTC,WTS	0		Probe enable bit
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	prben	: 1;	/* Probe enable bit */
	u64	res_2	: 63;	/* Reserved at bits 63:2 */
} AXP_21274_PRBEN;

/*
 * HRM Interval Ignore Count Register (IICn, n=0,3 – RW)
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
 * exactly how many interval timer ticks were skipped. Table 10–20 describes
 * the interval ignore count register (IIC).
 *
 *	Table 10–20 Interval Ignore Count Register (IIC)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:25>		MBZ,RAZ	0		Reserved
 *	OF			<24>		RO		0		Overflow – Indicates negative count
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
 * HRM 10.2.2.11 Wake-Up Delay Register (WDR – RW)
 *
 * The WDR register determines how long (in system cycles) the chipset waits
 * after a reset, or after sending a wake-up interrupt to a sleeping CPU,
 * before deasserting b_cfrst<1:0>.
 *
 *	Table 10–21 Wake-Up Delay Register (WDR)
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
 * HRM 10.2.2.12 Memory Programming Register (MPR0, MPR1, MPR2, MPR3 – WO)
 *
 * A write to these registers causes a RAM program cycle (a mode register set
 * command) to the associated memory array using the data written to the MPRDAT
 * field. Table 10–22 describes the memory programming registers.
 *
 *	Table 10–22 Memory Programming Register (MPRn)
 *	---------------------------------------------------------------------------
 *	Field		Bits		Type	Init	Description
 *	---------------------------------------------------------------------------
 *	RES			<63:13>		MBZ		0		Reserved
 *	MPRDAT		<12:0>		WO		—		Data to be written on address lines
 *											<12:0>
 *	---------------------------------------------------------------------------
 */
typedef struct
{
	u64	mprdat	: 13;	/* Data to be written on address lines <12:0> */
	u64	res_13	: 51;	/* Reserved at bits 63:13 */
} AXP_21274_MPRn;

/*
 * HRM 10.2.2.13 M-Port Control Register (MCTL – MBZ)
 *
 * The M-port control register controls chipset debug features. It must be 0
 * for normal operations. In Typhoon, this register is replaced by the CMONCTL
 * registers.
 */
typedef u64 AXP_21274_MCTL;

#endif /* _AXP_21274_REGISTERS_H_ */
