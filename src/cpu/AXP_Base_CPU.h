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

#define AXP_MAX_REGISTERS		32
#define AXP_INSTRUCTION_SIZE	4	/* Number of bytes in an Alpha AXP Instruction */
#define AXP_PAL_MODE			1
#define AXP_NORMAL_MODE			0

/*
 * Register Definitions
 *
 * The integer register is either signed or unsigned.
 */
typedef union
{
	u8		ub;
	u16		uw;
	u32		ul;
	i64		sq;
	u64		uq;
} AXP_INT_REGISTER;

/*
 * Integer Memory Format Definitions
 *
 * Byte Memory format (8-bits).
 */
typedef union
{
	i8		sb;
	u8		ub;
} AXP_B_MEMORY;

/*
 * Word Memory format (16-bits).
 */
typedef union
{
	i16		sw;
	u16		uw;
} AXP_W_MEMORY;

/*
 * Longword Memory format (32-bits).
 */
typedef union
{
	i32		sl;
	u32		ul;
} AXP_L_MEMORY;

/*
 * Quadword Memory format (64-bits).
 */
typedef union
{
	i64		sq;
	u64		uq;
} AXP_Q_MEMORY;

/*
 * Floating-point registers have a number of formats.
 *
 * VAX F Float Register format.
 */
typedef struct
{
	u64		zero : 29;
	u64		fractionLow : 16;
	u64		fractionHigh : 7;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_F_REGISTER_CVT;

typedef struct
{
	u64		zero : 29;
	u64		fraction : 23;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_F_REGISTER;

/*
 * VAX G Float Register format.
 */
typedef struct
{
	u64		fractionLow : 16;
	u64		fractionMidLow : 16;
	u64		fractionMidHigh : 16;
	u64		fractionHigh : 4;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_G_REGISTER_CVT;
typedef struct
{
	u64		fraction : 52;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_G_REGISTER;

/*
 * VAX D Float Register format.
 */
typedef struct
{
	u64		fractionLow : 16;
	u64		fractionMidLow : 16;
	u64		fractionMidHigh : 16;
	u64		fractionHigh : 7;
	u64		exponent : 8;
	u64		sign : 1;
} AXP_D_REGISTER_CVT;
typedef struct
{
	u64		fraction : 55;
	u64		exponent : 8;
	u64		sign : 1;
} AXP_D_REGISTER;

/*
 * IEEE S Float Register format.
 */
typedef struct
{
	u64		zero : 29;
	u64		fraction : 23;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_S_REGISTER_CVT;
typedef struct
{
	u64		fraction : 52;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_S_REGISTER;

/*
 * IEEE T Float Register format.
 */
typedef struct
{
	u64		fraction : 52;
	u64		exponent : 11;
	u64		sign : 1;
} AXP_T_REGISTER;

/*
 * IEEE X Float Register format.
 * 	This format occupies 2 consecutive Floating-point register, with the first
 * 	being an even numbered register.  This structure occupies 128 bits.
 */
typedef struct
{
	u64		fractionLow;
	u64		fractionHigh : 48;
	u64		exponent : 15;
	u64		sign : 1;
} AXP_X_REGISTER;

/*
 * Longword Integer Float Register format.
 */
typedef struct
{
	u64		zero_1 : 29;
	u64		integerLow : 30;
	u64		zero_2 : 3;
	u64		integerHigh : 1;
	u64		sign : 1;
} AXP_L_REGISTER;

/*
 * Quadword Integer Float Register format.
 */
typedef struct
{
	u64		integer : 63;
	u64		sign : 1;
} AXP_Q_REGISTER;

typedef struct
{
	u64		integerLow : 62;
	u64		integerHigh : 1;
	u64		sign : 1;
} AXP_Q_REGISTER_CVT;

typedef struct
{
	u64		integerLow : 32;
	u64		integerHigh : 31;
	u64		sign : 1;
} AXP_Q_REGISTER_V;

typedef struct
{
	u64		integerLowLow : 31;
	u64		integerLowHigh : 1;
	u64		integerHigh : 31;
	u64		sign : 1;
} AXP_Q_REGISTER_V_CVT;

/*
 * Floating-Point Register format.
 * 	Because IEEE X Float occupies 2 registers, it is not put in the following
 * 	union declaration.
 */
typedef union
{
	u64					uq;
	i64					sq;
	AXP_F_REGISTER_CVT	fCvt;
	AXP_F_REGISTER		f;
	AXP_G_REGISTER_CVT	gCvt;
	AXP_G_REGISTER		g;
	AXP_D_REGISTER_CVT	dCvt;
	AXP_D_REGISTER		d;
	AXP_S_REGISTER_CVT	sCvt;
	AXP_S_REGISTER		s;
	AXP_T_REGISTER		t;
	AXP_L_REGISTER		l;
	AXP_Q_REGISTER		q;
	AXP_Q_REGISTER_CVT	qCvt;
	AXP_Q_REGISTER_V	qV;
	AXP_Q_REGISTER_V_CVT qVCvt;
} AXP_FP_FORMATS;

typedef enum
{
	FPInteger,
	VAXfFloat,
	VAXgFloat,
	VAXdFloat,
	IEEEFloat,
	IEEEZero,
	IEEEFinite,
	IEEEDenormal,
	IEEEInfinity,
	IEEENotANumber
} AXP_FP_CONTENT;

typedef struct
{
	AXP_FP_FORMATS		fpv;
	AXP_FP_CONTENT		fpc;
} AXP_FP_REGISTER;

typedef union
{
	AXP_INT_REGISTER	r;
	AXP_FP_REGISTER		fp;
} AXP_REGISTER;

/*
 * Floating-Point Memory Format Definitions
 * 	The strangeness of these layouts has to do with how the registers and
 * 	silicon for the VAX architecture was defined.  Memory format, and thus
 * 	on-disk format for the VAX Float formats are in the VAX Architecture
 * 	layout.  This was something written to disk by a VAX can be read in by an
 * 	Alpha AXP and vice versa.  Gotta love backward compatibility (a Digital
 * 	hallmark).
 *
 * VAX F Float Memory format (32-bit structure)
 */
typedef struct
{
	u32		fractionHigh : 7;
	u32		exponent : 8;
	u32		sign : 1;
	u32		fractionLow : 16;
} AXP_F_MEMORY;

/*
 * VAX G Float Memory format (64-bit structure).
 */
typedef struct
{
	u64		fractionHigh : 4;
	u64		exponent : 11;
	u64		sign : 1;
	u64		fractionMidHigh : 16;
	u64		fractionMidLow : 16;
	u64		fractionLow : 16;
} AXP_G_MEMORY;

/*
 * VAX D Float Memory format (64-bit structure).
 */
typedef struct
{
	u64		fractionHigh : 7;
	u64		exponent : 8;
	u64		sign : 1;
	u64		fractionMidHigh : 16;
	u64		fractionMidLow : 16;
	u64		fractionLow : 16;
} AXP_D_MEMORY;

/*
 * IEEE S Float Memory format (32-bit structure).
 */
typedef struct
{
	u32		fraction : 23;
	u32		exponent : 8;
	u32		sign : 1;
} AXP_S_MEMORY;

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

/*
 * The following definitions are used to convert addresses from to their
 * big-endian form.  These definitions are XOR'd with the supplied address,
 * depending upon the size of the datum being loaded/stored:
 *
 *	Size		Bits	Hex
 *	--------	----	---
 *	Quadword	000		0x0
 *	Longword	100		0x4
 *	Word		110		0x6
 *	Byte		111		0x7
 */
#define AXP_BIG_ENDIAN_QUAD(addr)	(addr)
#define AXP_BIG_ENDIAN_LONG(addr)	((addr) ^ 0x4)
#define AXP_BIG_ENDIAN_WORD(addr)	((addr) ^ 0x6)
#define AXP_BIG_ENDIAN_BYTE(addr)	((addr) ^ 0x7)

/*
 * The following definitions will mask out various lengths of integers within
 * a 64-bit register.
 */
#define AXP_BYTE_MASK(val)			((val) & 0x00000000000000ffll)
#define AXP_WORD_MASK(val)			((val) & 0x000000000000ffffll)
#define AXP_LONG_MASK(val)			((val) & 0x00000000ffffffffll)
#define AXP_QUAD_MASK(val)			(val)

/*
 * These definitions zero extend various sized data
 */
#define AXP_ZEXT_BYTE(val)		(0x00000000000000ffll & (val))
#define AXP_ZEXT_WORD(val)		(0x000000000000ffffll & (val))
#define AXP_ZEXT_LONG(val)		(0x00000000ffffffffll & (val))
#define AXP_ZEXT_QUAD(val)		(val)

/*
 * These definitions zero extend various sized data
 */
#define AXP_SEXT_BYTE(val)												\
	((val) & 0x0000000000000080ll) ? (0xffffffffffffff00ll | (val)) :	\
			AXP_ZEXT_BYTE(val)
#define AXP_SEXT_WORD(val)												\
	((val) & 0x0000000000008000ll) ? (0xffffffffffff0000ll | (val)) :	\
			AXP_ZEXT_WORD(val)
#define AXP_SEXT_LONG(val)												\
	((val) & 0x0000000080000000ll) ? (0xffffffff00000000ll | (val)) :	\
			AXP_ZEXT_LONG(val)
#define AXP_SEXT_QUAD(val)		(val)

/*
 * The following list the possible exceptions, interrupts and Machine Checks.
 *
 *	Table: Exceptions, Interrupts, and Machine Checks Summary
 *								SavedPC	NewMode	R02		R03		R04		R05
 *								-------	-------	----	----	-----	---
 *	Exceptions -- Faults:
 *	Floating Disabled Fault		Current	Kernel	SCBv	SCBp
 *
 *	Memory Management Faults:
 *	Access Control Violation	Current	Kernel	SCBv	SCBp	VA		MMF
 *	Translation Not Valid		Current	Kernel	SCBv	SCBp	VA		MMF
 *	Fault on Read				Current	Kernel	SCBv	SCBp	VA		MMF
 *	Fault on Write				Current	Kernel	SCBv	SCBp	VA		MMF
 *	Fault on Execute			Current	Kernel	SCBv	SCBp	VA		MMF
 *
 *	Exceptions -- Arithmetic Traps:
 *	Arithmetic Traps			Next	Kernel	SCBv	SCBp	Mask	Exc
 *
 *	Exceptions -- Synchronous Traps:
 *	Breakpoint Trap				Next	Kernel	SCBv	SCBp
 *	Bugcheck Trap				Next	Kernel	SCBv	SCBp
 *	Change Mode to K/E/S/U		Next	MostPrv	SCBv	SCBp
 *	Illegal Instruction			Next	Kernel	SCBv	SCBp
 *	Illegal Operand				Next	Kernel	SCBv	SCBp
 *	Data Alignment Trap			Next	Kernel	SCBv	SCBp	VA		RW
 *
 *	Interrupts:
 *	Asynch System Trap (4)		Current	Kernel	SCBv	SCBp
 *	Interval Clock				Current	Kernel	SCBv	SCBp
 *	Interprocessor Interrupt	Current	Kernel	SCBv	SCBp
 *	Software Interrupts			Current	Kernel	SCBv	SCBp
 *	Performance Monitor			Current	Kernel	SCBv	SCBp	IMP		IMP
 *	Passive Release				Current	Kernel	SCBv	SCBp
 *	Powerfail					Current	Kernel	SCBv	SCBp
 *	I/o Device					Current	Kernel	SCBv	SCBp
 *
 *	Machine Checks:
 *	Processor Correctable		Current	Kernel	SCBv	SCBp	LAOff
 *	System Correctable			Current	Kernel	SCBv	SCBp	LAOff
 *	System						Current	Kernel	SCBv	SCBp	LAOff
 *	Processor					Current	Kernel	SCBv	SCBp	LAOff
 *
 *	Current = 	PC of the instruction at which the exception or interrupt or
 *				machine check was taken.
 *	MostPrv = 	More privileged of the current and new modes
 *	SCBv =		The SCB vector quadword.
 *	SCBp =		The SCB parameter quadword.
 *	VA =		Exact virtual address that triggered the memory management
 *				fault or data alignment trap
 *	Mask =		Register Write Mask
 *	LAOff =		Offset from base logoff area in the HWRPB
 *	MMF =		Memory Management Flags
 *	Exc =		Exception Summary parameter
 *	RW =		Read/Load=0, Write/Store=1 for data alignment traps
 */
typedef enum
{
	NoException,
	FloatingDisabledFault,
	AccessControlViolation,
	TranslationNotValid,
	FaultOnRead,
	FaultOnWrite,
	FaultOnExecute,
	ArithmeticTraps,
	BreakpointTrap,
	BucgcheckTrap,
	ChangeModeToKESU,
	IllegalInstruction,
	IllegalOperand,
	DataAlignmentTrap,
	AST,
	IntervalClock,
	InterprocessorInterrupt,
	SoftwareInterrupts,
	PerformanceMonitor,
	PassiveRelease,
	Powerfail,
	IODevice,
	ProcessorCorrectable,
	SystemCorrectable,
	System,
	Processor
} AXP_EXCEPTIONS;

typedef struct
{
	AXP_EXCEPTIONS	exception;
	AXP_PC			savedPC;
	u64				R02;
	u64				R03;
	u64				R04;
	u64				R05;
} AXP_EXCEPT_INFO;

#endif /* _AXP_BASE_CPU_DEFS_ */
