/*
 * Copyright (C) Jonathan D. Belanger 2017-2018.
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
 *
 *	V01.003		14-Jan-2018	Jonathan D. Belanger
 *	Added some macros for reading and writing from and to the IPRs by the
 *	HW_MFPR and HW_MTPR instructions, respectively.  These macros will take
 *	into account what portions of an IPR may be read-only (RO) or write-only
 *	(WO).
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
#define AXP_EBOX_READ_CC(dest, cpu)			\
		*((u64 *) &dest) = *((u64 *) &cpu->cc)
#define AXP_EBOX_WRITE_CC(src, cpu)	\
		cpu->cc.offset = *((u32 *) &src)

typedef struct
{
	u32	res_1 : 4;
	u32 counter : 28;		/* CC[31:4] in AXP_EBOX_CC */
	u32	cc_ena : 1;			/* Counter Enable */
	u32 res_2 : 31;
} AXP_EBOX_CC_CTL;			/* Cycle Counter Control Register */
#define AXP_EBOX_WRITE_CC_CTL(src, cpu)		\
		*((u64 *) &(cpu->ccCtl)) = *((u64 *) &src) & 0x00000001fffffff0ll

typedef u64 AXP_EBOX_VA;	/* Virtual Address Register */
#define AXP_EBOX_READ_VA(dest, cpu)			\
		*((u64 *) &dest) = cpu->va

typedef struct
{
	u64	b_endian : 1;		/* Big Endian Mode */
	u64	va_48 : 1;			/* 0 = 43 bit, 1 = 48 bit addressing */
	u64	va_form_32 : 1;		/* Controls interpretation of VA_FORM register */
	u64 res : 27;
	u64 vptb : 34;			/* Virtual Page Table Base */
} AXP_EBOX_VA_CTL;
#define AXP_EBOX_WRITE_VA_CTL(src, cpu)		\
		*((u64 *) &(cpu->vaCtl)) = *((u64 *) &src) & 0xffffffffc0000007ll

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
#define GET_VA(reg)			((reg) & 0x00003ffffffffff8ll) >> 3
#define SAVE_VA(reg, va)	(((reg) & 0xffffc00000000000ll) | ((va) << 3))
#define GET_SEXT(reg)		((reg) & 0x0000800000000000ll) >> 47
#define SAVE_SEXT(reg, sext) (((reg) & 0xffff7ffffffffff8ll) | ((sext) << 47))
#define GET_VPTB(reg)		((reg) & 0xffffff8000000000ll) >> 43
#define SAVE_VPTB(reg, vptb) (((reg)&0x0000007ffffffff8ll) | ((vptb) << 43))

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
	u64					form;
} AXP_EBOX_VA_FORM;
#define AXP_EBOX_READ_VA_FORM(dest, cpu)	\
		*((u64 *) &dest) = (cpu->vaForm.form & (0xffffffffc03ffff8ll |	\
				((cpu->vaCtl.va_form_32 == 0) ? 0x000000003fc00000ll : 0)))

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
#define AXP_FBOX_READ_FPCR(dest, cpu)		\
		*((u64 *) &dest) = (*(u64 *) &(cpu->fpcr)) & 0xffff000000000000ll
#define AXP_FBOX_WRITE_FPCR(src, cpu)		\
		*((u64 *) &cpu->fpcr) = (*(u64 *) &src) & 0xffff000000000000ll

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
#define AXP_IBOX_WRITE_ITB_TAG(src, cpu)	\
		*((u64 *) &cpu->itbTag) = *((u64 *) &src) & 0x0000ffffffffe000ll

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
#define AXP_IBOX_WRITE_ITB_PTE(src, cpu)	\
		*((u64 *) &cpu->itbPte) = *((u64 *) &src) & 0x00000fffffffef70ll

typedef struct
{
	u64	res_1 : 13;
	u64	inval_itb : 35;				/* ITB Virtual address(tag) to invalidate */
	u64	res_2 : 16;
} AXP_IBOX_ITB_IS;
#define AXP_IBOX_WRITE_ITB_IS(src, cpu)		\
		*((u64 *) &cpu->itbIs) = *((u64 *) &src) & 0x0000ffffffffe000ll

typedef union
{
	AXP_PC	exc_pc;
	u64	exc_addr;
} AXP_IBOX_EXC_ADDR;
#define AXP_IBOX_READ_EXC_ADDR(dest, cpu)	\
		*((u64 *) &dest) = (cpu)->excAddr.exc_addr

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
	u64						form;
} AXP_IBOX_IVA_FORM;
#define AXP_IBOX_READ_IVA_FORM(dest, cpu)	\
	*((u64 *) &dest) = (cpu->ivaForm.form & (0xffffffffc03ffff8ll | \
			((cpu->vaCtl.va_form_32 == 0) ? 0x000000003fc00000ll : 0)))

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
#define AXP_IBOX_READ_CM(dest, cpu)			\
		*((u64 *) &dest) = *((u64 *) &cpu->ierCm) & 0x0000000000000018ll
#define AXP_IBOX_READ_IER(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->ierCm) & 0x0000007ffffe0000ll
#define AXP_IBOX_READ_IER_CM(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->ierCm) & 0x0000007ffffe0018ll
#define AXP_IBOX_WRITE_CM(src, cpu)			\
		*((u64 *) &cpu->ierCm) = 			\
			(*((u64 *) &cpu->ierCm) & 0x0000007ffffe0000ll) |	\
			(*((u64 *) &src) & 0x0000000000000018ll)
#define AXP_IBOX_WRITE_IER(src, cpu)		\
		*((u64 *) &cpu->ierCm) = 			\
			(*((u64 *) &cpu->ierCm) & 0x0000000000000018ll) |	\
			(*((u64 *) &src) & 0x0000007ffffe0000ll)
#define AXP_IBOX_WRITE_IER_CM(src, cpu)		\
		*((u64 *) &cpu->ierCm) = *((u64 *) &src) & 0x0000007ffffe0018ll

typedef struct
{
	u64	res_1 : 14;
	u64	sir : 15;					/* Software interrupt requests */
	u64	res_2 : 35;
} AXP_IBOX_SIRR;
#define AXP_IBOX_READ_SIRR(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->sirr) & 0x000000001fffc000ll
#define AXP_IBOX_WRITE_SIRR(src, cpu)		\
		*((u64 *) &cpu->sirr) = *((u64 *) &src) & 0x000000001fffc000ll

/*
 * Interrupt Summary Register - Used to report what interrupts are currently
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
#define AXP_IBOX_READ_ISUM(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->iSum) & 0x0000007fffffc618ll

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
#define AXP_IBOX_WRITE_HW_INT_CLR(src, cpu)	\
		*((u64 *) &cpu->hwIntClr) = *((u64 *) &src) & 0x00000001f4000000ll

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
#define AXP_IBOX_READ_EXC_SUM(dest, cpu)	\
		*((u64 *) &dest) = *((u64 *) &cpu->excSum) & 0xfffffc0000003fffll

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
#define AXP_IBOX_READ_PAL_BASE(dest, cpu)	\
		*((u64 *) &dest) = cpu->palBase.pal_base_pc
#define AXP_IBOX_WRITE_PAL_BASE(src, cpu)	\
		cpu->palBase.pal_base_pc = *((u64 *) &src) & 0x00000fffffff8000ll

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
#define AXP_IBOX_READ_I_CTL(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->iCtl) & 0xffffffffffffdfffll
#define AXP_IBOX_WRITE_I_CTL(src, cpu)		\
		*((u64 *) &cpu->iCtl) = *((u64 *) &src) & 0xffffffffc07fbfffll

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
#define AXP_IBOX_READ_I_STAT(dest, cpu)		\
	*((u64 *) &dest) = *((u64 *) &cpu->iStat) & 0x00000000000060000000ll
#define AXP_IBOX_WRITE_I_STAT(src, cpu)		\
	*((u64 *) &cpu->iStat) = *((u64 *) &src) & 0x00000000000060000000ll

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
#define AXP_IBOX_READ_PCTX(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->pCtx) & 0x00007f8000001fe6ll
/* NOTE: No write macro because writing to individual fields is easier */

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
#define AXP_IBOX_READ_PCTR_CTL(dest, cpu)	\
		*((u64 *) &dest) = *((u64 *) &cpu->pCtrCtl) & 0xfffffffff7ffffdfll
#define AXP_IBOX_WRITE_PCTR_CTL(src, cpu)	\
		*((u64 *) &cpu->pCtrCtl) = *((u64 *) &src) & 0xfffffffff7ffffdfll

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
#define AXP_MBOX_WRITE_DTB_TAG0(src, cpu)	\
		*((u64 *) &cpu->dtbTag0) = *((u64 *) &src) & 0x0000ffffffffe000ll
#define AXP_MBOX_WRITE_DTB_TAG1(src, cpu)	\
		*((u64 *) &cpu->dtbTag1) = *((u64 *) &src) & 0x0000ffffffffe000ll

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
#define AXP_MBOX_WRITE_DTB_PTE0(src, cpu)	\
		*((u64 *) &cpu->dtbPte0) = *((u64 *) &src) & 0x7fffffff0000ff76ll
#define AXP_MBOX_WRITE_DTB_PTE1(src, cpu)	\
		*((u64 *) &cpu->dtbPte1) = *((u64 *) &src) & 0x7fffffff0000ff76ll

typedef struct
{
	u64 alt_mode : 2;
	u64 res : 62;
} AXP_MBOX_DTB_ALTMODE;
#define AXP_MBOX_WRITE_DTB_ALTMODE(src, cpu) \
		*((u64 *) &cpu->dtbAltMode) = *((u64 *) &src) & 0x0000000000000003ll

#define AXP_MBOX_ALTMODE_KERNEL	0
#define AXP_MBOX_ALTMODE_EXEC	1
#define AXP_MBOX_ALTMODE_SUPER	2
#define AXP_MBOX_ALTMODE_USER	3

typedef AXP_IBOX_ITB_IS AXP_MBOX_DTB_IS;
#define AXP_MBOX_WRITE_DTB_IS0(src, cpu)	\
		*((u64 *) &cpu->dtbIs0) = *((u64 *) &src) & 0x0000ffffffffe000ll
#define AXP_MBOX_WRITE_DTB_IS1(src, cpu)	\
		*((u64 *) &cpu->dtbIs1) = *((u64 *) &src) & 0x0000ffffffffe000ll

typedef struct
{
	u64 res_1 : 24;
	u64 asn : 8;
	u64 res_2 : 32;
} AXP_MBOX_DTB_ASN;
#define AXP_MBOX_WRITE_DTB_ASN0(src, cpu)	\
		*((u64 *) &cpu->dtbAsn0) = *((u64 *) &src) & 0x00000000ff000000ll
#define AXP_MBOX_WRITE_DTB_ASN1(src, cpu)	\
		*((u64 *) &cpu->dtbAsn1) = *((u64 *) &src) & 0x00000000ff000000ll

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
#define AXP_MBOX_READ_MM_STAT(dest, cpu)	\
		*((u64 *) &dest) = *((u64 *) &cpu->mmStat) & 0x00000000000007ffll

typedef struct
{
	u64 res_1 : 1;
	u64 spe : 3;
	u64 res_2 : 60;
} AXP_MBOX_M_CTL;
#define AXP_MBOX_WRITE_M_CTL(src, cpu)		\
		*((u64 *) &cpu->mCtl) = *((u64 *) &src) & 0x000000000000000ell

typedef struct
{
	u64 set_en : 2;
	u64 f_hit : 1;
	u64 res_1 : 1;
	u64 f_bad_tpar : 1;
	u64 f_bad_decc : 1;
	u64 dctag_par_en : 1;
	u64 dcdat_err_en : 1;
	u64 res_2 : 56;
} AXP_MBOX_DC_CTL;
#define AXP_MBOX_WRITE_DC_CTL(src, cpu)		\
		*((u64 *) &cpu->dcCtl) = *((u64 *) &src) & 0x00000000000000f7ll

typedef struct
{
	u64 tperr_p0 : 1;
	u64 tperr_p1 : 1;
	u64 ecc_err_st : 1;
	u64 ecc_err_ld : 1;
	u64 seo : 1;
	u64 res : 59;
} AXP_MBOX_DC_STAT;
#define AXP_MBOX_READ_DC_STAT(dest, cpu)	\
		*((u64 *) &dest) = *((u64 *) &cpu->dcStat) & 0x000000000000001fll
#define AXP_MBOX_WRITE_DC_STAT(src, cpu)	\
		*((u64 *) &cpu->dcStat) = *((u64 *) &src) & 0x000000000000001fll

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
#define AXP_CBOX_READ_C_DATA(dest, cpu)		\
		*((u64 *) &dest) = *((u64 *) &cpu->cData) & 0x000000000000003fll
#define AXP_CBOX_WRITE_C_DATA(src, cpu)		\
		*((u64 *) &cpu->cData) = *((u64 *) &src) & 0x000000000000003fll

typedef struct
{
	u64 c_shift : 1;
	u64 res : 63;
} AXP_CBOX_C_SHFT;
#define AXP_CBOX_WRITE_C_SHFT(src, cpu)		\
		*((u64 *) &cpu->cShft) = *((u64 *) &src) & 0x0000000000000001ll

/*
 * HRM Table 5-25 Cbox Read IPR Fields Description
 *
 * These IPRs are read via the C_DATA IPR.
 *
 * Name					Description
 * -----------------	-------------------------------------------------------
 * C_SYNDROME_1[7:0]	Syndrome for upper QW in OW of victim that was scrubbed.
 * C_SYNDROME_0[7:0]	Syndrome for lower QW in OW of victim that was scrubbed.
 * C_STAT[4:0]			Bits	Error Status
 *						-----	-----------------------------------------------
 *						00000	Either no error, or error on a speculative
 *								load, or a Bcache victim read due to a
 *								Dcache/Bcache miss
 *						00001	BC_PERR (Bcache tag parity error)
 *						00010	DC_PERR (duplicate tag parity error)
 *						00011	DSTREAM_MEM_ERR
 *						00100	DSTREAM_BC_ERR
 *						00101	DSTREAM_DC_ERR
 *						0011x	PROBE_BC_ERR
 *						01000	Reserved
 *						01001	Reserved
 *						01010	Reserved
 *						01011	ISTREAM_MEM_ERR
 *						01100	ISTREAM_BC_ERR
 *						01101	Reserved
 *						1xxxx	DOUBLE_BIT_ERROR
 * C_STS[3:0]			If C_STAT equals xxx_MEM_ERR or xxx_BC_ERR, then C_STS
 *						contains the status of the block as follows; otherwise,
 *						the value of C_STS is x:
 *						Bit Value	Status of Block
 *						---------	---------------
 *						7:4			Reserved
 *						3			Parity
 *						2			Valid
 *						1			Dirty
 *						0			Shared
 * C_ADDR[6:42]			Address of last reported ECC or parity error. If C_STAT
 *						value is DSTREAM_DC_ERR, only bits 6:19 are valid.
 */
typedef struct
{
	u8	c_syndrome_1;
	u8	c_syndrome_2;
	u32	c_stat : 5;
	u32	c_sts : 4;
	u64	c_addr;
} AXP_CBOX_READ_IPR;

/*
 * C_STAT values
 */
#define AXP_C_STAT_NOERR			0	/* Either no error, or speculative error */
#define AXP_C_STAT_BC_PERR			1	/* Bcache Parity Error */
#define AXP_C_STAT_DC_PERR			2	/* Dcache Parity Error */
#define AXP_C_STAT_DSTREAM_MEM_ERR	3	/* Dstream Memory Error */
#define AXP_C_STAT_DSTREAM_BC_ERR	4	/* Dstream Bcache Error */
#define AXP_C_STAT_DSTREAM_DC_ERR	5	/* Dstream Dcache Error */
#define AXP_C_STAT_PROBE_BC_ERR1	6	/* Probe Bcache Error 1 */
#define AXP_C_STAT_PROBE_BC_ERR2	7	/* Probe Bcache Error 2 */
#define AXP_C_STAT_ISTREAM_MEM_ERR	11	/* Istream Memory Error */
#define AXP_C_STAT_ISTREAM_BC_ERR	12	/* Istream Bcache Error */
#define AXP_C_STAT_DOUBLE_BIT_ERROR	16	/* Double Bit Error */

/*
 * C_STS values
 */
#define AXP_C_STS_PARITY	3
#define AXP_C_STS_VALID		2
#define AXP_C_STS_DIRTY		1
#define AXP_C_STS_SHARED	0

/*
 * IPR Index values.
 */
#define AXP_IPR_ITB_TAG							0x00
#define AXP_IPR_ITB_PTE							0x01
#define AXP_IPR_ITB_IAP							0x02
#define AXP_IPR_ITB_IA							0x03
#define AXP_IPR_ITB_IS							0x04
#define AXP_IPR_EXC_ADDR						0x06
#define AXP_IPR_IVA_FORM						0x07
#define AXP_IPR_CM								0x09
#define AXP_IPR_IER								0x0A
#define AXP_IPR_IER_CM							0x0B
#define AXP_IPR_SIRR							0x0C
#define AXP_IPR_ISUM							0x0D
#define AXP_IPR_HW_INT_CLR						0x0E
#define AXP_IPR_EXC_SUM							0x0F
#define AXP_IPR_PAL_BASE						0x10
#define AXP_IPR_I_CTL							0x11
#define AXP_IPR_IC_FLUSH_ASM					0x12
#define AXP_IPR_IC_FLUSH						0x13
#define AXP_IPR_PCTR_CTL						0x14
#define AXP_IPR_CLR_MAP							0x15
#define AXP_IPR_I_STAT							0x16
#define AXP_IPR_SLEEP							0x17

#define AXP_IPR_DTB_TAG0						0x20
#define AXP_IPR_DTB_PTE0						0x21
#define AXP_IPR_DTB_IS0							0x24
#define AXP_IPR_DTB_ASN0						0x25
#define AXP_IPR_DTB_ALTMODE						0x26
#define AXP_IPR_MM_STAT							0x27
#define AXP_IPR_M_CTL							0x28
#define AXP_IPR_DC_CTL							0x29
#define AXP_IPR_DC_STAT							0x2A

#define AXP_IPR_C_DATA							0x2B
#define AXP_IPR_C_SHFT							0x2C

#define AXP_IPR_PCXT0							0x40
#define AXP_IPR_PCXT0_ASN						0x41
#define AXP_IPR_PCXT0_ASTER						0x42
#define AXP_IPR_PCXT0_ASTER_ASN					0x43
#define AXP_IPR_PCXT0_ASTRR						0x44
#define AXP_IPR_PCXT0_ASTRR_ASN					0x45
#define AXP_IPR_PCXT0_ASTRR_ASTER				0x46
#define AXP_IPR_PCXT0_ASTRR_ASTER_ASN			0x47
#define AXP_IPR_PCXT0_PPCE						0x48
#define AXP_IPR_PCXT0_PPCE_ASN					0x49
#define AXP_IPR_PCXT0_PPCE_ASTER				0x4A
#define AXP_IPR_PCXT0_PPCE_ASTER_ASN			0x4B
#define AXP_IPR_PCXT0_PPCE_ASTRR				0x4C
#define AXP_IPR_PCXT0_PPCE_ASTRR_ASN			0x4D
#define AXP_IPR_PCXT0_PPCE_ASTRR_ASTER			0x4E
#define AXP_IPR_PCXT0_PPCE_ASTRR_ASTER_ASN		0x4F
#define AXP_IPR_PCXT0_FPE						0x50
#define AXP_IPR_PCXT0_FPE_ASN					0x51
#define AXP_IPR_PCXT0_FPE_ASTER					0x52
#define AXP_IPR_PCXT0_FPE_ASTER_ASN				0x53
#define AXP_IPR_PCXT0_FPE_ASTRR					0x54
#define AXP_IPR_PCXT0_FPE_ASTRR_ASN				0x55
#define AXP_IPR_PCXT0_FPE_ASTRR_ASTER			0x56
#define AXP_IPR_PCXT0_FPE_ASTRR_ASTER_ASN		0x57
#define AXP_IPR_PCXT0_FPE_PPCE					0x58
#define AXP_IPR_PCXT0_FPE_PPCE_ASN				0x59
#define AXP_IPR_PCXT0_FPE_PPCE_ASTER			0x5A
#define AXP_IPR_PCXT0_FPE_PPCE_ASTER_ASN		0x5B
#define AXP_IPR_PCXT0_FPE_PPCE_ASTRR			0x5C
#define AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASN		0x5D
#define AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER		0x5E
#define AXP_IPR_PCXT0_FPE_PPCE_ASTRR_ASTER_ASN	0x5F
#define AXP_IPR_PCXT1							0x60
#define AXP_IPR_PCXT1_ASN						0x61
#define AXP_IPR_PCXT1_ASTER						0x62
#define AXP_IPR_PCXT1_ASTER_ASN					0x63
#define AXP_IPR_PCXT1_ASTRR						0x64
#define AXP_IPR_PCXT1_ASTRR_ASN					0x65
#define AXP_IPR_PCXT1_ASTRR_ASTER				0x66
#define AXP_IPR_PCXT1_ASTRR_ASTER_ASN			0x67
#define AXP_IPR_PCXT1_PPCE						0x68
#define AXP_IPR_PCXT1_PPCE_ASN					0x69
#define AXP_IPR_PCXT1_PPCE_ASTER				0x6A
#define AXP_IPR_PCXT1_PPCE_ASTER_ASN			0x6B
#define AXP_IPR_PCXT1_PPCE_ASTRR				0x6C
#define AXP_IPR_PCXT1_PPCE_ASTRR_ASN			0x6D
#define AXP_IPR_PCXT1_PPCE_ASTRR_ASTER			0x6E
#define AXP_IPR_PCXT1_PPCE_ASTRR_ASTER_ASN		0x6F
#define AXP_IPR_PCXT1_FPE						0x70
#define AXP_IPR_PCXT1_FPE_ASN					0x71
#define AXP_IPR_PCXT1_FPE_ASTER					0x72
#define AXP_IPR_PCXT1_FPE_ASTER_ASN				0x73
#define AXP_IPR_PCXT1_FPE_ASTRR					0x74
#define AXP_IPR_PCXT1_FPE_ASTRR_ASN				0x75
#define AXP_IPR_PCXT1_FPE_ASTRR_ASTER			0x76
#define AXP_IPR_PCXT1_FPE_ASTRR_ASTER_ASN		0x77
#define AXP_IPR_PCXT1_FPE_PPCE					0x78
#define AXP_IPR_PCXT1_FPE_PPCE_ASN				0x79
#define AXP_IPR_PCXT1_FPE_PPCE_ASTER			0x7A
#define AXP_IPR_PCXT1_FPE_PPCE_ASTER_ASN		0x7B
#define AXP_IPR_PCXT1_FPE_PPCE_ASTRR			0x7C
#define AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASN		0x7D
#define AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER		0x7E
#define AXP_IPR_PCXT1_FPE_PPCE_ASTRR_ASTER_ASN	0x7F

#define AXP_IPR_DTB_TAG1						0xA0
#define AXP_IPR_DTB_PTE1						0xA1
#define AXP_IPR_DTB_IAP							0xA2
#define AXP_IPR_DTB_IA							0xA3
#define AXP_IPR_DTB_IS1							0xA4
#define AXP_IPR_DTB_ASN1						0xA5

#define AXP_IPR_CC								0xC0
#define AXP_IPR_CC_CTL							0xC1
#define AXP_IPR_VA								0xC2
#define AXP_IPR_VA_FORM							0xC3
#define AXP_IPR_VA_CTL							0xC4

/*
 * Process Context components.
 */
#define AXP_IPR_PCTX_ASN						0x01
#define AXP_IPR_PCTX_ASTER						0x02
#define AXP_IPR_PCTX_ASTRR						0x04
#define AXP_IPR_PCTX_PPCE						0x08
#define AXP_IPR_PCTX_FPE						0x10

#endif /*_AXP_21264_IPR_DEFS_ */
