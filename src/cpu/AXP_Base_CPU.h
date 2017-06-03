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
 *	This header file contains useful definitions to be used throughout the
 *	Digital Alpha AXP emulation software that are common to all Alpha AXP
 *	Processors.
 *
 *	Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		26-May-2017	Jonathan D. Belanger
 *	Move some base IPRs from the 21264 IPR definitions to here.  Added a
 *	definition for a Page Table Entry.  Also, changed the Stack Pointers to
 *	pointers that will point to other locations (KSP is either R30 or [HW]PCB).
 */
#ifndef _AXP_BASE_CPU_DEFS_
#define _AXP_BASE_CPU_DEFS_

#include "AXP_Utility.h"

#define AXP_MAX_REGISTERS	32
#define AXP_INSTRUCTION_SIZE 4	/* Number of bytes in an Alpha AXP Instruction */

/*
 * Program Counter (PC) Definition
 */
typedef struct
{
	u64 pal : 1;
	u64 res : 1;
	u64 pc  : 62;
} AXP_PC;

/*
 * Page Table Entry (PTE) Definition
 */
typedef struct
{
	u64	v : 1;			/* Valid Bit */
	u64	_for : 1;		/* Fault on Read */
	u64	fow : 1;		/* Fault on Write */
	u64	foe : 1;		/* Fault on Execute */
	u64	_asm : 1;		/* Address Space Match */
	u64	gh : 2;			/* Granularity Hint */
	u64	nomb : 1;		/* Translation Buffer Miss Memory Barrier */
	u64	kre : 1;		/* Kernel Read Enabled */
	u64	ere_ure : 1;	/* Executive (OpenVMS)/User (UNIX) Read Enabled */
	u64	sre : 1;		/* Supervisor Read Enabled */
	u64	ure : 1;		/* User Read Enabled */
	u64	kwe : 1;		/* Kernel Write Enabled */
	u64	ewe_uwe : 1;	/* Executive (OpenVMS)/User (UNIX) Write Enabled */
	u64	swe : 1;		/* Supervisor Write Enabled */
	u64	uwe : 1;		/* User Write Enabled */
	u64	res : 16;
	u64	prf : 32;		/* Page Frame Number */
} AXP_PTE;

/*
 * The following are for IPRs defined in the Architectural Reference Manual
 *
 *											Input		Output		Context
 *	Register Name		Mnemonic	Access	R16			R0			Switched
 *	-------------------	--------	-------	---------	---------	--------
 *	Address Space Num	ASN			R		?			Number		Yes
 *	AST Enable			ASTEN		R/W		Mask		Mask		Yes
 *	AST Summary Reg		ASTSR		R/W		Mask		Mask		Yes
 *	Data Align Trap Fix	DATFX		W		Value		?			Yes
 *	Executive Stack Ptr	ESP			R/W		Address		Address		Yes
 *	Floating-point Ena	FEN			R/W		Value		Value		Yes
 *	Interproc Int. Req	IPIR		W		Number		?			No
 *	Interrupt Prio Lvl	IPL			R/W		Value		Value		No
 *	Kernel Stack Ptr	KSP			None	?			?			Yes
 *	Machine Chk Err Sum	MCES		R/W		Value		Value		No
 *	*Perf Monitoring	PERFMON		W		IMP			IMP			No
 *	Priv Ctx Blk Base	PCBB		R		?			Address		No
 *	Proc Base Register	PRBR		R/W		Value		Value		No
 *	Page Table Base Reg	PTBR		R		?			Frame		Yes
 *	Sys Ctrl Blk Base	SCBB		R/W		Frame		Frame		No
 *	S/W Int. Req Reg	SIRR		W		Level		?			No
 *	S/W Int. Summ Reg	SISR		R		?			Mask		No
 *	Supervi Stack Ptr	SSP			R/W		Address		Address		Yes
 *	Sys Page Tbl Base	SYSPTBR		R/W		Value		Value		Yes
 *	TB Check			TBCHK		R		Number		Status		No
 *	TB Invalid. All		TBIA		W		?			?			No	Pseudo
 *	TB Inv. All Proc	TBIAP		W		?			?			No	Pseudo
 *	TB Invalid. Single	TBIS		W		Address		?			No	Pseudo
 *	TB Inv. Single Data	TBISD		W		Address		?			No	Pseudo
 *	TB Inv. Singl Instr	TBISI		W		Address		?			No	Pseudo
 *	User Stack Pointer	USP			R/W		Address		Address		Yes
 *	Virt Addr Boundary	VIRBND		R/W		Address		Address		Yes
 *	Virt Page Tbl Base	VPTB		R/W		Address		Address		No
 *	Who-Am-I			WHAMI		R		?			Number		No
 *	*PERFMON is implementation specific and not defined in the BASE IPRs.
 */
typedef u64 AXP_BASE_ASN;

typedef struct
{
	u64	ken : 1;
	u64	een : 1;
	u64 sen : 1;
	u64 uen : 1;
	u64 res : 60;
} AXP_BASE_ASTEN;

typedef struct
{
	u64	kcl : 1;
	u64	ecl : 1;
	u64 scl : 1;
	u64 ucl : 1;
	u64	kon : 1;
	u64	eon : 1;
	u64 son : 1;
	u64 uon : 1;
	u64 res : 56;
} AXP_BASE_ASTEN_R16;

typedef struct
{
	u64	ken : 1;
	u64	een : 1;
	u64 sen : 1;
	u64 uen : 1;
	u64 res : 60;
} AXP_BASE_ASTSR;

typedef struct
{
	u64	kcl : 1;
	u64	ecl : 1;
	u64 scl : 1;
	u64 ucl : 1;
	u64	kon : 1;
	u64	eon : 1;
	u64 son : 1;
	u64 uon : 1;
	u64 res : 56;
} AXP_BASE_ASTSR_R16;

typedef struct
{
	u64	dat : 2;
	u64 res : 62;
} AXP_BASE_DATFX;

typedef u64 *AXP_BASE_ESP;	/* HWPCB+8 (OpenVMS) n/a (UNIX) */

typedef struct
{
	u64	fen : 1;
	u64 res : 63;
} AXP_BASE_FEN;

typedef u64 AXP_BASE_IPIR;

typedef struct
{
	u64	ipl : 4;
	u64 res : 60;
} AXP_BASE_IPL;

typedef u64 *AXP_BASE_KSP;	/* HWPCB+0 (OpenVMS) PCB+0 (UNIX) */

typedef struct
{
	u64	mck : 1;
	u64	sce : 1;
	u64	pce : 1;
	u64 dpc : 1;
	u64 dsc : 1;
	u64 res : 27;
	u64 imp : 32;
} AXP_BASE_MCES;

typedef struct
{
	u64 pa : 48;
	u64 res : 16;
} AXP_BASE_PCBB;

typedef u64 AXP_BASE_PRBR;

typedef struct
{
	u32 pfn;
	u32 res;
} AXP_BASE_PTBR;

typedef struct
{
	u32 pfn;
	u32 res;
} AXP_BASE_SCBB;

typedef struct
{
	u64	lvl : 4;
	u64 res : 60;
} AXP_BASE_SIRR;

typedef struct
{
	u64 res_1 : 1;
	u64 ir1 : 1;
	u64 ir2 : 1;
	u64 ir3 : 1;
	u64 ir4 : 1;
	u64 ir5 : 1;
	u64 ir6 : 1;
	u64 ir7 : 1;
	u64 ir8 : 1;
	u64 ir9 : 1;
	u64 ira : 1;
	u64 irb : 1;
	u64 irc : 1;
	u64 ird : 1;
	u64 ire : 1;
	u64 irf : 1;
	u64 res_2 : 48;
} AXP_BASE_SISR;

typedef u64 *AXP_BASE_SSP;	/* HWPCB+16 (OpenVMS) n/a (UNIX) */

typedef struct
{
	u32 pfn;
	u32 res;
} AXP_BASE_SYSPTBR;

typedef struct
{
	u64 prs : 1;
	u64 res_1 : 62;
	u64 imp : 1;
} AXP_BASE_TBCHK;

typedef u64 AXP_BASE_TBCHK_R16;

typedef u64 *AXP_BASE_USP;	/* HWPCB+24 (OpenVMS) PCB+8 (UNIX) */

typedef u64 AXP_BASE_VIRBND;

typedef u64 AXP_BASE_VPTB;

typedef u64 AXP_BASE_WHAMI;

#endif /* _AXP_BASE_CPU_DEFS_ */
