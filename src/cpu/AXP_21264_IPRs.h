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
 *	This header file contains the structures and definitions for the Internal
 *	Processor Registers (IPRs) for the 21264 generation of the Alpha AXP CPU.
 *
 *	Revision History:
 *
 *	V01.000		08-May-2017	Jonathan D. Belanger
 *	Initially written with the Ebox, Ibox, Mbox and Cbox IPRs.
 *	NOTE:	Some of these registers may not be required.  Specifically, the
 *			Cbox registers are used to read initialization data either 6-bits
 *			or 1-bit at a time.
 *
 *	V01.001		10-May-2017	Jonathan D. Belanger
 *	Included the AXP Utility header file for definitions like u64, i64, etc..
 *
 *	V01.002		26-May-2017	Jonathan D. Belanger
 *	Move some base IPRs from here to the base definitions.
 */
#ifndef _AXP_21264_IPR_DEFS_
#define _AXP_21264_IPR_DEFS_

/*
 * The following definitions are for the Ebox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	Cycle counter		CC			1100 0000	5		RW		1L			1
 *	Cycle counter ctrl	CC_CTL		1100 0001	5		W0		1L			?
 *	Virtual address		VA			1100 0010	4,5,6,7	RO 		1L			1
 *	Virtual addr ctrl	VA_CTL		1100 0100	5		WO		1L			?
 *	Virtual addr format	VA_FORM		1100 0011	4,5,6,7	RO		1L			1
 */
typedef struct
{
	u32	counter;	
	u32	offset;
} AXP_EBOX_CC;				/* Cycle Counter Register */

typedef struct
{
	u32	res_1 : 4;
	u32 counter : 28;		/* CC[31:4] in AXP_EBOX_CC */
	u32	cc_ena : 1;			/* Counter Enable */
	u32 res_2 : 31;
} AXP_EBOX_CC_CTL;			/* Cycle Counter Control Register */

typedef u64 AXP_EBOX_VA;	/* Virtual Address Register */

typedef struct
{
	u64	b_endian : 1;		/* Big Endian Mode */
	u64	va_48 : 1;			/* 0 = 43 bit, 1 = 48 bit addressing */
	u64	va_form_32 : 1;		/* Controls interpretation of VA_FORM register */
	u64 res : 27;
	u64 vptb : 34;			/* Virtual Page Table Base */
} AXP_EBOX_VA_CTL;

typedef struct
{
	u64	res : 3;
	u64 va : 30;			/* Virtual Page Table Entry Address */
	u64 vptb : 31;			/* Virtual Page Table Base */
} AXP_EBOX_VA_FORM_00;

typedef struct
{
	u64	res : 3;
	u64 va_sext_vptb : 61;	/* Combined VA, SEXT, and VPTB */
} AXP_EBOX_VA_FORM_10;

/*
 * For the above VA_FORM format, the following three fields have overlapping
 * bits.
 *
 *	VPTB[63:43]
 *	SEXT(VA[47])
 *	VA[47:3]
 *
 * The following macros will extract and store these values appropriately.
 */
#define GET_VA(reg)			((reg) & 0x00003ffffffffff8) >> 3
#define SAVE_VA(reg, va)	(((reg) & 0xffffc00000000000) | ((va) << 3))
#define GET_SEXT(reg)		((reg) & 0x0000800000000000) >> 47
#define SAVE_SEXT(reg, sext) (((reg) & 0xffff7ffffffffff8) | ((sext) << 47))
#define GET_VPTB(reg)		((reg) & 0xffffff8000000000) >> 43
#define SAVE_VPTB(reg, vptb) (((reg)&0x0000007ffffffff8) | ((vptb) << 43))

typedef struct
{
	u64	res_1 : 3;
	u64 va : 19;			/* Virtual Page Table Entry Address */
	u64 res_2 : 8;
	u64 vptb : 34;			/* Virtual Page Table Base */
} AXP_EBOX_VA_FORM_01;

typedef union
{
	AXP_EBOX_VA_FORM_00	form00;		/* VA_48 = 0 and VA_FORM_32 = 0*/
	AXP_EBOX_VA_FORM_10	form10;		/* VA_48 = 1 and VA_FORM_32 = 0*/
	AXP_EBOX_VA_FORM_01	form01;		/* VA_48 = 0 and VA_FORM_32 = 1*/
} AXP_EBOX_VA_FORM;

/*
 * The following definitions are for the Fbox IPRs
 *	The dyn field has the following values:
 *		00 = Chopped
 *		01 = Minus infinity
 *		10 = Normal
 *		11 = Plus infinity
 */
typedef struct
{
	u64	res : 48;
	u64 dnz : 1;					/* Denormal operands to zero */
	u64 invd : 1;					/* Invalid operation disable */
	u64 dzed : 1;					/* Division by zero disabled */
	u64 ovfd : 1;					/* Overflow disabled */
	u64 inv : 1;					/* Invalid operation */
	u64 dze : 1;					/* Divide by zero */
	u64 ovf : 1;					/* Overflow */
	u64 unf : 1;					/* Underflow */
	u64 ine : 1;					/* Inexact result */
	u64 iov : 1;					/* Integer overflow */
	u64 dyn : 2;					/* Dynamic rounding mode */
	u64 undz : 1;					/* Underflow to zero */
	u64 unfd : 1;					/* Underflow disabled */
	u64 ined : 1;					/* Inexact disabled */
	u64 sum : 1;					/* Summary bit (OR of exception bits) */
} AXP_FBOX_FPCR;

/*
 * The following definitions are for the Ibox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	ITB tag array write	ITB_TAG		0000 0000	6		WO		0L			?
 *	ITB PTE array write	ITB_PTE		0000 0001	4,0		WO		0L			?
 *	ITB inval all proc	ITB_IAP		0000 0010	4		WO		0L			?
 *	(ASM=0)
 *	ITB invalidate all	ITB_IA		0000 0011	4		WO		0L			?		Pseudo
 *	ITB invalid single	ITB_IS		0000 0100	4,6		WO		0L			?
 *	Exception address	EXC_ADDR	0000 0110	?		RO		0L			3
 *	Instruction VA fmt	IVA_FORM	0000 0111	5		RO		0L			3
 *	Current mode		CM			0000 1001	4		RW		0L			3
 *	Interrupt enable	IER			0000 1010	4		RW		0L			3
 *	Inter ena & cur mod	IER_CM		0000 10xx	4		RW		0L			3
 *	Software inter req	SIRR		0000 1100	4		RW		0L			3
 *	Interrupt summary	ISUM		0000 1101	?		RO		?			?
 *	Hardware inter clr	HW_INT_CLR	0000 1110	4		WO		0L			?
 *	Exception summary	EXC_SUM		0000 1111	?		RO		0L			3
 *	PAL base address	PAL_BASE	0001 0000	4		RW		0L			3
 *	Ibox control		I_CTL		0001 0001	4		RW		0L			3
 *	Ibox status			I_STAT		0001 0110	4		RW		0L			3
 *	Icache flush		IC_FLUSH	0001 0011	4		W		0L			?		Pseudo
 *	Icache flush ASM	IC_FLUSH_ASM 0001 0010	4		WO		0L			?		Pseudo
 *	Clear virt-2-physmap CLR_MAP	0001 0101	4,5,6,7	WO		0L			?		Pseudo
 *	Sleep mode			SLEEP		0001 0111	4,5,6,7	WO		0L			?		Pseudo
 *	Process ctx reg 	PCTX		01xn nnnn*	4		W		0L			3
 *	Process ctx reg		PCTX		01xx xxxx	4		R		0L			3
 *	Perf counter ctrl	PCTR_CTL	0001 0100	4		RW		0L			3
 *		*When n equals 1, that process context field is selected (FPE, PPCE, ASTRR,
 *		ASTER, ASN).
 */
typedef struct
{
	u64	res_1 : 13;
	u64 tag : 35;					/* Virtual address[47:13] = ITB tag */
	u64 res_2 : 16;
} AXP_IBOX_ITB_TAG;

typedef struct
{
	u64 res_1 : 4;
	u64 _asm : 1;					/* Address space match */
	u64 gh : 2;						/* Granularity hint */
	u64 res_2 : 1;
	u64 kre : 1;					/* Kernel read/execute */
	u64 ere : 1;					/* Executive read/execute */
	u64 sre : 1;					/* Supervisor read/execute */
	u64 ure : 1;					/* User read/execute */
	u64 res_3 : 1;
	u64 pfn : 31;					/* Page frame number */
	u64 res_4 : 20;
} AXP_IBOX_ITB_PTE;

typedef struct
{
	u64	res_1 : 13;
	u64	inval_itb : 35;				/* ITB Virtual address(tag) to invalidate */
	u64	res_2 : 16;
} AXP_IBOX_ITB_IS;

typedef union
{
	AXP_PC	exc_pc;
	u64	exc_addr;
} AXP_IBOX_EXC_ADDR;


typedef struct
{
	u64	res : 3;
	u64	va : 30;
	u64	vptb : 31;
} AXP_IBOX_IVA_FORM_00;

typedef struct
{
	u64	res : 3;
	u64	va_sext_vptb : 61;
} AXP_IBOX_IVA_FORM_10;

/*
 * For the above VA_FORM format, the following three fields have overlapping
 * bits.
 *
 *	VPTB[63:43]
 *	SEXT(VA[47])
 *	VA[47:3]
 *
 * The definitions used for the IVA_FORM when VA_48 = 1 and VA_FORM_32 = 0.
 */

typedef struct
{
	u64	res_1 : 3;
	u64	va : 19;
	u64	res_2 : 8;
	u64	vptb : 34;
} AXP_IBOX_IVA_FORM_01;

typedef union
{
	AXP_IBOX_IVA_FORM_00	form00;		/* VA_48 = 0 and VA_FORM_32 = 0*/
	AXP_IBOX_IVA_FORM_10	form10;		/* VA_48 = 1 and VA_FORM_32 = 0*/
	AXP_IBOX_IVA_FORM_01	form01;		/* VA_48 = 0 and VA_FORM_32 = 1*/
} AXP_IBOX_IVA_FORM;

/*
 * Interrupt enable and current mode register
 * 	The cm field can have the following values:
 * 		00 = Kernel
 * 		01 = Executive
 * 		10 = Supervisor
 * 		11 = User
 */
typedef struct
{
	u64	res_1 : 3;
	u64	cm : 2;						/* Current mode */
	u64	res_2 : 8;
	u64	asten : 1;					/* AST interrupt enable */
	u64	sien : 15;					/* Software interrupt enable */
	u64	pcen : 2;					/* Performance counter interrupt enable */
	u64	cren : 1;					/* Correct read error interrupt enable */
	u64	slen : 1;					/* Serial line interrupt enable */
	u64	eien : 6;					/* External interrupt enable */
	u64	res_3 : 25;
} AXP_IBOX_IER_CM;

typedef struct
{
	u64	res_1 : 14;
	u64	sir : 15;					/* Software interrupt requests */
	u64	res_2 : 35;
} AXP_IBOX_SIRR;

/*
 * Interrupt Summary Register - Used to report what interrups are currently
 * pending.
 * 	The pc field can have the following values:
 * 		0 = PC0
 * 		1 = PC1
 */
typedef struct
{
	u64	res_1 : 3;
	u64	astk : 1;					/* Kernel AST interrupt */
	u64	aste : 1;					/* Executive AST interrupt */
	u64	res_2 : 4;
	u64	asts : 1;					/* Supervisor AST interrupt */
	u64	astu : 1;					/* User AST interrupt */
	u64	res_3 : 3;
	u64	si : 15;					/* Software interrupt */
	u64	pc : 2;						/* Performance counter interrupts */
	u64	cr : 1;						/* Corrected read error interrupt */
	u64	sl : 1;						/* Serial line interrupt */
	u64	ei : 6;						/* External interrupts */
	u64	res_4 : 25;
} AXP_IBOX_ISUM;

typedef struct
{
	u64	res_1 : 26;
	u64	fbtp : 1;					/* Force bad Icache fill parity */
	u64	mchk_d : 1;					/* Clear Dstream machine check */
	u64	res_2 : 1;
	u64	pc : 2;						/* CLear performance counter */
	u64	cr : 1;						/* Clear corrected read */
	u64	sl : 1;						/* Clear serial line */
	u64	res_3 : 31;
} AXP_IBOX_HW_INT_CLR;

typedef struct
{
	u64	swc : 1;					/* Software completion possible */
	u64	inv : 1;					/* Invalid operation trap */
	u64	dze : 1;					/* Divide by zero trap */
	u64	ovf : 1;					/* Floating point overflow trap */
	u64	unf : 1;					/* Floating point underflow trap */
	u64	ine : 1;					/* Floating point inexact error trap */
	u64	iov : 1;					/* Integer overflow trap */
	u64	_int : 1;					/* Ebox(1)/Fbox(0) for iov field */
	u64	reg : 5;					/* Destination/source register for trap */
	u64	bad_iva : 1;				/* Bad Istream VA */
	u64	res : 27;
	u64	pc_ovfl : 1;				/* EXC_ADDR improperly SEXT in 48-bit mode */
	u64	set_inv : 1;				/* PALcode should set FPCR[INV] */
	u64	set_dze : 1;				/* PALcode should set FPCR[DZE] */
	u64	set_ovf : 1;				/* PALcode should set FPCR[OVF] */
	u64	set_unf : 1;				/* PALcode should set FPCR[UNF] */
	u64	set_ine : 1;				/* PALcode should set FPCR[INE] */
	u64	set_iov : 1;				/* PALcode should set FPCR[IOV] */
	u64	sext_set_iov : 16;			/* Sign-extended (SEXT) of SET_IOV */
} AXP_IBOX_EXC_SUM;

typedef union
{
	struct
	{
		u64	res_1 : 15;
		u64	pal_base : 29;			/* Base physical address for PALcode */
		u64	res_2 : 20;
	} fields;
	AXP_PC	pal_base_addr;
	u64	pal_base_pc;
} AXP_IBOX_PAL_BASE;

/*
 * Ibox Control Register
 * 	The chip_id field can have the following values:
 * 		3 (0b000011) = 21264 pass 2.3
 * 		5 (0b000101) = 21264 pass 2.4
 */
typedef struct
{
	u64	spce : 1;					/* System performance counter enable */
	u64	ic_en : 2;					/* Icache set enable */
	u64	spe : 3;					/* Super page mode enable */
	u64	sde : 2;					/* PALshadow register enable */
	u64 sbe : 2;					/* Stream buffer enable */
	u64 bp_mode : 2;				/* Branch prediction mode selection */
	u64 hwe : 1;					/* Allow PAL reserved opcodes in Kernel */
	u64 sl_xmit : 1;				/* Cause SROM to advance to next bit */
	u64 sl_rcv : 1;
	u64 va_48 : 1;					/* Enable 48-bit addresses (43 otherwise) */
	u64 va_form_32 : 1;				/* Address formatting on read of IVA_FORM */
	u64 single_issue_h : 1;			/* Force instruction issue from bottom of queues */
	u64 pct0_en : 1;				/* Enable performance counter #0 */
	u64 pct1_en : 1;				/* Enable performance counter #1 */
	u64 call_pal_r23 : 1;			/* Use PALshadow register R23, instead of R27 */
	u64 mchk_en : 1;				/* Machine check enable */
	u64 tb_mb_en : 1;				/* Insert MB on TB fills (1 = multiprocessors) */
	u64 bist_fail : 1;				/* Indicates status of BiST 1=pass/0=fail */
	u64 chip_id : 6;				/* Chip revision ID */
	u64 vptb : 18;					/* Virtual Page Table Base */
	u64 sext_vptb : 16;				/* Sign extension of 'vptb' */
} AXP_IBOX_I_CTL;

#define AXP_I_CTL_BP_MODE_FALL		0x2		/* 1x, where 'x' is not relevant */
#define AXP_I_CTL_BP_MODE_DYN		0x0		/* 0x, where 'x' is relevant */
#define AXP_I_CTL_BP_MODE_LOCAL		0x1		/* Local History Prediction */
#define AXP_I_CTL_BP_MODE_CHOICE	0x0		/* Choice selected Local/Global */

#define AXP_I_CTL_SDE_ENABLE		0x2		/* bit 0 does not affect 21264 operation */

typedef struct
{
	u64 res_1 : 29;
	u64 tpe : 1;					/* Icache tag parity error */
	u64 dpe : 1;					/* Icache data parity error */
	u64 res_2 : 33;
} AXP_IBOX_I_STAT;

/*
 * Process Context Register
 * 	The 'aster' and 'astrr' fields can have the following values:
 * 		0x1 = Kernal mode
 * 		0x2 = Supervisor mode
 * 		0x4 = Executive mode
 * 		0x8 = User mode
 */
typedef struct
{
	u64	res_1 : 1;
	u64	ppce : 1;					/* Process performance counting enable */
	u64	fpe : 1;					/* Floating point enable */
	u64	res_2 : 2;
	u64 aster : 4;					/* AST enable register */
	u64 astrr : 4;
	u64 res_3 : 26;
	u64 asn : 8;					/* Address space number */
	u64 res_4 : 17;
} AXP_IBOX_PCTX;

/*
 * Performance Counter Control Register
 * 	The 'sl1' field can have the following values:
 * 		0b0000 = Counter 1 counts cycles
 * 		0b0001 = Counter 1 counts retired conditional branches
 * 		0b0010 = Counter 1 counts retired branch mispredicts
 * 		0b0011 = Counter 1 counts retired DTB single misses * 2
 * 		0b0100 = Counter 1 counts retired DTB double double misses
 * 		0b0101 = Counter 1 counts retired ITB misses
 * 		0b0110 = Counter 1 counts retired unaligned traps
 * 		0b0111 = Counter 1 counts replay traps
 * 	The 'sl0' field can have the following values:
 * 		0b0 = Counter 0 counts cycles
 * 		0b1 = Counter 0 counts retired instructions
 */
typedef struct
{
	u64 sl1 : 4;					/* SL1 input select */
	u64 sl0 : 1;					/* SL0 input select */
	u64 res_1 : 1;
	u64 pctr1 : 20;					/* Performance counter 1 */
	u64 res_2 : 2;
	u64 pctr0 : 20;					/* Performance counter 0 */
	u64 sext_pctr0 : 16;
} AXP_IBOX_PCTR_CTL;

/*
 * The following definitions are for the Mbox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	DTB tag array wri0	DTB_TAG0	0010 0000	2,6		WO		0L			?
 *	DTB tag array wri1	DTB_TAG1	1010 0000	1,5		WO		1L			?
 *	DTB PTE array wri0	DTB_PTE0	0010 0001	0,4		WO		0L			?
 *	DTB PTE array wri1	DTB_PTE1	1010 0001	3,7		WO		0L			?
 *	DTB alt proc mode	DTB_ALTMODE	0010 0110	6		WO		1L			?
 *	DTB inval all proc	DTB_IAP		1010 0010	7		WO		1L			?		Pseudo
 *	(ASM=0)
 *	DTB invalidate all	DTB_IA		1010 0011	7		WO		1L			?		Pseudo
 *	DTB inv single (arr0) DTB_IS0	0010 0100	6		WO		0L			?		Pseudo
 *	DTB inv single (arr1) DTB_IS1	1010 0100	7		WO		1L			?		Pseudo
 *	DTB addr space num 0 DTB_ASN0	0010 0101	4		WO		0L			?
 *	DTB addr space num 1 DTB_ASN1	1010 0101	7		WO		1L			?
 *	Memory mgmt status	MM_STAT		0010 0111	?		RO		0L			3
 *	Mbox control		M_CTL		0010 1000	6		WO		0L			?
 *	Dcache control		DC_CTL		0010 1001	6		WO		0L			?
 *	Dcache status		DC_STAT		0010 1010	6		RW		0L			3
 */
typedef struct
{
	u64 res_1 : 13;
	u64 va : 35;
	u64 res_2 : 16;
} AXP_MBOX_DTB_TAG;

typedef struct
{
	u64 res_1 : 1;
	u64 _for : 1;
	u64 fow : 1;
	u64 res_2 : 1;
	u64 _asm : 1;
	u64 gh : 2;
	u64 res_3 : 1;
	u64 kre : 1;
	u64 ere : 1;
	u64 sre : 1;
	u64 ure : 1;
	u64 kwe : 1;
	u64 ewe : 1;
	u64 swe : 1;
	u64 uwe : 1;
	u64 res_4 : 16;
	u64 pa : 31;
	u64 res_5 : 1;
} AXP_MBOX_DTB_PTE;

typedef struct
{
	u64 alt_mode : 2;
	u64 res : 62;
} AXP_MBOX_DTB_ALTMODE;

typedef AXP_IBOX_ITB_IS AXP_MBOX_DTB_IS;

typedef struct
{
	u64 res_1 : 24;
	u64 asn : 8;
	u64 res_2 : 32;
} AXP_MBOX_DTB_ASN;

typedef struct
{
	u64 wr : 1;
	u64 acv : 1;
	u64 _for : 1;
	u64 fow : 1;
	u64 opcodes : 6;
	u64 dc_tag_perr : 1;
	u64 res : 53;
} AXP_MBOX_MM_STAT;

typedef struct
{
	u64 res_1 : 1;
	u64 spe : 3;
	u64 res_2 : 60;
} AXP_MBOX_M_CTL;

typedef struct
{
	u64 set_en : 2;
	u64 f_hit : 1;
	u64 res_1 : 1;
	u64 f_bad_tpar : 1;
	u64 f_bad_decc : 1;
	u64 dctag_par_en : 1;
	u64 dctab_err_en : 1;
	u64 res_2 : 56;
} AXP_MBOX_DC_CTL;

typedef struct
{
	u64 tperr_p0 : 1;
	u64 tperr_p1 : 1;
	u64 ecc_err_st : 1;
	u64 ecc_eff_ld : 1;
	u64 seo : 1;
	u64 res : 59;
} AXP_MBOX_DC_STAT;

/*
 * The following definitions are for the Cbox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	Cbox data			C_DATA		0010 1011	6		RW		0L			3
 *	Cbox shift control	C_SHFT		0010 1100	6		WO		0L			?
 */
typedef struct
{
	u64 cdata : 6;
	u64 res : 58;
} AXP_CBOX_C_DATA;

typedef struct
{
	u64 c_shift : 1;
	u64 res : 63;
} AXP_CBOX_C_SHFT;

#endif /*_AXP_21264_IPR_DEFS_ */
