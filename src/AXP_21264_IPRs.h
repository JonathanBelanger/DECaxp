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
 */
#ifndef _AXP_21264_IPR_DEFS_
#define _AXP_21264_IPR_DEFS_

#include "AXP_Utility.h"

/*
 * The following definitions are for the Ebox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	Cycle counter		CC			1100 0000	5		RW		1L			1
 *	Cycle counter ctrl	CC_CTL		1100 0001	5		W0		1L			—
 *	Virtual address		VA			1100 0010	4,5,6,7	RO 		1L			1
 *	Virtual addr ctrl	VA_CTL		1100 0100	5		WO		1L			—
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
	u62 res_2 : 8;
	u64 vptb : 34;			/* Virtual Page Table Base */
} AXP_EBOX_VA_FORM_01;

typedef union
{
	AXP_EBOX_VA_FORM_00	form00;		/* VA_48 = 0 and VA_FORM_32 = 0*/
	AXP_EBOX_VA_FORM_10	form10;		/* VA_48 = 1 and VA_FORM_32 = 0*/
	AXP_EBOX_VA_FORM_01	form01;		/* VA_48 = 0 and VA_FORM_32 = 1*/
} AXP_EBOX_VA_FORM;

/*
 * The following definitions are for the Ibox IPRs
 *
 *																MT/MF		Latency
 *												Score-			Issued		for
 *									Index		Board			from Ebox	MFPR
 *	Register Name		Mnemonic	(Binary)	Bit		Access	Pipe		(Cycles)
 *	------------------	--------	--------	-------	------	-----------	--------
 *	ITB tag array write	ITB_TAG		0000 0000	6		WO		0L			—
 *	ITB PTE array write	ITB_PTE		0000 0001	4,0		WO		0L			—
 *	ITB inval all proc	ITB_IAP		0000 0010	4		WO		0L			—
 *	(ASM=0)
 *	ITB invalidate all	ITB_IA		0000 0011	4		WO		0L			—		Pseudo
 *	ITB invalid single	ITB_IS		0000 0100	4,6		WO		0L			—
 *	Exception address	EXC_ADDR	0000 0110	—		RO		0L			3
 *	Instruction VA fmt	IVA_FORM	0000 0111	5		RO		0L			3
 *	Current mode		CM			0000 1001	4		RW		0L			3
 *	Interrupt enable	IER			0000 1010	4		RW		0L			3
 *	Inter ena & cur mod	IER_CM		0000 10xx	4		RW		0L			3
 *	Software inter req	SIRR		0000 1100	4		RW		0L			3
 *	Interrupt summary	ISUM		0000 1101	—		RO		—			—
 *	Hardware inter clr	HW_INT_CLR	0000 1110	4		WO		0L			—
 *	Exception summary	EXC_SUM		0000 1111	—		RO		0L			3
 *	PAL base address	PAL_BASE	0001 0000	4		RW		0L			3
 *	Ibox control		I_CTL		0001 0001	4		RW		0L			3
 *	Ibox status			I_STAT		0001 0110	4		RW		0L			3
 *	Icache flush		IC_FLUSH	0001 0011	4		W		0L			—		Pseudo
 *	Icache flush ASM	IC_FLUSH_ASM 0001 0010	4		WO		0L			—		Pseudo
 *	Clear virt-2-physmap CLR_MAP	0001 0101	4,5,6,7	WO		0L			—		Pseudo
 *	Sleep mode			SLEEP		0001 0111	4,5,6,7	WO		0L			—		Pseudo
 *	Process ctx reg 	PCTX		01xn nnnn*	4		W		0L			3
 *	Process ctx reg		PCTX		01xx xxxx	4		R		0L			3
 *	Perf counter ctrl	PCTR_CTL	0001 0100	4		RW		0L			3
 *		*When n equals 1, that process context field is selected (FPE, PPCE, ASTRR,
 *		ASTER, ASN).
 */
typedef struct
{
	u64	res_1 : 13;
	u64 va : 35;
	u64 res_2 : 16;
} AXP_IBOX_ITB_TAG;

typedef struct
{
	u64	res_1 : 13;
	u64	inval_itb : 35;
	u64	res_2 : 16;
} AXP_IBOX_ITB_IS;

typedef struct
{
	u64 res_1 : 4;
	u64 asm : 1;
	u64 gh : 2;
	u64 res_2 : 1;
	u64 kre : 1;
	u64 ere : 1;
	u64 sre : 1;
	u64 ure : 1;
	u64 res_3 : 1;
	u64 pfn : 31;
	u64 res_4 : 20;
} AXP_IBOX_IER_CM;

typedef struct
{
	u64	pal : 1;
	u64	res : 1;
	u64	pc : 62;
} AXP_INSTRUCTION_VA;

typedef union
{
	AXP_INSTRUCTION_VA	exc_addr;
	u64	exc_pc;
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
	u62	res_2 : 8;
	u64	vptb : 34;
} AXP_IBOX_IVA_FORM_01;

typedef union
{
	AXP_IBOX_IVA_FORM_00	form00;		/* VA_48 = 0 and VA_FORM_32 = 0*/
	AXP_IBOX_IVA_FORM_10	form10;		/* VA_48 = 1 and VA_FORM_32 = 0*/
	AXP_IBOX_IVA_FORM_01	form01;		/* VA_48 = 0 and VA_FORM_32 = 1*/
} AXP_IBOX_IVA_FORM;

typedef struct
{
	u64	res_1 : 3;
	u64	cm : 2;
	u64	res_2 : 8;
	u64	asten : 1;
	u64	sien : 15;
	u64	pcen : 2;
	u64	cren : 1;
	u64	slen : 1;
	u64	eien : 6;
	u64	res_3 : 25;
} AXP_IBOX_IER_CM;

typedef struct
{
	u64	res_1 : 14;
	u64	sir : 15;
	u64	res_2 : 35;
} AXP_IBOX_SIRR;

typedef struct
{
	u64	res_1 : 3;
	u64	astk : 1;
	u64	aste : 1;
	u64	res_2 : 4;
	u64	asts : 1;
	u64	astu : 1;
	u64	res_3 : 3;
	u64	si : 15;
	u64	pc : 2;
	u64	cr : 1;
	u64	sl : 1;
	u64	ei : 6;
	u64	res_4 : 25;
} AXP_IBOX_ISUM;

typedef struct
{
	u64	res_1 : 26;
	u64	fbtp : 1;
	u64	mchk_d : 1;
	u64	res_2 : 1;
	u64	pc : 2;
	u64	cr : 1;
	u64	sl : 1;
	u64	res_3 : 31;
} AXP_IBOX_HW_INT_CLR;

typedef struct
{
	u64	swc : 1;
	u64	inv : 1;
	u64	dze : 1;
	u64	fov : 1;
	u64	unf : 1;
	u64	ine : 1;
	u64	iov : 1;
	u64	int : 1;
	u64	reg : 5;
	u64	bad_iva : 1;
	u64	res : 27;
	u64	pc_ovfl : 1;
	u64	set_inv : 1;
	u64	set_dze : 1;
	u64	set_ovf : 1;
	u64	set_unf : 1;
	u64	set_ine : 1;
	u64	set_iov : 1;
	u64	sext_set_iov : 16;
} AXP_IBOX_EXC_SUM;

typedef union
{
	struct
	{
		u64	res_1 : 15;
		u64	pal_base : 29;
		u64	res_2 : 20;
	} fields;
	AXP_INSTRUCTION_VA	pal_base_addr;
	u64	pal_base_pc;
} AXP_IBOX_PAL_BASE;

typedef struct
{
	u64	spce : 1;
	u64	ic_en : 2;
	u64	spe : 3;
	u64	sde : 2;
	u64 sbe : 2;
	u64 bp_mode : 2;
	u64 hwe : 1;
	u64 sl_xmit : 1;
	u64 sl_rcv : 1;
	u64 va_48 : 1;
	u64 va_form_32 : 1;
	u64 single_issue_h : 1;
	u64 pct0_en : 1;
	u64 pct1_en : 1;
	u64 call_pal_r23 : 1;
	u64 mchk_en : 1;
	u64 tb_mb_en : 1;
	u64 bist_fail : 1;
	u64 chip_id : 6;
	u64 vptb 18;
	u64 sext_vptb : 16;
} AXP_IBOX_I_CTL;

typedef struct
{
	u64 res_1 : 29;
	u64 tpe : 1;
	u64 dpe : 1;
	u64 res_2 : 33;
} AXP_IBOX_I_STAT;

typedef struct
{
	u64	res_1 : 1;
	u64	ppce : 1;
	u64	fpe : 1;
	u64	res_2 : 2;
	u64 aster : 4;
	u64 astrr : 4;
	u64 res_3 : 26;
	u64 asn : 8;
	u64 res_4 : 17;
} AXP_IBOX_PCTX;

typedef struct
{
	u64 sl1 : 4;
	u64 sl0 : 1;
	u64 res_1 : 1
	u64 pctr1 : 20;
	u64 res_2 : 2;
	u64 pctr0 : 20;
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
 *	DTB tag array wri0	DTB_TAG0	0010 0000	2,6		WO		0L			—
 *	DTB tag array wri1	DTB_TAG1	1010 0000	1,5		WO		1L			—
 *	DTB PTE array wri0	DTB_PTE0	0010 0001	0,4		WO		0L			—
 *	DTB PTE array wri1	DTB_PTE1	1010 0001	3,7		WO		0L			—
 *	DTB alt proc mode	DTB_ALTMODE	0010 0110	6		WO		1L			—
 *	DTB inval all proc	DTB_IAP		1010 0010	7		WO		1L			—		Pseudo
 *	(ASM=0)
 *	DTB invalidate all	DTB_IA		1010 0011	7		WO		1L			—		Pseudo
 *	DTB inv single (arr0) DTB_IS0	0010 0100	6		WO		0L			—		Pseudo
 *	DTB inv single (arr1) DTB_IS1	1010 0100	7		WO		1L			—		Pseudo
 *	DTB addr space num 0 DTB_ASN0	0010 0101	4		WO		0L			—
 *	DTB addr space num 1 DTB_ASN1	1010 0101	7		WO		1L			—
 *	Memory mgmt status	MM_STAT		0010 0111	—		RO		0L			3
 *	Mbox control		M_CTL		0010 1000	6		WO		0L			—
 *	Dcache control		DC_CTL		0010 1001	6		WO		0L			—
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
	u64 for : 1;
	u64 fow : 1;
	u64 res_2 : 1;
	u64 asm : 1;
	u64 gh : 2;
	u64 res_3 : 1;
	u64 kre : 1;
	u64 ere : 1;
	u64 sre : 1;
	u64 ure : 1;
	u64 rwe : 1;
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
	u64 for : 1;
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
 *	Cbox shift control	C_SHFT		0010 1100	6		WO		0L			Ò
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

/*
 * The following are for IPRs defined in the Architectural Reference Manual
 *
 *											Input		Output		Context
 *	Register Name		Mnemonic	Access†	R16			R0			Switched
 *	-------------------	--------	-------	---------	---------	--------
 *	Address Space Num	ASN			R		—			Number		Yes
 *	AST Enable			ASTEN		R/W		Mask		Mask		Yes
 *	AST Summary Reg		ASTSR		R/W		Mask		Mask		Yes
 *	Data Align Trap Fix	DATFX		W		Value		—			Yes
 *	Executive Stack Ptr	ESP			R/W		Address		Address		Yes
 *	Floating-point Ena	FEN			R/W		Value		Value		Yes
 *	Interproc Int. Req	IPIR		W		Number		—			No
 *	Interrupt Prio Lvl	IPL			R/W		Value		Value		No
 *	Kernel Stack Ptr	KSP			None	—			—			Yes
 *	Machine Chk Err Sum	MCES		R/W		Value		Value		No
 *	*Perf Monitoring	PERFMON		W		IMP			IMP			No
 *	Priv Ctx Blk Base	PCBB		R		—			Address		No
 *	Proc Base Register	PRBR		R/W		Value		Value		No
 *	Page Table Base Reg	PTBR		R		—			Frame		Yes
 *	Sys Ctrl Blk Base	SCBB		R/W		Frame		Frame		No
 *	S/W Int. Req Reg	SIRR		W		Level		—			No
 *	S/W Int. Summ Reg	SISR		R		—			Mask		No
 *	Supervi Stack Ptr	SSP			R/W		Address		Address		Yes
 *	Sys Page Tbl Base	SYSPTBR		R/W		Value		Value		Yes
 *	TB Check			TBCHK		R		Number		Status		No
 *	TB Invalid. All		TBIA		W		—			—			No	Pseudo
 *	TB Inv. All Proc	TBIAP		W		—			—			No	Pseudo
 *	TB Invalid. Single	TBIS		W		Address		—			No	Pseudo
 *	TB Inv. Single Data	TBISD		W		Address		—			No	Pseudo
 *	TB Inv. Singl Instr	TBISI		W		Address		—			No	Pseudo
 *	User Stack Pointer	USP			R/W		Address		Address		Yes
 *	Virt Addr Boundary	VIRBND		R/W		Address		Address		Yes
 *	Virt Page Tbl Base	VPTB		R/W		Address		Address		No
 *	Who-Am-I			WHAMI		R		—			Number		No
 *	† Access symbols are defined in Table 13–2.
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

typedef u64 AXP_BASE_ESP;

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

typedef struct
{
	u64	mck : 1;
	u64	sce : 1;
	u64	pce : 1;
	u64 dpc : 1;
	u64 dsc : 1;
	u64 res : 27;
	u63 imp : 32;
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
} AXP_BASE_SISR;

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

typedef u64 AXP_BASE_SSP;

typedef struct
{
	u32 pfn;
	u32 res;
} AXP_BASE_SYSPTBR;

typedef struct
{
	u64 prs : 1;
	u64 res_1 : 62;
	u64 imp : 1
} AXP_BASE_TBCHK;

typedef u64 AXP_BASE_TBCHK_R16;

typedef u64 AXP_BASE_USP;

typedef u64 AXP_BASE_VIRBND;

typedef u64 AXP_BASE_VPTB;

typedef u64 AXP_BASE_WHAMI;

#endif /*_AXP_21264_IPR_DEFS_ */