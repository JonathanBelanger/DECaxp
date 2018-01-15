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
 *	This source file contains the functions needed to help the Ibox decode
 *	instructions more easily.
 *
 * Revision History:
 *
 *	V01.000		22-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ibox_InstructionInfo.h"

/*
 * The following module specific structure and variable contains a list of the
 * instructions, operation types, and register mappings that are used to assist
 * in decoding the Alpha AXP instructions.  The opcode is the index into this
 * array.
 */
struct instructDecode
{
	AXP_INS_TYPE	format;
	AXP_OPER_TYPE	type;
	AXP_REG_DECODE	registers;
	u16				whichQ;
	AXP_PIPELINE	pipeline;
};

static struct instructDecode insDecode[] =
{
/*	Format	Type	Registers   												Opcode	Mnemonic	Description 				*/
	{Pcd,	Branch,	{ .raw = 0}, AXP_IQ, EboxL0},								/* 00	CALL_PAL	Trap to PALcode				*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 01				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 02				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 03				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 04				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 05				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 06				Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE, PipelineNone},						/* 07				Reserved for Digital		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1U0U1},	/* 08	LDA			Load address				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1U0U1},	/* 09	LDAH		Load address high			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 0A	LDBU		Load zero-extended byte		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 0B	LDQ_U		Load unaligned quadword		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 0C	LDWU		Load zero-extended word		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 0D	STW			Store word					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 0E	STB			Store byte					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 0F	STQ_U		Store unaligned quadword	*/
	{Opr,	Other,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1U0U1},/*10 ADDL	Add longword				*/
	{Opr,	Other,	{ .raw = AXP_OPCODE_11}, AXP_IQ, EboxL0L1U0U1},				/* 11	AND			Logical product				*/
	{Opr,	Logic,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxU0U1},/*12	MSKBL	Mask byte low				*/
	{Opr,	Oper,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxU1},/*13	MULL		Multiply longword			*/
	{FP,	Arith,	{ .raw = AXP_OPCODE_14}, AXP_COND, EboxL0L1},				/* 14	ITOFS		Int to float move, S_float	*/
	{FP,	Other,	{ .raw = AXP_OPCODE_15}, AXP_FQ, FboxOther},				/* 15	ADDF		Add F_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_16}, AXP_FQ, FboxOther},				/* 16	ADDS		Add S_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_17}, AXP_FQ, EboxL0L1U0U1},				/* 17	CVTLQ		Convert longword to quad	*/
	{Mfc,	Other,	{ .raw = AXP_OPCODE_18}, AXP_IQ, PipelineNone},				/* 18	TRAPB		Trap barrier				*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA}, AXP_IQ, EboxL0L1},					/* 19	HW_MFPR		Reserved for PALcode		*/
	{Mbr,	Branch,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0},			/* 1A	JMP			Jump						*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 1B	HW_LD		Reserved for PALcode		*/
	{Cond,	Arith,	{ .raw = AXP_OPCODE_1C}, AXP_COND, EboxL0L1U0U1},			/* 1C	SEXTB		Sign extend byte			*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RB}, AXP_IQ, EboxL0L1},					/* 1D	HW_MTPR		Reserved for PALcode		*/
	{PAL,	Branch,	{ .raw = AXP_SRC1_RB}, AXP_IQ, EboxL0},						/* 1E	HW_RET		Reserved for PALcode		*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 1F	HW_ST		Reserved for PALcode		*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ, FboxOther},		/* 20 	LDF			Load F_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ, FboxOther},		/* 21	LDG			Load G_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ, FboxOther},		/* 22	LDS			Load S_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ, FboxOther},		/* 23	LDT			Load T_floating				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ, FboxOther},		/* 24	STF			Store F_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ, FboxOther},		/* 25	STG			Store G_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ, FboxOther},		/* 26	STS			Store S_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ, FboxOther},		/* 27	STT			Store T_floating			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 28	LDL			Load sign-extended long		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 29	LDQ			Load quadword				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 2A	LDL_L		Load sign-extend long lock	*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ, EboxL0L1},		/* 2B	LDQ_L		Load quadword locked		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 2C	STL			Store longword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 2D	STQ			Store quadword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 2E	STL_C		Store longword conditional	*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ, EboxL0L1},		/* 2F	STQ_C		Store quadword conditional	*/
	{Bra,	Branch,	{ .raw = AXP_DEST_RA}, AXP_IQ, EboxL0},						/* 30	BR			Unconditional branch		*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 31	FBEQ		Floating branch if = zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 32	FBLT		Floating branch if < zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 33	FBLE		Floating branch if <= zero	*/
	{Mbr,	Branch,	{ .raw = AXP_DEST_RA}, AXP_IQ, EboxL0},						/* 34	BSR			Branch to subroutine		*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 35	FBNE		Floating branch if != zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 36	FBGE		Floating branch if >=zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ, FboxOther},					/* 37	FBGT		Floating branch if > zero	*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 38	BLBC		Branch if low bit clear		*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 39	BEQ			Branch if = zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 3A	BLT			Branch if < zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 3B	BLE			Branch if <= zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 3C	BLBS		Branch if low bit set		*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 3D	BNE			Branch if != zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0},						/* 3E	BGE			Branch if >= zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ, EboxL0}						/* 3F	BGT			Branch if > zero			*/
};

static AXP_PIPELINE hw_mxpr_pipe[] =
{
		EboxL0,			/* ITB_TAG								- b'0000 0000' */
		EboxL0, 		/* ITB_PTE								- b'0000 0001' */
		EboxL0,			/* ITB_IAP								- b'0000 0010' */
		EboxL0,			/* ITB_IA								- b'0000 0011' */
		EboxL0,			/* ITB_IS								- b'0000 0100' */
		PipelineNone,
		EboxL0,			/* EXC_ADDR								- b'0000 0110' */
		EboxL0,			/* IVA_FORM								- b'0000 0111' */
		PipelineNone,
		EboxL0,			/* CM									- b'0000 1001' */
		EboxL0,			/* IER									- b'0000 1010' */
		EboxL0,			/* IER_CM								- b'0000 1011' */
		EboxL0,			/* SIRR									- b'0000 1100' */
		EboxL0L1,		/* ISUM									- b'0000 1101' */
		EboxL0,			/* HW_INT_CLR							- b'0000 1110' */
		EboxL0,			/* EXC_SUM								- b'0000 1111' */
		EboxL0,			/* PAL_BASE								- b'0001 0000' */
		EboxL0,			/* I_CTL								- b'0001 0001' */
		EboxL0,			/* IC_FLUSH_ASM							- b'0001 0010' */
		EboxL0,			/* IC_FLUSH								- b'0001 0011' */
		EboxL0,			/* PCTR_CTL								- b'0001 0100' */
		EboxL0,			/* CLR_MAP								- b'0001 0101' */
		EboxL0,			/* I_STAT								- b'0001 0110' */
		EboxL0,			/* SLEEP								- b'0001 0111' */
		PipelineNone,
		PipelineNone,
		EboxL0,			/* DTB_TAG0								- b'0010 0000' */
		EboxL0,			/* DTB_PTE0								- b'0010 0001' */
		PipelineNone,
		PipelineNone,
		EboxL0,			/* DTB_IS0								- b'0010 0100' */
		EboxL0,			/* DTB_ASN0								- b'0010 0101' */
		EboxL1,			/* DTB_ALTMODE							- b'0010 0110' */
		EboxL0L1,		/* MM_STAT								- b'0010 0111' */
		EboxL0,			/* M_CTL								- b'0010 1000' */
		EboxL0,			/* DC_CTL								- b'0010 1001' */
		EboxL0,			/* DC_STAT								- b'0010 1010' */
		EboxL0,			/* C_DATA								- b'0010 1011' */
		EboxL0,			/* C_SHFT								- b'0010 1100' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,	/*										- b'0011 0000' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		EboxL0,			/* PCXT									- b'0100 0000' */
		EboxL0,			/* PCXT[ASN]							- b'0100 0001' */
		EboxL0,			/* PCXT[ASTER]							- b'0100 0010' */
		EboxL0,			/* PCXT[ASTER, ASN]						- b'0100 0011' */
		EboxL0,			/* PCXT[ASTRR]							- b'0100 0100' */
		EboxL0,			/* PCXT[ASTRR, ASN]						- b'0100 0101' */
		EboxL0,			/* PCXT[ASTRR, ASTER]					- b'0100 0110' */
		EboxL0,			/* PCXT[ASTRR, ASTER, ASN]				- b'0100 0111' */
		EboxL0,			/* PCXT[PPCE]							- b'0100 1000' */
		EboxL0,			/* PCXT[PPCE, ASN]						- b'0100 1001' */
		EboxL0,			/* PCXT[PPCE, ASTER]					- b'0100 1010' */
		EboxL0,			/* PCXT[PPCE, ASTER, ASN]				- b'0100 1011' */
		EboxL0,			/* PCXT[PPCE, ASTRR]					- b'0100 1100' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASN]				- b'0100 1101' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASTER]				- b'0100 1110' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASTER, ASN]		- b'0100 1111' */
		EboxL0,			/* PCXT[FPE]							- b'0101 0000' */
		EboxL0,			/* PCXT[FPE, ASN]						- b'0101 0001' */
		EboxL0,			/* PCXT[FPE, ASTER]						- b'0101 0010' */
		EboxL0,			/* PCXT[FPE, ASTER, ASN]				- b'0101 0011' */
		EboxL0,			/* PCXT[FPE, ASTRR]						- b'0101 0100' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASN]				- b'0101 0101' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASTER]				- b'0101 0110' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASTER, ASN]			- b'0101 0111' */
		EboxL0,			/* PCXT[FPE, PPCE]						- b'0101 1000' */
		EboxL0,			/* PCXT[FPE, PPCE, ASN]					- b'0101 1001' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTER]				- b'0101 1010' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTER, ASN]			- b'0101 1011' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR]				- b'0101 1100' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASN]			- b'0101 1101' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASTER]		- b'0101 1110' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASTER, ASN]	- b'0101 1111' */
		EboxL0,			/* PCXT]								- b'0110 0000' */
		EboxL0,			/* PCXT[ASN]							- b'0110 0001' */
		EboxL0,			/* PCXT[ASTER]							- b'0110 0010' */
		EboxL0,			/* PCXT[ASTER, ASN]						- b'0110 0011' */
		EboxL0,			/* PCXT[ASTRR]							- b'0110 0100' */
		EboxL0,			/* PCXT[ASTRR, ASN]						- b'0110 0101' */
		EboxL0,			/* PCXT[ASTRR, ASTER]					- b'0110 0110' */
		EboxL0,			/* PCXT[ASTRR, ASTER, ASN]				- b'0110 0111' */
		EboxL0,			/* PCXT[PPCE]							- b'0110 1000' */
		EboxL0,			/* PCXT[PPCE, ASN]						- b'0110 1001' */
		EboxL0,			/* PCXT[PPCE, ASTER]					- b'0110 1010' */
		EboxL0,			/* PCXT[PPCE, ASTER, ASN]				- b'0110 1011' */
		EboxL0,			/* PCXT[PPCE, ASTRR]					- b'0110 1100' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASN]				- b'0110 1101' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASTER]				- b'0110 1110' */
		EboxL0,			/* PCXT[PPCE, ASTRR, ASTER, ASN]		- b'0110 1111' */
		EboxL0,			/* PCXT[FPE]							- b'0111 0000' */
		EboxL0,			/* PCXT[FPE, ASN]						- b'0111 0001' */
		EboxL0,			/* PCXT[FPE, ASTER]						- b'0111 0010' */
		EboxL0,			/* PCXT[FPE, ASTER, ASN]				- b'0111 0011' */
		EboxL0,			/* PCXT[FPE, ASTRR]						- b'0111 0100' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASN]				- b'0111 0101' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASTER]				- b'0111 0110' */
		EboxL0,			/* PCXT[FPE, ASTRR, ASTER, ASN]			- b'0111 0111' */
		EboxL0,			/* PCXT[FPE, PPCE]						- b'0111 1000' */
		EboxL0,			/* PCXT[FPE, PPCE, ASN]					- b'0111 1001' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTER]				- b'0111 1010' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTER, ASN]			- b'0111 1011' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR]				- b'0111 1100' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASN]			- b'0111 1101' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASTER]		- b'0111 1110' */
		EboxL0,			/* PCXT[FPE, PPCE, ASTRR, ASTER, ASN]]	- b'0111 1111' */
		PipelineNone,	/*										- b'1000 0000' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,	/*										- b'1001 0000' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		EboxL1,			/* DTB_TAG1								- b'1010 0000' */
		EboxL0,			/* DTB_PTE1								- b'1010 0001' */
		EboxL1,			/* DTB_IAP								- b'1010 0010' */
		EboxL1,			/* DTB_IA 								- b'1010 0011' */
		EboxL1,			/* DTB_IS1								- b'1010 0100' */
		EboxL1,			/* DTB_ASN1								- b'1010 0101' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,	/*										- b'1100 0000' */
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		PipelineNone,
		EboxL1,			/* CC									- b'1100 0000' */
		EboxL1,			/* CC_CTL								- b'1100 0001' */
		EboxL1,			/* VA									- b'1100 0010' */
		EboxL1,			/* VA_FORM								- b'1100 0011' */
		EboxL1			/* VA_CTL								- b'1100 0100' */
};

/*
 * AXP_Dispatcher
 *	This function is called to dispatch the instruction to the correct function
 *	for execution.  It also handles Invalid Opcode events.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Values:
 *	None.
 */
void AXP_Dispatcher(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{
	static const int	label[] =
	{
		&&OP_CALL_PAL - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&RESERVED_OP - &&OP_CALL_PAL,
		&&OP_LDA - &&OP_CALL_PAL,
		&&OP_LDAH - &&OP_CALL_PAL,
		&&OP_LDBU - &&OP_CALL_PAL,
		&&OP_LDQ_U - &&OP_CALL_PAL,
		&&OP_LDWU - &&OP_CALL_PAL,
		&&OP_STW - &&OP_CALL_PAL,
		&&OP_STB - &&OP_CALL_PAL,
		&&OP_STQ_U - &&OP_CALL_PAL,
		&&OP_ADDL - &&OP_CALL_PAL,
		&&OP_AND - &&OP_CALL_PAL,
		&&OP_MSKBL - &&OP_CALL_PAL,
		&&OP_MULL - &&OP_CALL_PAL,
		&&OP_ITOFS - &&OP_CALL_PAL,
		&&OP_ADDF - &&OP_CALL_PAL,
		&&OP_ADDS - &&OP_CALL_PAL,
		&&OP_CVTLQ - &&OP_CALL_PAL,
		&&OP_TRAPB - &&OP_CALL_PAL,
		&&OP_HW_MFPR - &&OP_CALL_PAL,
		&&OP_JMP - &&OP_CALL_PAL,
		&&OP_HW_LD - &&OP_CALL_PAL,
		&&OP_SEXTB - &&OP_CALL_PAL,
		&&OP_HW_MTPR - &&OP_CALL_PAL,
		&&OP_HW_RET - &&OP_CALL_PAL,
		&&OP_HW_ST - &&OP_CALL_PAL,
		&&OP_LDF - &&OP_CALL_PAL,
		&&OP_LDG - &&OP_CALL_PAL,
		&&OP_LDS - &&OP_CALL_PAL,
		&&OP_LDT - &&OP_CALL_PAL,
		&&OP_STF - &&OP_CALL_PAL,
		&&OP_STG - &&OP_CALL_PAL,
		&&OP_STS - &&OP_CALL_PAL,
		&&OP_STT - &&OP_CALL_PAL,
		&&OP_LDL - &&OP_CALL_PAL,
		&&OP_LDQ - &&OP_CALL_PAL,
		&&OP_LDL_L - &&OP_CALL_PAL,
		&&OP_LDQ_L - &&OP_CALL_PAL,
		&&OP_STL - &&OP_CALL_PAL,
		&&OP_STQ - &&OP_CALL_PAL,
		&&OP_STL_C - &&OP_CALL_PAL,
		&&OP_STQ_C - &&OP_CALL_PAL,
		&&OP_BR - &&OP_CALL_PAL,
		&&OP_FBEQ - &&OP_CALL_PAL,
		&&OP_FBLT - &&OP_CALL_PAL,
		&&OP_FBLE - &&OP_CALL_PAL,
		&&OP_BSR - &&OP_CALL_PAL,
		&&OP_FBNE - &&OP_CALL_PAL,
		&&OP_FBGE - &&OP_CALL_PAL,
		&&OP_FBGT - &&OP_CALL_PAL,
		&&OP_BLBC - &&OP_CALL_PAL,
		&&OP_BEQ - &&OP_CALL_PAL,
		&&OP_BLT - &&OP_CALL_PAL,
		&&OP_BLE - &&OP_CALL_PAL,
		&&OP_BLBS - &&OP_CALL_PAL,
		&&OP_BNE - &&OP_CALL_PAL,
		&&OP_BGE - &&OP_CALL_PAL,
		&&OP_BGT - &&OP_CALL_PAL
	};

	goto *(&&OP_CALL_PAL + label[instr->opcode]);

OP_CALL_PAL:
	/* TODO: Need to recalculate new PC and implement like a jump */
	goto COMPLETION_OP;

OP_LDA:
	instr->excRegMask = AXP_LDA(cpu, instr);
	goto COMPLETION_OP;

OP_LDAH:
	instr->excRegMask = AXP_LDAH(cpu, instr);
	goto COMPLETION_OP;

OP_LDBU:
	instr->excRegMask = AXP_LDBU(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ_U:
	instr->excRegMask = AXP_LDQ_U(cpu, instr);
	goto COMPLETION_OP;

OP_LDWU:
	instr->excRegMask = AXP_LDWU(cpu, instr);
	goto COMPLETION_OP;

OP_STW:
	instr->excRegMask = AXP_STW(cpu, instr);
	goto COMPLETION_OP;

OP_STB:
	instr->excRegMask = AXP_STB(cpu, instr);
	goto COMPLETION_OP;

OP_STQ_U:
	instr->excRegMask = AXP_STQ_U(cpu, instr);
	goto COMPLETION_OP;

OP_ADDL:
	switch (instr->function)
	{
		case AXP_FUNC_ADDL:
			instr->excRegMask = AXP_ADDL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S4ADDL:
			instr->excRegMask = AXP_S4ADDL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBL:
			instr->excRegMask = AXP_SUBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S4SUBL:
			instr->excRegMask = AXP_S4SUBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPBGE:
			instr->excRegMask = AXP_CMPBGE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S8ADDL:
			instr->excRegMask = AXP_S8ADDL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S8SUBL:
			instr->excRegMask = AXP_S8SUBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPULT:
			instr->excRegMask = AXP_CMPULT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDQ:
			instr->excRegMask = AXP_ADDQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S4ADDQ:
			instr->excRegMask = AXP_S4ADDQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBQ:
			instr->excRegMask = AXP_SUBQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S4SUBQ:
			instr->excRegMask = AXP_S4SUBQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPEQ:
			instr->excRegMask = AXP_CMPEQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S8ADDQ:
			instr->excRegMask = AXP_S8ADDQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_S8SUBQ:
			instr->excRegMask = AXP_S8SUBQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPULE:
			instr->excRegMask = AXP_CMPULE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDL_V:
			instr->excRegMask = AXP_ADDL_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBL_V:
			instr->excRegMask = AXP_SUBL_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPLT:
			instr->excRegMask = AXP_CMPLT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDQ_V:
			instr->excRegMask = AXP_ADDQ_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBQ_V:
			instr->excRegMask = AXP_SUBQ_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPLE:
			instr->excRegMask = AXP_CMPLE(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_AND:
	switch (instr->function)
	{
		case AXP_FUNC_AND:
			instr->excRegMask = AXP_AND(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_BIC:
			instr->excRegMask = AXP_BIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVLBS:
			instr->excRegMask = AXP_CMOVLBS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVLBC:
			instr->excRegMask = AXP_CMOVLBC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_BIS:
			instr->excRegMask = AXP_BIS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVEQ:
			instr->excRegMask = AXP_CMOVEQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVNE:
			instr->excRegMask = AXP_CMOVNE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ORNOT:
			instr->excRegMask = AXP_ORNOT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_XOR:
			instr->excRegMask = AXP_XOR(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVLT:
			instr->excRegMask = AXP_CMOVLT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVGE:
			instr->excRegMask = AXP_CMOVGE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EQV:
			instr->excRegMask = AXP_EQV(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_AMASK:
			instr->excRegMask = AXP_AMASK(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVLE:
			instr->excRegMask = AXP_CMOVLE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMOVGT:
			instr->excRegMask = AXP_CMOVGT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_IMPLVER:
			instr->excRegMask = AXP_IMPLVER(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_MSKBL:
	switch (instr->function)
	{
		case AXP_FUNC_MSKBL:
			instr->excRegMask = AXP_MSKBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTBL:
			instr->excRegMask = AXP_EXTBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSBL:
			instr->excRegMask = AXP_INSBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKWL:
			instr->excRegMask = AXP_MSKWL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTWL:
			instr->excRegMask = AXP_EXTWL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSWL:
			instr->excRegMask = AXP_INSWL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKLL:
			instr->excRegMask = AXP_MSKLL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTLL:
			instr->excRegMask = AXP_EXTLL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSLL:
			instr->excRegMask = AXP_INSLL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ZAP:
			instr->excRegMask = AXP_ZAP(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ZAPNOT:
			instr->excRegMask = AXP_ZAPNOT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKQL:
			instr->excRegMask = AXP_MSKQL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SRL:
			instr->excRegMask = AXP_SRL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTQL:
			instr->excRegMask = AXP_EXTQL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SLL:
			instr->excRegMask = AXP_SLL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSQL:
			instr->excRegMask = AXP_INSQL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SRA:
			instr->excRegMask = AXP_SRA(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKWH:
			instr->excRegMask = AXP_MSKWH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSWH:
			instr->excRegMask = AXP_INSWH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTWH:
			instr->excRegMask = AXP_EXTWH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKLH:
			instr->excRegMask = AXP_MSKLH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSLH:
			instr->excRegMask = AXP_INSLH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTLH:
			instr->excRegMask = AXP_EXTLH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MSKQH:
			instr->excRegMask = AXP_MSKQH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_INSQH:
			instr->excRegMask = AXP_INSQH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXTQH:
			instr->excRegMask = AXP_EXTQH(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_MULL:
	switch (instr->function)
	{
		case AXP_FUNC_MULL:
			instr->excRegMask = AXP_MULL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULQ:
			instr->excRegMask = AXP_MULQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_UMULH:
			instr->excRegMask = AXP_UMULH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULL_V:
			instr->excRegMask = AXP_MULL_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULQ_V:
			instr->excRegMask = AXP_MULQ_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_ITOFS:
	switch (instr->function)
	{
		case AXP_FUNC_ITOFS:
			instr->excRegMask = AXP_ITOFS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_C:
			instr->excRegMask = AXP_SQRTF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_C:
			instr->excRegMask = AXP_SQRTS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ITOFF:
			instr->excRegMask = AXP_ITOFF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ITOFT:
			instr->excRegMask = AXP_ITOFT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_C:
			instr->excRegMask = AXP_SQRTG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_C:
			instr->excRegMask = AXP_SQRTT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_M:
			instr->excRegMask = AXP_SQRTS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_M:
			instr->excRegMask = AXP_SQRTT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF:
			instr->excRegMask = AXP_SQRTF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS:
			instr->excRegMask = AXP_SQRTS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG:
			instr->excRegMask = AXP_SQRTG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT:
			instr->excRegMask = AXP_SQRTT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_D:
			instr->excRegMask = AXP_SQRTS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_D:
			instr->excRegMask = AXP_SQRTT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_UC:
			instr->excRegMask = AXP_SQRTF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_UC:
			instr->excRegMask = AXP_SQRTS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_UC:
			instr->excRegMask = AXP_SQRTG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_UC:
			instr->excRegMask = AXP_SQRTT_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_UM:
			instr->excRegMask = AXP_SQRTS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_UM:
			instr->excRegMask = AXP_SQRTT_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_U:
			instr->excRegMask = AXP_SQRTF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_U:
			instr->excRegMask = AXP_SQRTS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_U:
			instr->excRegMask = AXP_SQRTG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_U:
			instr->excRegMask = AXP_SQRTT_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_UD:
			instr->excRegMask = AXP_SQRTS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_UD:
			instr->excRegMask = AXP_SQRTT_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_SC:
			instr->excRegMask = AXP_SQRTF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_SC:
			instr->excRegMask = AXP_SQRTG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_S:
			instr->excRegMask = AXP_SQRTF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_S:
			instr->excRegMask = AXP_SQRTG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_SUC:
			instr->excRegMask = AXP_SQRTF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUC:
			instr->excRegMask = AXP_SQRTS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_SUC:
			instr->excRegMask = AXP_SQRTG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUC:
			instr->excRegMask = AXP_SQRTT_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUM:
			instr->excRegMask = AXP_SQRTS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUM:
			instr->excRegMask = AXP_SQRTT_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTF_SU:
			instr->excRegMask = AXP_SQRTF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SU:
			instr->excRegMask = AXP_SQRTS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTG_SU:
			instr->excRegMask = AXP_SQRTG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SU:
			instr->excRegMask = AXP_SQRTT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUD:
			instr->excRegMask = AXP_SQRTS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUD:
			instr->excRegMask = AXP_SQRTT_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUIC:
			instr->excRegMask = AXP_SQRTS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUIC:
			instr->excRegMask = AXP_SQRTT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUIM:
			instr->excRegMask = AXP_SQRTS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUIM:
			instr->excRegMask = AXP_SQRTT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUI:
			instr->excRegMask = AXP_SQRTS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUI:
			instr->excRegMask = AXP_SQRTT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTS_SUID:
			instr->excRegMask = AXP_SQRTS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SQRTT_SUID:
			instr->excRegMask = AXP_SQRTT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_ADDF:
	switch (instr->function)
	{
		case AXP_FUNC_ADDF_C:
			instr->excRegMask = AXP_ADDF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_C:
			instr->excRegMask = AXP_SUBF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_C:
			instr->excRegMask = AXP_MULF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_C:
			instr->excRegMask = AXP_DIVF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_C:
			instr->excRegMask = AXP_CVTDG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_C:
			instr->excRegMask = AXP_ADDG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_C:
			instr->excRegMask = AXP_SUBG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_C:
			instr->excRegMask = AXP_MULG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_C:
			instr->excRegMask = AXP_DIVG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_C:
			instr->excRegMask = AXP_CVTGF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_C:
			instr->excRegMask = AXP_CVTGD_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_C:
			instr->excRegMask = AXP_CVTGQ_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQF_C:
			instr->excRegMask = AXP_CVTQF_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQG_C:
			instr->excRegMask = AXP_CVTQG_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF:
			instr->excRegMask = AXP_ADDF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF:
			instr->excRegMask = AXP_SUBF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF:
			instr->excRegMask = AXP_MULF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF:
			instr->excRegMask = AXP_DIVF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG:
			instr->excRegMask = AXP_CVTDG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG:
			instr->excRegMask = AXP_ADDG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG:
			instr->excRegMask = AXP_SUBG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG:
			instr->excRegMask = AXP_MULG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG:
			instr->excRegMask = AXP_DIVG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGEQ:
			instr->excRegMask = AXP_CMPGEQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGLT:
			instr->excRegMask = AXP_CMPGLT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGLE:
			instr->excRegMask = AXP_CMPGLE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF:
			instr->excRegMask = AXP_CVTGF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD:
			instr->excRegMask = AXP_CVTGD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ:
			instr->excRegMask = AXP_CVTGQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQF:
			instr->excRegMask = AXP_CVTQF(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQG:
			instr->excRegMask = AXP_CVTQG(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_UC:
			instr->excRegMask = AXP_ADDF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_UC:
			instr->excRegMask = AXP_SUBF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_UC:
			instr->excRegMask = AXP_MULF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_UC:
			instr->excRegMask = AXP_DIVF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_UC:
			instr->excRegMask = AXP_CVTDG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_UC:
			instr->excRegMask = AXP_ADDG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_UC:
			instr->excRegMask = AXP_SUBG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_UC:
			instr->excRegMask = AXP_MULG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_UC:
			instr->excRegMask = AXP_DIVG_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_UC:
			instr->excRegMask = AXP_CVTGF_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_UC:
			instr->excRegMask = AXP_CVTGD_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_VC:
			instr->excRegMask = AXP_CVTGQ_VC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_U:
			instr->excRegMask = AXP_ADDF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_U:
			instr->excRegMask = AXP_SUBF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_U:
			instr->excRegMask = AXP_MULF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_U:
			instr->excRegMask = AXP_DIVF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_U:
			instr->excRegMask = AXP_CVTDG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_U:
			instr->excRegMask = AXP_ADDG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_U:
			instr->excRegMask = AXP_SUBG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_U:
			instr->excRegMask = AXP_MULG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_U:
			instr->excRegMask = AXP_DIVG_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_U:
			instr->excRegMask = AXP_CVTGF_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_U:
			instr->excRegMask = AXP_CVTGD_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_V:
			instr->excRegMask = AXP_CVTGQ_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_SC:
			instr->excRegMask = AXP_ADDF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_SC:
			instr->excRegMask = AXP_SUBF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_SC:
			instr->excRegMask = AXP_MULF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_SC:
			instr->excRegMask = AXP_DIVF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_SC:
			instr->excRegMask = AXP_CVTDG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_SC:
			instr->excRegMask = AXP_ADDG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_SC:
			instr->excRegMask = AXP_SUBG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_SC:
			instr->excRegMask = AXP_MULG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_SC:
			instr->excRegMask = AXP_DIVG_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_SC:
			instr->excRegMask = AXP_CVTGF_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_SC:
			instr->excRegMask = AXP_CVTGD_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_SC:
			instr->excRegMask = AXP_CVTGQ_SC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_S:
			instr->excRegMask = AXP_ADDF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_S:
			instr->excRegMask = AXP_SUBF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_S:
			instr->excRegMask = AXP_MULF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_S:
			instr->excRegMask = AXP_DIVF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_S:
			instr->excRegMask = AXP_CVTDG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_S:
			instr->excRegMask = AXP_ADDG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_S:
			instr->excRegMask = AXP_SUBG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_S:
			instr->excRegMask = AXP_MULG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_S:
			instr->excRegMask = AXP_DIVG_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGEQ_S:
			instr->excRegMask = AXP_CMPGEQ_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGLT_S:
			instr->excRegMask = AXP_CMPGLT_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPGLE_S:
			instr->excRegMask = AXP_CMPGLE_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_S:
			instr->excRegMask = AXP_CVTGF_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_S:
			instr->excRegMask = AXP_CVTGD_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_S:
			instr->excRegMask = AXP_CVTGQ_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_SUC:
			instr->excRegMask = AXP_ADDF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_SUC:
			instr->excRegMask = AXP_SUBF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_SUC:
			instr->excRegMask = AXP_MULF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_SUC:
			instr->excRegMask = AXP_DIVF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_SUC:
			instr->excRegMask = AXP_CVTDG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_SUC:
			instr->excRegMask = AXP_ADDG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_SUC:
			instr->excRegMask = AXP_SUBG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_SUC:
			instr->excRegMask = AXP_MULG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_SUC:
			instr->excRegMask = AXP_DIVG_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_SUC:
			instr->excRegMask = AXP_CVTGF_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_SUC:
			instr->excRegMask = AXP_CVTGD_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_SVC:
			instr->excRegMask = AXP_CVTGQ_SVC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDF_SU:
			instr->excRegMask = AXP_ADDF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBF_SU:
			instr->excRegMask = AXP_SUBF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULF_SU:
			instr->excRegMask = AXP_MULF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVF_SU:
			instr->excRegMask = AXP_DIVF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTDG_SU:
			instr->excRegMask = AXP_CVTDG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDG_SU:
			instr->excRegMask = AXP_ADDG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBG_SU:
			instr->excRegMask = AXP_SUBG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULG_SU:
			instr->excRegMask = AXP_MULG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVG_SU:
			instr->excRegMask = AXP_DIVG_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGF_SU:
			instr->excRegMask = AXP_CVTGF_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGD_SU:
			instr->excRegMask = AXP_CVTGD_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTGQ_SV:
			instr->excRegMask = AXP_CVTGQ_SV(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_ADDS:
	switch (instr->function)
	{
		case AXP_FUNC_ADDS_C:
			instr->excRegMask = AXP_ADDS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_C:
			instr->excRegMask = AXP_SUBS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_C:
			instr->excRegMask = AXP_MULS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_C:
			instr->excRegMask = AXP_DIVS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_C:
			instr->excRegMask = AXP_ADDT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_C:
			instr->excRegMask = AXP_SUBT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_C:
			instr->excRegMask = AXP_MULT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_C:
			instr->excRegMask = AXP_DIVT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_C:
			instr->excRegMask = AXP_CVTTS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_C:
			instr->excRegMask = AXP_CVTTQ_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_C:
			instr->excRegMask = AXP_CVTQS_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_C:
			instr->excRegMask = AXP_CVTQT_C(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_M:
			instr->excRegMask = AXP_ADDS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_M:
			instr->excRegMask = AXP_SUBS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_M:
			instr->excRegMask = AXP_MULS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_M:
			instr->excRegMask = AXP_DIVS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_M:
			instr->excRegMask = AXP_ADDT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_M:
			instr->excRegMask = AXP_SUBT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_M:
			instr->excRegMask = AXP_MULT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_M:
			instr->excRegMask = AXP_DIVT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_M:
			instr->excRegMask = AXP_CVTTS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_M:
			instr->excRegMask = AXP_CVTTQ_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_M:
			instr->excRegMask = AXP_CVTQS_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_M:
			instr->excRegMask = AXP_CVTQT_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS:
			instr->excRegMask = AXP_ADDS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS:
			instr->excRegMask = AXP_SUBS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS:
			instr->excRegMask = AXP_MULS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS:
			instr->excRegMask = AXP_DIVS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT:
			instr->excRegMask = AXP_ADDT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT:
			instr->excRegMask = AXP_SUBT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT:
			instr->excRegMask = AXP_MULT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT:
			instr->excRegMask = AXP_DIVT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTUN:
			instr->excRegMask = AXP_CMPTUN(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTEQ:
			instr->excRegMask = AXP_CMPTEQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTLT:
			instr->excRegMask = AXP_CMPTLT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTLE:
			instr->excRegMask = AXP_CMPTLE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS:
			instr->excRegMask = AXP_CVTTS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ:
			instr->excRegMask = AXP_CVTTQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS:
			instr->excRegMask = AXP_CVTQS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT:
			instr->excRegMask = AXP_CVTQT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_D:
			instr->excRegMask = AXP_ADDS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_D:
			instr->excRegMask = AXP_SUBS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_D:
			instr->excRegMask = AXP_MULS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_D:
			instr->excRegMask = AXP_DIVS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_D:
			instr->excRegMask = AXP_ADDT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_D:
			instr->excRegMask = AXP_SUBT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_D:
			instr->excRegMask = AXP_MULT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_D:
			instr->excRegMask = AXP_DIVT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_D:
			instr->excRegMask = AXP_CVTTS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_D:
			instr->excRegMask = AXP_CVTTQ_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_D:
			instr->excRegMask = AXP_CVTQS_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_D:
			instr->excRegMask = AXP_CVTQT_D(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_UC:
			instr->excRegMask = AXP_ADDS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_UC:
			instr->excRegMask = AXP_SUBS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_UC:
			instr->excRegMask = AXP_MULS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_UC:
			instr->excRegMask = AXP_DIVS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_UC:
			instr->excRegMask = AXP_ADDT_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_UC:
			instr->excRegMask = AXP_SUBT_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_UC:
			instr->excRegMask = AXP_MULT_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_UC:
			instr->excRegMask = AXP_DIVT_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_UC:
			instr->excRegMask = AXP_CVTTS_UC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_VC:
			instr->excRegMask = AXP_CVTTQ_VC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_UM:
			instr->excRegMask = AXP_ADDS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_UM:
			instr->excRegMask = AXP_SUBS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_UM:
			instr->excRegMask = AXP_MULS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_UM:
			instr->excRegMask = AXP_DIVS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_UM:
			instr->excRegMask = AXP_ADDT_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_UM:
			instr->excRegMask = AXP_SUBT_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_UM:
			instr->excRegMask = AXP_MULT_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_UM:
			instr->excRegMask = AXP_DIVT_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_UM:
			instr->excRegMask = AXP_CVTTS_UM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_VM:
			instr->excRegMask = AXP_CVTTQ_VM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_U:
			instr->excRegMask = AXP_ADDS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_U:
			instr->excRegMask = AXP_SUBS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_U:
			instr->excRegMask = AXP_MULS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_U:
			instr->excRegMask = AXP_DIVS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_U:
			instr->excRegMask = AXP_ADDT_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_U:
			instr->excRegMask = AXP_SUBT_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_U:
			instr->excRegMask = AXP_MULT_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_U:
			instr->excRegMask = AXP_DIVT_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_U:
			instr->excRegMask = AXP_CVTTS_U(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_V:
			instr->excRegMask = AXP_CVTTQ_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_UD:
			instr->excRegMask = AXP_ADDS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_UD:
			instr->excRegMask = AXP_SUBS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_UD:
			instr->excRegMask = AXP_MULS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_UD:
			instr->excRegMask = AXP_DIVS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_UD:
			instr->excRegMask = AXP_ADDT_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_UD:
			instr->excRegMask = AXP_SUBT_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_UD:
			instr->excRegMask = AXP_MULT_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_UD:
			instr->excRegMask = AXP_DIVT_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_UD:
			instr->excRegMask = AXP_CVTTS_UD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_VD:
			instr->excRegMask = AXP_CVTTQ_VD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTST:
			instr->excRegMask = AXP_CVTST(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUC:
			instr->excRegMask = AXP_ADDS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUC:
			instr->excRegMask = AXP_SUBS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUC:
			instr->excRegMask = AXP_MULS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUC:
			instr->excRegMask = AXP_DIVS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUC:
			instr->excRegMask = AXP_ADDT_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUC:
			instr->excRegMask = AXP_SUBT_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUC:
			instr->excRegMask = AXP_MULT_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUC:
			instr->excRegMask = AXP_DIVT_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUC:
			instr->excRegMask = AXP_CVTTS_SUC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVC:
			instr->excRegMask = AXP_CVTTQ_SVC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUM:
			instr->excRegMask = AXP_ADDS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUM:
			instr->excRegMask = AXP_SUBS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUM:
			instr->excRegMask = AXP_MULS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUM:
			instr->excRegMask = AXP_DIVS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUM:
			instr->excRegMask = AXP_ADDT_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUM:
			instr->excRegMask = AXP_SUBT_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUM:
			instr->excRegMask = AXP_MULT_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUM:
			instr->excRegMask = AXP_DIVT_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUM:
			instr->excRegMask = AXP_CVTTS_SUM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVM:
			instr->excRegMask = AXP_CVTTQ_SVM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SU:
			instr->excRegMask = AXP_ADDS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SU:
			instr->excRegMask = AXP_SUBS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SU:
			instr->excRegMask = AXP_MULS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SU:
			instr->excRegMask = AXP_DIVS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SU:
			instr->excRegMask = AXP_ADDT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SU:
			instr->excRegMask = AXP_SUBT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SU:
			instr->excRegMask = AXP_MULT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SU:
			instr->excRegMask = AXP_DIVT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTUN_SU:
			instr->excRegMask = AXP_CMPTUN_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTEQ_SU:
			instr->excRegMask = AXP_CMPTEQ_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTLT_SU:
			instr->excRegMask = AXP_CMPTLT_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CMPTLE_SU:
			instr->excRegMask = AXP_CMPTLE_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SU:
			instr->excRegMask = AXP_CVTTS_SU(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SV:
			instr->excRegMask = AXP_CVTTQ_SV(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUD:
			instr->excRegMask = AXP_ADDS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUD:
			instr->excRegMask = AXP_SUBS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUD:
			instr->excRegMask = AXP_MULS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUD:
			instr->excRegMask = AXP_DIVS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUD:
			instr->excRegMask = AXP_ADDT_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUD:
			instr->excRegMask = AXP_SUBT_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUD:
			instr->excRegMask = AXP_MULT_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUD:
			instr->excRegMask = AXP_DIVT_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUD:
			instr->excRegMask = AXP_CVTTS_SUD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVD:
			instr->excRegMask = AXP_CVTTQ_SVD(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTST_S:
			instr->excRegMask = AXP_CVTST_S(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUIC:
			instr->excRegMask = AXP_ADDS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUIC:
			instr->excRegMask = AXP_SUBS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUIC:
			instr->excRegMask = AXP_MULS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUIC:
			instr->excRegMask = AXP_DIVS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUIC:
			instr->excRegMask = AXP_ADDT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUIC:
			instr->excRegMask = AXP_SUBT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUIC:
			instr->excRegMask = AXP_MULT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUIC:
			instr->excRegMask = AXP_DIVT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUIC:
			instr->excRegMask = AXP_CVTTS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVIC:
			instr->excRegMask = AXP_CVTTQ_SVIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_SUIC:
			instr->excRegMask = AXP_CVTQS_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_SUIC:
			instr->excRegMask = AXP_CVTQT_SUIC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUIM:
			instr->excRegMask = AXP_ADDS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUIM:
			instr->excRegMask = AXP_SUBS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUIM:
			instr->excRegMask = AXP_MULS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUIM:
			instr->excRegMask = AXP_DIVS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUIM:
			instr->excRegMask = AXP_ADDT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUIM:
			instr->excRegMask = AXP_SUBT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUIM:
			instr->excRegMask = AXP_MULT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUIM:
			instr->excRegMask = AXP_DIVT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUIM:
			instr->excRegMask = AXP_CVTTS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVIM:
			instr->excRegMask = AXP_CVTTQ_SVIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_SUIM:
			instr->excRegMask = AXP_CVTQS_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_SUIM:
			instr->excRegMask = AXP_CVTQT_SUIM(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUI:
			instr->excRegMask = AXP_ADDS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUI:
			instr->excRegMask = AXP_SUBS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUI:
			instr->excRegMask = AXP_MULS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUI:
			instr->excRegMask = AXP_DIVS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUI:
			instr->excRegMask = AXP_ADDT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUI:
			instr->excRegMask = AXP_SUBT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUI:
			instr->excRegMask = AXP_MULT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUI:
			instr->excRegMask = AXP_DIVT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUI:
			instr->excRegMask = AXP_CVTTS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVI:
			instr->excRegMask = AXP_CVTTQ_SVI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_SUI:
			instr->excRegMask = AXP_CVTQS_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_SUI:
			instr->excRegMask = AXP_CVTQT_SUI(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDS_SUID:
			instr->excRegMask = AXP_ADDS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBS_SUID:
			instr->excRegMask = AXP_SUBS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULS_SUID:
			instr->excRegMask = AXP_MULS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVS_SUID:
			instr->excRegMask = AXP_DIVS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ADDT_SUID:
			instr->excRegMask = AXP_ADDT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SUBT_SUID:
			instr->excRegMask = AXP_SUBT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MULT_SUID:
			instr->excRegMask = AXP_MULT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_DIVT_SUID:
			instr->excRegMask = AXP_DIVT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTS_SUID:
			instr->excRegMask = AXP_CVTTS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTTQ_SVID:
			instr->excRegMask = AXP_CVTTQ_SVID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQS_SUID:
			instr->excRegMask = AXP_CVTQS_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQT_SUID:
			instr->excRegMask = AXP_CVTQT_SUID(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_CVTLQ:
	switch (instr->function)
	{
		case AXP_FUNC_CVTLQ:
			instr->excRegMask = AXP_CVTLQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CPYS:
			instr->excRegMask = AXP_CPYS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CPYSN:
			instr->excRegMask = AXP_CPYSN(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CPYSE:
			instr->excRegMask = AXP_CPYSE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MT_FPCR:
			instr->excRegMask = AXP_MT_FPCR(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MF_FPCR:
			instr->excRegMask = AXP_MF_FPCR(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVEQ:
			instr->excRegMask = AXP_FCMOVEQ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVNE:
			instr->excRegMask = AXP_FCMOVNE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVLT:
			instr->excRegMask = AXP_FCMOVLT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVGE:
			instr->excRegMask = AXP_FCMOVGE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVLE:
			instr->excRegMask = AXP_FCMOVLE(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FCMOVGT:
			instr->excRegMask = AXP_FCMOVGT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQL:
			instr->excRegMask = AXP_CVTQL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQL_V:
			instr->excRegMask = AXP_CVTQL_V(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CVTQL_SV:
			instr->excRegMask = AXP_CVTQL_SV(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_TRAPB:
	switch (instr->function)
	{
		case AXP_FUNC_TRAPB:
			instr->excRegMask = AXP_TRAPB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_EXCB:
			instr->excRegMask = AXP_EXCB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MB:
			instr->excRegMask = AXP_MB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_WMB:
			instr->excRegMask = AXP_WMB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FETCH:
			instr->excRegMask = AXP_FETCH(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FETCH_M:
			instr->excRegMask = AXP_FETCH_M(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_RPCC:
			instr->excRegMask = AXP_RPCC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_RC:
			instr->excRegMask = AXP_RC(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_ECB:
			instr->excRegMask = AXP_ECB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_RS:
			instr->excRegMask = AXP_RS(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_WH64:
			instr->excRegMask = AXP_WH64(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_WH64EN:
			instr->excRegMask = AXP_WH64EN(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_HW_MFPR:
	if ((instr->pc.pal == AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWMFPR(cpu, instr);
	goto COMPLETION_OP;

OP_JMP:
	instr->excRegMask = AXP_JMP(cpu, instr);
	goto COMPLETION_OP;

OP_HW_LD:
	if ((instr->pc.pal == AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWLD(cpu, instr);
	goto COMPLETION_OP;

OP_SEXTB:
	switch (instr->function)
	{
		case AXP_FUNC_SEXTB:
			instr->excRegMask = AXP_SEXTB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_SEXTW:
			instr->excRegMask = AXP_SEXTW(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CTPOP:
			instr->excRegMask = AXP_CTPOP(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_PERR:
			instr->excRegMask = AXP_PERR(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CTLZ:
			instr->excRegMask = AXP_CTLZ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_CTTZ:
			instr->excRegMask = AXP_CTTZ(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_UNPKBW:
			instr->excRegMask = AXP_UNPKBW(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_UNPKBL:
			instr->excRegMask = AXP_UNPKBL(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_PKWB:
			instr->excRegMask = AXP_PKWB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_PKLB:
			instr->excRegMask = AXP_PKLB(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MINSB8:
			instr->excRegMask = AXP_MINSB8(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MINSW4:
			instr->excRegMask = AXP_MINSW4(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MINUB8:
			instr->excRegMask = AXP_MINUB8(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MINUW4:
			instr->excRegMask = AXP_MINUW4(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MAXUB8:
			instr->excRegMask = AXP_MAXUB8(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MAXUW4:
			instr->excRegMask = AXP_MAXUW4(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MAXSB8:
			instr->excRegMask = AXP_MAXSB8(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_MAXSW4:
			instr->excRegMask = AXP_MAXSW4(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FTOIT:
			instr->excRegMask = AXP_FTOIT(cpu, instr);
			goto COMPLETION_OP;
			break;

		case AXP_FUNC_FTOIS:
			instr->excRegMask = AXP_FTOIS(cpu, instr);
			goto COMPLETION_OP;
			break;

		default:
			goto RESERVED_OP;
			break;
	}

OP_HW_MTPR:
	if ((instr->pc.pal == AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWMTPR(cpu, instr);
	goto COMPLETION_OP;

OP_HW_RET:
	if ((instr->pc.pal == AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWRET(cpu, instr);
	goto COMPLETION_OP;

OP_HW_ST:
	if ((instr->pc.pal == AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWST(cpu, instr);
	goto COMPLETION_OP;

OP_LDF:
	instr->excRegMask = AXP_LDF(cpu, instr);
	goto COMPLETION_OP;

OP_LDG:
	instr->excRegMask = AXP_LDG(cpu, instr);
	goto COMPLETION_OP;

OP_LDS:
	instr->excRegMask = AXP_LDS(cpu, instr);
	goto COMPLETION_OP;

OP_LDT:
	instr->excRegMask = AXP_LDT(cpu, instr);
	goto COMPLETION_OP;

OP_STF:
	instr->excRegMask = AXP_STF(cpu, instr);
	goto COMPLETION_OP;

OP_STG:
	instr->excRegMask = AXP_STG(cpu, instr);
	goto COMPLETION_OP;

OP_STS:
	instr->excRegMask = AXP_STS(cpu, instr);
	goto COMPLETION_OP;

OP_STT:
	instr->excRegMask = AXP_STT(cpu, instr);
	goto COMPLETION_OP;

OP_LDL:
	instr->excRegMask = AXP_LDL(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ:
	instr->excRegMask = AXP_LDQ(cpu, instr);
	goto COMPLETION_OP;

OP_LDL_L:
	instr->excRegMask = AXP_LDL_L(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ_L:
	instr->excRegMask = AXP_LDQ_L(cpu, instr);
	goto COMPLETION_OP;

OP_STL:
	instr->excRegMask = AXP_STL(cpu, instr);
	goto COMPLETION_OP;

OP_STQ:
	instr->excRegMask = AXP_STQ(cpu, instr);
	goto COMPLETION_OP;

OP_STL_C:
	instr->excRegMask = AXP_STL_C(cpu, instr);
	goto COMPLETION_OP;

OP_STQ_C:
	instr->excRegMask = AXP_STQ_C(cpu, instr);
	goto COMPLETION_OP;

OP_BR:
	instr->excRegMask = AXP_BR(cpu, instr);
	goto COMPLETION_OP;

OP_FBEQ:
	instr->excRegMask = AXP_FBEQ(cpu, instr);
	goto COMPLETION_OP;

OP_FBLT:
	instr->excRegMask = AXP_FBLT(cpu, instr);
	goto COMPLETION_OP;

OP_FBLE:
	instr->excRegMask = AXP_FBLE(cpu, instr);
	goto COMPLETION_OP;

OP_BSR:
	instr->excRegMask = AXP_BSR(cpu, instr);
	goto COMPLETION_OP;

OP_FBNE:
	instr->excRegMask = AXP_FBNE(cpu, instr);
	goto COMPLETION_OP;

OP_FBGE:
	instr->excRegMask = AXP_FBGE(cpu, instr);
	goto COMPLETION_OP;

OP_FBGT:
	instr->excRegMask = AXP_FBGT(cpu, instr);
	goto COMPLETION_OP;

OP_BLBC:
	instr->excRegMask = AXP_BLBC(cpu, instr);
	goto COMPLETION_OP;

OP_BEQ:
	instr->excRegMask = AXP_BEQ(cpu, instr);
	goto COMPLETION_OP;

OP_BLT:
	instr->excRegMask = AXP_BLT(cpu, instr);
	goto COMPLETION_OP;

OP_BLE:
	instr->excRegMask = AXP_BLE(cpu, instr);
	goto COMPLETION_OP;

OP_BLBS:
	instr->excRegMask = AXP_BLBS(cpu, instr);
	goto COMPLETION_OP;

OP_BNE:
	instr->excRegMask = AXP_BNE(cpu, instr);
	goto COMPLETION_OP;

OP_BGE:
	instr->excRegMask = AXP_BGE(cpu, instr);
	goto COMPLETION_OP;

OP_BGT:
	instr->excRegMask = AXP_BGT(cpu, instr);
	goto COMPLETION_OP;

RESERVED_OP:
	instr->excRegMask = IllegalOperand;
	AXP_21264_Ibox_Event(
					cpu,
					AXP_OPCDEC,
					instr->pc,
					instr->pc,
					instr->opcode,
					AXP_UNMAPPED_REG,
					false,
					false);

COMPLETION_OP:

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_InstructionFormat
 *	This function is called to determine what format of instruction is specified
 *	in the supplied 32-bit instruction.
 *
 * Input Parameters:
 *	inst:
 *		An Alpha AXP formatted instruction.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	A value representing the type of instruction specified.
 */
AXP_INS_TYPE AXP_InstructionFormat(AXP_INS_FMT inst)
{
	AXP_INS_TYPE retVal = Res;

	/*
	 * Opcodes can only be between 0x00 and 0x3f.  Any other value is reserved.
	 */
	if ((inst.pal.opcode >= 0x00) && (inst.pal.opcode <= 0x3f))
	{

		/*
		 * Look up the instruction type from the instruction array, which is
		 * indexed by opcode.
		 */
		retVal = insDecode[inst.pal.opcode].format;

		/*
		 * If the returned value from the above look up is 'Cond', then we are
		 * dealing with opcode = 0x1c.  This particular opcode has 2 potential
		 * values, depending upon the function code.  If the function code is
		 * either 0x70 or 0x78, then type instruction type is 'FP' (Floating
		 * Point).  Otherwise, it is 'Opr'.
		 */
		if (retVal == Cond)
			retVal = ((inst.fp.func == 0x70) || (inst.fp.func == 0x78)) ? FP : Opr;
	}

	/*
	 * Return what we found to the caller.
	 */
	return(retVal);
}

/*
 * AXP_OperationType
 *	This function is called to determine what operation of instruction is specified
 *	in the supplied 32-bit instruction.
 *
 * Input Parameters:
 *	opcode:
 *		An Alpha AXP operation code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	A value representing the type of instruction specified.
 */
AXP_OPER_TYPE AXP_OperationType(u32 opcode)
{
	AXP_OPER_TYPE retVal = Other;

	/*
	 * Opcodes can only be between 0x00 and 0x3f.  Any other value is reserved.
	 */
	if ((opcode >= 0x00) && (opcode <= 0x3f))
	{

		/*
		 * Look up the instruction type from the instruction array, which is
		 * indexed by opcode.
		 */
		retVal = insDecode[opcode].type;
	}

	/*
	 * Return what we found to the caller.
	 */
	return(retVal);
}

/*
 * AXP_RegisterDecoding
 *	This function is called to determine what the registers of instruction
 *	specified in the supplied 32-bit instruction are used for (destination,
 *	source 1, and source 2).
 *
 * Input Parameters:
 *	opcode:
 *		An Alpha AXP operation code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	A value representing the type of instruction specified.
 */
AXP_REG_DECODE AXP_RegisterDecoding(u32 opcode)
{
	AXP_REG_DECODE retVal = {.raw = 0};

	/*
	 * Opcodes can only be between 0x00 and 0x3f.  Any other value is reserved.
	 */
	if ((opcode >= 0x00) && (opcode <= 0x3f))
	{

		/*
		 * Look up the instruction type from the instruction array, which is
		 * indexed by opcode.
		 */
		retVal = insDecode[opcode].registers;
	}

	/*
	 * Return what we found to the caller.
	 */
	return(retVal);
}

/*
 * AXP_InstructionQueue
 *	This function is called to determine what instruction queue the instruction
 *	is specified in the supplied 32-bit instruction.
 *
 * Input Parameters:
 *	opcode:
 *		An Alpha AXP operation code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	A value representing the type of instruction specified.
 */
u16 AXP_InstructionQueue(u32 opcode)
{
	u16 retVal = AXP_NONE;

	/*
	 * Opcodes can only be between 0x00 and 0x3f.  Any other value is reserved.
	 */
	if ((opcode >= 0x00) && (opcode <= 0x3f))
	{

		/*
		 * Look up the instruction type from the instruction array, which is
		 * indexed by opcode.
		 */
		retVal = insDecode[opcode].whichQ;
	}

	/*
	 * Return what we found to the caller.
	 */
	return(retVal);
}

/*
 * AXP_InstructionPipeline
 *	This function is called to determine what instruction pipeline the
 *	instruction is allowed to execute.
 *
 * Input Parameters:
 *	opcode:
 *		An Alpha AXP operation code.
 *	func:
 *		An Alpha AXP function code, to qualify the operation code.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	A value representing the pipeline of instruction specified.
 */
AXP_PIPELINE AXP_InstructionPipeline(u32 opcode, u32 func)
{
	AXP_PIPELINE	retVal = insDecode[opcode].pipeline;

	/*
	 * Some opcode functions are executed in different pipelines.
	 */
	switch(opcode)
	{
		case ITFP:
			switch(func)
			{
				case AXP_FUNC_SQRTF_C:
				case AXP_FUNC_SQRTS_C:
				case AXP_FUNC_SQRTG_C:
				case AXP_FUNC_SQRTT_C:
				case AXP_FUNC_SQRTS_M:
				case AXP_FUNC_SQRTT_M:
				case AXP_FUNC_SQRTF:
				case AXP_FUNC_SQRTS:
				case AXP_FUNC_SQRTG:
				case AXP_FUNC_SQRTT:
				case AXP_FUNC_SQRTS_D:
				case AXP_FUNC_SQRTT_D:
				case AXP_FUNC_SQRTF_UC:
				case AXP_FUNC_SQRTS_UC:
				case AXP_FUNC_SQRTG_UC:
				case AXP_FUNC_SQRTT_UC:
				case AXP_FUNC_SQRTS_UM:
				case AXP_FUNC_SQRTT_UM:
				case AXP_FUNC_SQRTF_U:
				case AXP_FUNC_SQRTS_U:
				case AXP_FUNC_SQRTG_U:
				case AXP_FUNC_SQRTT_U:
				case AXP_FUNC_SQRTS_UD:
				case AXP_FUNC_SQRTT_UD:
				case AXP_FUNC_SQRTF_SC:
				case AXP_FUNC_SQRTG_SC:
				case AXP_FUNC_SQRTF_S:
				case AXP_FUNC_SQRTG_S:
				case AXP_FUNC_SQRTF_SUC:
				case AXP_FUNC_SQRTS_SUC:
				case AXP_FUNC_SQRTG_SUC:
				case AXP_FUNC_SQRTT_SUC:
				case AXP_FUNC_SQRTS_SUM:
				case AXP_FUNC_SQRTT_SUM:
				case AXP_FUNC_SQRTF_SU:
				case AXP_FUNC_SQRTS_SU:
				case AXP_FUNC_SQRTG_SU:
				case AXP_FUNC_SQRTT_SU:
				case AXP_FUNC_SQRTS_SUD:
				case AXP_FUNC_SQRTT_SUD:
				case AXP_FUNC_SQRTS_SUIC:
				case AXP_FUNC_SQRTT_SUIC:
				case AXP_FUNC_SQRTS_SUIM:
				case AXP_FUNC_SQRTT_SUIM:
				case AXP_FUNC_SQRTS_SUI:
				case AXP_FUNC_SQRTT_SUI:
				case AXP_FUNC_SQRTS_SUID:
				case AXP_FUNC_SQRTT_SUID:
					retVal = FboxOther;
					break;

				default:
					break;
			}
			break;

		case FLTV:
			switch(func)
			{
				case AXP_FUNC_MULF_C:
				case AXP_FUNC_DIVF_C:
				case AXP_FUNC_MULG_C:
				case AXP_FUNC_DIVG_C:
				case AXP_FUNC_MULF:
				case AXP_FUNC_DIVF:
				case AXP_FUNC_MULG:
				case AXP_FUNC_DIVG:
				case AXP_FUNC_MULF_UC:
				case AXP_FUNC_DIVF_UC:
				case AXP_FUNC_MULG_UC:
				case AXP_FUNC_DIVG_UC:
				case AXP_FUNC_MULF_U:
				case AXP_FUNC_DIVF_U:
				case AXP_FUNC_MULG_U:
				case AXP_FUNC_DIVG_U:
				case AXP_FUNC_MULF_SC:
				case AXP_FUNC_DIVF_SC:
				case AXP_FUNC_MULG_SC:
				case AXP_FUNC_DIVG_SC:
				case AXP_FUNC_MULF_S:
				case AXP_FUNC_DIVF_S:
				case AXP_FUNC_MULG_S:
				case AXP_FUNC_DIVG_S:
				case AXP_FUNC_MULF_SUC:
				case AXP_FUNC_DIVF_SUC:
				case AXP_FUNC_MULG_SUC:
				case AXP_FUNC_DIVG_SUC:
				case AXP_FUNC_MULF_SU:
				case AXP_FUNC_DIVF_SU:
				case AXP_FUNC_MULG_SU:
				case AXP_FUNC_DIVG_SU:
					retVal = FboxMul;
					break;

				default:
					break;
			}
			break;

		case FLTI:
			switch(func)
			{
				case AXP_FUNC_MULS_C:
				case AXP_FUNC_DIVS_C:
				case AXP_FUNC_MULT_C:
				case AXP_FUNC_DIVT_C:
				case AXP_FUNC_MULS_M:
				case AXP_FUNC_DIVS_M:
				case AXP_FUNC_MULT_M:
				case AXP_FUNC_DIVT_M:
				case AXP_FUNC_MULS:
				case AXP_FUNC_DIVS:
				case AXP_FUNC_MULT:
				case AXP_FUNC_DIVT:
				case AXP_FUNC_MULS_D:
				case AXP_FUNC_DIVS_D:
				case AXP_FUNC_MULT_D:
				case AXP_FUNC_DIVT_D:
				case AXP_FUNC_MULS_UC:
				case AXP_FUNC_DIVS_UC:
				case AXP_FUNC_MULT_UC:
				case AXP_FUNC_DIVT_UC:
				case AXP_FUNC_MULS_UM:
				case AXP_FUNC_DIVS_UM:
				case AXP_FUNC_MULT_UM:
				case AXP_FUNC_DIVT_UM:
				case AXP_FUNC_MULS_U:
				case AXP_FUNC_DIVS_U:
				case AXP_FUNC_MULT_U:
				case AXP_FUNC_DIVT_U:
				case AXP_FUNC_MULS_UD:
				case AXP_FUNC_DIVS_UD:
				case AXP_FUNC_MULT_UD:
				case AXP_FUNC_DIVT_UD:
				case AXP_FUNC_MULS_SUC:
				case AXP_FUNC_DIVS_SUC:
				case AXP_FUNC_MULT_SUC:
				case AXP_FUNC_DIVT_SUC:
				case AXP_FUNC_MULS_SUM:
				case AXP_FUNC_DIVS_SUM:
				case AXP_FUNC_MULT_SUM:
				case AXP_FUNC_DIVT_SUM:
				case AXP_FUNC_MULS_SU:
				case AXP_FUNC_DIVS_SU:
				case AXP_FUNC_MULT_SU:
				case AXP_FUNC_DIVT_SU:
				case AXP_FUNC_MULS_SUD:
				case AXP_FUNC_DIVS_SUD:
				case AXP_FUNC_MULT_SUD:
				case AXP_FUNC_DIVT_SUD:
				case AXP_FUNC_MULS_SUIC:
				case AXP_FUNC_DIVS_SUIC:
				case AXP_FUNC_MULT_SUIC:
				case AXP_FUNC_DIVT_SUIC:
				case AXP_FUNC_MULS_SUIM:
				case AXP_FUNC_DIVS_SUIM:
				case AXP_FUNC_MULT_SUIM:
				case AXP_FUNC_MULS_SUI:
				case AXP_FUNC_DIVS_SUI:
				case AXP_FUNC_MULT_SUI:
				case AXP_FUNC_DIVT_SUI:
				case AXP_FUNC_MULS_SUID:
				case AXP_FUNC_DIVS_SUID:
				case AXP_FUNC_MULT_SUID:
					retVal = FboxMul;
					break;

				default:
					break;
			}
			break;

		case FLTL:
			switch(func)
			{
				case AXP_FUNC_CVTLQ:
				case AXP_FUNC_CVTQL:
				case AXP_FUNC_CVTQL_V:
				case AXP_FUNC_CVTQL_SV:
					break;

				default:
					retVal = FboxOther;
					break;
			}
			break;

		case MISC:
			switch(func)
			{
				case AXP_FUNC_MB:
				case AXP_FUNC_WMB:
				case AXP_FUNC_FETCH:
				case AXP_FUNC_FETCH_M:
				case AXP_FUNC_RPCC:
				case AXP_FUNC_ECB:
				case AXP_FUNC_RC:
				case AXP_FUNC_RS:
				case AXP_FUNC_WH64:
				case AXP_FUNC_WH64EN:
					retVal = EboxL1;
					break;

				default:
					break;
			}
			break;

		case FPTI:
			switch (func)
			{
				case AXP_FUNC_PERR:
				case AXP_FUNC_UNPKBW:
				case AXP_FUNC_UNPKBL:
				case AXP_FUNC_PKWB:
				case AXP_FUNC_PKLB:
				case AXP_FUNC_MINSB8:
				case AXP_FUNC_MINSW4:
				case AXP_FUNC_MINUB8:
				case AXP_FUNC_MINUW4:
				case AXP_FUNC_MAXUB8:
				case AXP_FUNC_MAXUW4:
				case AXP_FUNC_MAXSB8:
				case AXP_FUNC_MAXSW4:
					retVal = EboxU0;
					break;

				case AXP_FUNC_FTOIT:
				case AXP_FUNC_FTOIS:
					retVal = FboxOther;
					break;

				default:
					break;
			}
			break;

		case HW_MTPR:
		case HW_MFPR:
			retVal = hw_mxpr_pipe[func];
			break;

		default:
			break;
	}

	/*
	 * Return what we found to the caller.
	 */
	return(retVal);
}
