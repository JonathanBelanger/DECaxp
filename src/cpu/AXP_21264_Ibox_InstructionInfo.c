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
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"

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
		&&OP_CALL_PAL - &&OP_CALL_PAL,		/* OPCODE: 0x00 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x01 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x02 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x03 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x04 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x05 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x06 */
		&&RESERVED_OP - &&OP_CALL_PAL,		/* OPCODE: 0x07 */
		&&OP_LDA - &&OP_CALL_PAL,			/* OPCODE: 0x08 */
		&&OP_LDAH - &&OP_CALL_PAL,			/* OPCODE: 0x09 */
		&&OP_LDBU - &&OP_CALL_PAL,			/* OPCODE: 0x0a */
		&&OP_LDQ_U - &&OP_CALL_PAL,			/* OPCODE: 0x0b */
		&&OP_LDWU - &&OP_CALL_PAL,			/* OPCODE: 0x0c */
		&&OP_STW - &&OP_CALL_PAL,			/* OPCODE: 0x0d */
		&&OP_STB - &&OP_CALL_PAL,			/* OPCODE: 0x0e */
		&&OP_STQ_U - &&OP_CALL_PAL,			/* OPCODE: 0x0f */
		&&OP_ADDL - &&OP_CALL_PAL,			/* OPCODE: 0x10 */
		&&OP_AND - &&OP_CALL_PAL,			/* OPCODE: 0x11 */
		&&OP_MSKBL - &&OP_CALL_PAL,			/* OPCODE: 0x12 */
		&&OP_MULL - &&OP_CALL_PAL,			/* OPCODE: 0x13 */
		&&OP_ITOFS - &&OP_CALL_PAL,			/* OPCODE: 0x14 */
		&&OP_ADDF - &&OP_CALL_PAL,			/* OPCODE: 0x15 */
		&&OP_ADDS - &&OP_CALL_PAL,			/* OPCODE: 0x16 */
		&&OP_CVTLQ - &&OP_CALL_PAL,			/* OPCODE: 0x17 */
		&&OP_TRAPB - &&OP_CALL_PAL,			/* OPCODE: 0x18 */
		&&OP_HW_MFPR - &&OP_CALL_PAL,		/* OPCODE: 0x19 */
		&&OP_JMP - &&OP_CALL_PAL,			/* OPCODE: 0x1a */
		&&OP_HW_LD - &&OP_CALL_PAL,			/* OPCODE: 0x1b */
		&&OP_SEXTB - &&OP_CALL_PAL,			/* OPCODE: 0x1c */
		&&OP_HW_MTPR - &&OP_CALL_PAL,		/* OPCODE: 0x1d */
		&&OP_HW_RET - &&OP_CALL_PAL,		/* OPCODE: 0x1e */
		&&OP_HW_ST - &&OP_CALL_PAL,			/* OPCODE: 0x1f */
		&&OP_LDF - &&OP_CALL_PAL,			/* OPCODE: 0x20 */
		&&OP_LDG - &&OP_CALL_PAL,			/* OPCODE: 0x21 */
		&&OP_LDS - &&OP_CALL_PAL,			/* OPCODE: 0x22 */
		&&OP_LDT - &&OP_CALL_PAL,			/* OPCODE: 0x23 */
		&&OP_STF - &&OP_CALL_PAL,			/* OPCODE: 0x24 */
		&&OP_STG - &&OP_CALL_PAL,			/* OPCODE: 0x25 */
		&&OP_STS - &&OP_CALL_PAL,			/* OPCODE: 0x26 */
		&&OP_STT - &&OP_CALL_PAL,			/* OPCODE: 0x27 */
		&&OP_LDL - &&OP_CALL_PAL,			/* OPCODE: 0x28 */
		&&OP_LDQ - &&OP_CALL_PAL,			/* OPCODE: 0x29 */
		&&OP_LDL_L - &&OP_CALL_PAL,			/* OPCODE: 0x2a */
		&&OP_LDQ_L - &&OP_CALL_PAL,			/* OPCODE: 0x2b */
		&&OP_STL - &&OP_CALL_PAL,			/* OPCODE: 0x2c */
		&&OP_STQ - &&OP_CALL_PAL,			/* OPCODE: 0x2d */
		&&OP_STL_C - &&OP_CALL_PAL,			/* OPCODE: 0x2e */
		&&OP_STQ_C - &&OP_CALL_PAL,			/* OPCODE: 0x2f */
		&&OP_BR - &&OP_CALL_PAL,			/* OPCODE: 0x30 */
		&&OP_FBEQ - &&OP_CALL_PAL,			/* OPCODE: 0x31 */
		&&OP_FBLT - &&OP_CALL_PAL,			/* OPCODE: 0x32 */
		&&OP_FBLE - &&OP_CALL_PAL,			/* OPCODE: 0x33 */
		&&OP_BSR - &&OP_CALL_PAL,			/* OPCODE: 0x34 */
		&&OP_FBNE - &&OP_CALL_PAL,			/* OPCODE: 0x35 */
		&&OP_FBGE - &&OP_CALL_PAL,			/* OPCODE: 0x36 */
		&&OP_FBGT - &&OP_CALL_PAL,			/* OPCODE: 0x37 */
		&&OP_BLBC - &&OP_CALL_PAL,			/* OPCODE: 0x38 */
		&&OP_BEQ - &&OP_CALL_PAL,			/* OPCODE: 0x39 */
		&&OP_BLT - &&OP_CALL_PAL,			/* OPCODE: 0x3a */
		&&OP_BLE - &&OP_CALL_PAL,			/* OPCODE: 0x3b */
		&&OP_BLBS - &&OP_CALL_PAL,			/* OPCODE: 0x3c */
		&&OP_BNE - &&OP_CALL_PAL,			/* OPCODE: 0x3d */
		&&OP_BGE - &&OP_CALL_PAL,			/* OPCODE: 0x3e */
		&&OP_BGT - &&OP_CALL_PAL			/* OPCODE: 0x3f */
	};
	static const int	opcode14Label[] =
	{
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x0 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x1 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x2 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x3 */
		&&FUNC_ITOF - &&FUNC_ITOF,			/* FNC: 0x4 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x5 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x6 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x7 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x8 */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0x9 */
		&&FUNC_SQRTFG - &&FUNC_ITOF,		/* FNC: 0xa */
		&&FUNC_SQRTST - &&FUNC_ITOF,		/* FNC: 0xb */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0xc */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0xd */
		&&RESERVED_OP - &&FUNC_ITOF,		/* FNC: 0xe */
		&&RESERVED_OP - &&FUNC_ITOF			/* FNC: 0xf */
	};
	static const int	opcode15Label[] =
	{
		&&FUNC_VAX_ADD - &&FUNC_VAX_ADD,	/* FNC: 0x0 */
		&&FUNC_VAX_SUB - &&FUNC_VAX_ADD,	/* FNC: 0x1 */
		&&FUNC_VAX_MUL - &&FUNC_VAX_ADD,	/* FNC: 0x2 */
		&&FUNC_VAX_DIV - &&FUNC_VAX_ADD,	/* FNC: 0x3 */
		&&RESERVED_OP - &&FUNC_VAX_ADD,		/* FNC: 0x4 */
		&&FUNC_VAX_CMPEQ - &&FUNC_VAX_ADD,	/* FNC: 0x5 */
		&&FUNC_VAX_CMPLT - &&FUNC_VAX_ADD,	/* FNC: 0x6 */
		&&FUNC_VAX_CMPLE - &&FUNC_VAX_ADD,	/* FNC: 0x7 */
		&&RESERVED_OP - &&FUNC_VAX_ADD,		/* FNC: 0x8 */
		&&RESERVED_OP - &&FUNC_VAX_ADD,		/* FNC: 0x9 */
		&&RESERVED_OP - &&FUNC_VAX_ADD,		/* FNC: 0xa */
		&&RESERVED_OP - &&FUNC_VAX_ADD,		/* FNC: 0xb */
		&&FUNC_VAX_CVTF - &&FUNC_VAX_ADD,	/* FNC: 0xc */
		&&FUNC_VAX_CVTD - &&FUNC_VAX_ADD,	/* FNC: 0xd */
		&&FUNC_VAX_CVTG - &&FUNC_VAX_ADD,	/* FNC: 0xe */
		&&FUNC_VAX_CVTQ - &&FUNC_VAX_ADD	/* FNC: 0xf */
	};
	static const int	opcode16Label[] =
	{
		&&FUNC_IEEE_ADD - &&FUNC_IEEE_ADD,	/* FNC: 0x0 */
		&&FUNC_IEEE_SUB - &&FUNC_IEEE_ADD,	/* FNC: 0x1 */
		&&FUNC_IEEE_MUL - &&FUNC_IEEE_ADD,	/* FNC: 0x2 */
		&&FUNC_IEEE_DIV - &&FUNC_IEEE_ADD,	/* FNC: 0x3 */
		&&FUNC_IEEE_CMPUN - &&FUNC_IEEE_ADD,/* FNC: 0x4 */
		&&FUNC_IEEE_CMPEQ - &&FUNC_IEEE_ADD,/* FNC: 0x5 */
		&&FUNC_IEEE_CMPLT - &&FUNC_IEEE_ADD,/* FNC: 0x6 */
		&&FUNC_IEEE_CMPLE - &&FUNC_IEEE_ADD,/* FNC: 0x7 */
		&&RESERVED_OP - &&FUNC_IEEE_ADD,	/* FNC: 0x8 */
		&&RESERVED_OP - &&FUNC_IEEE_ADD,	/* FNC: 0x9 */
		&&RESERVED_OP - &&FUNC_IEEE_ADD,	/* FNC: 0xa */
		&&RESERVED_OP - &&FUNC_IEEE_ADD,	/* FNC: 0xb */
		&&FUNC_IEEE_CVTS - &&FUNC_IEEE_ADD,	/* FNC: 0xc */
		&&RESERVED_OP - &&FUNC_IEEE_ADD,	/* FNC: 0xd */
		&&FUNC_IEEE_CVTT - &&FUNC_IEEE_ADD,	/* FNC: 0xe */
		&&FUNC_IEEE_CVTQ - &&FUNC_IEEE_ADD	/* FNC: 0xf */
	};
	AXP_FP_FUNC	fpFunc;

	goto *(&&OP_CALL_PAL + label[instr->opcode]);

OP_CALL_PAL:	/* OPCODE: 0x00 */
	instr->excRegMask = AXP_CALL_PAL(cpu, instr);
	goto COMPLETION_OP;

OP_LDA:			/* OPCODE: 0x08 */
	instr->excRegMask = AXP_LDA(cpu, instr);
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_LDAH:		/* OPCODE: 0x09 */
	instr->excRegMask = AXP_LDAH(cpu, instr);
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_LDBU:		/* OPCODE: 0x0a */
	instr->excRegMask = AXP_LDBU(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ_U:		/* OPCODE: 0x0b */
	instr->excRegMask = AXP_LDQ_U(cpu, instr);
	goto COMPLETION_OP;

OP_LDWU:		/* OPCODE: 0x0c */
	instr->excRegMask = AXP_LDWU(cpu, instr);
	goto COMPLETION_OP;

OP_STW:			/* OPCODE: 0x0d */
	instr->excRegMask = AXP_STW(cpu, instr);
	goto COMPLETION_OP;

OP_STB:			/* OPCODE: 0x0e */
	instr->excRegMask = AXP_STB(cpu, instr);
	goto COMPLETION_OP;

OP_STQ_U:		/* OPCODE: 0x0f */
	instr->excRegMask = AXP_STQ_U(cpu, instr);
	goto COMPLETION_OP;

OP_ADDL:		/* OPCODE: 0x10 */
	switch (instr->function)
	{
		case AXP_FUNC_ADDL:
			instr->excRegMask = AXP_ADDL(cpu, instr);
			break;

		case AXP_FUNC_S4ADDL:
			instr->excRegMask = AXP_S4ADDL(cpu, instr);
			break;

		case AXP_FUNC_SUBL:
			instr->excRegMask = AXP_SUBL(cpu, instr);
			break;

		case AXP_FUNC_S4SUBL:
			instr->excRegMask = AXP_S4SUBL(cpu, instr);
			break;

		case AXP_FUNC_CMPBGE:
			instr->excRegMask = AXP_CMPBGE(cpu, instr);
			break;

		case AXP_FUNC_S8ADDL:
			instr->excRegMask = AXP_S8ADDL(cpu, instr);
			break;

		case AXP_FUNC_S8SUBL:
			instr->excRegMask = AXP_S8SUBL(cpu, instr);
			break;

		case AXP_FUNC_CMPULT:
			instr->excRegMask = AXP_CMPULT(cpu, instr);
			break;

		case AXP_FUNC_ADDQ:
			instr->excRegMask = AXP_ADDQ(cpu, instr);
			break;

		case AXP_FUNC_S4ADDQ:
			instr->excRegMask = AXP_S4ADDQ(cpu, instr);
			break;

		case AXP_FUNC_SUBQ:
			instr->excRegMask = AXP_SUBQ(cpu, instr);
			break;

		case AXP_FUNC_S4SUBQ:
			instr->excRegMask = AXP_S4SUBQ(cpu, instr);
			break;

		case AXP_FUNC_CMPEQ:
			instr->excRegMask = AXP_CMPEQ(cpu, instr);
			break;

		case AXP_FUNC_S8ADDQ:
			instr->excRegMask = AXP_S8ADDQ(cpu, instr);
			break;

		case AXP_FUNC_S8SUBQ:
			instr->excRegMask = AXP_S8SUBQ(cpu, instr);
			break;

		case AXP_FUNC_CMPULE:
			instr->excRegMask = AXP_CMPULE(cpu, instr);
			break;

		case AXP_FUNC_ADDL_V:
			instr->excRegMask = AXP_ADDL_V(cpu, instr);
			break;

		case AXP_FUNC_SUBL_V:
			instr->excRegMask = AXP_SUBL_V(cpu, instr);
			break;

		case AXP_FUNC_CMPLT:
			instr->excRegMask = AXP_CMPLT(cpu, instr);
			break;

		case AXP_FUNC_ADDQ_V:
			instr->excRegMask = AXP_ADDQ_V(cpu, instr);
			break;

		case AXP_FUNC_SUBQ_V:
			instr->excRegMask = AXP_SUBQ_V(cpu, instr);
			break;

		case AXP_FUNC_CMPLE:
			instr->excRegMask = AXP_CMPLE(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_AND:			/* OPCODE: 0x11 */
	switch (instr->function)
	{
		case AXP_FUNC_AND:
			instr->excRegMask = AXP_AND(cpu, instr);
			break;

		case AXP_FUNC_BIC:
			instr->excRegMask = AXP_BIC(cpu, instr);
			break;

		case AXP_FUNC_CMOVLBS:
			instr->excRegMask = AXP_CMOVLBS(cpu, instr);
			break;

		case AXP_FUNC_CMOVLBC:
			instr->excRegMask = AXP_CMOVLBC(cpu, instr);
			break;

		case AXP_FUNC_BIS:
			instr->excRegMask = AXP_BIS(cpu, instr);
			break;

		case AXP_FUNC_CMOVEQ:
			instr->excRegMask = AXP_CMOVEQ(cpu, instr);
			break;

		case AXP_FUNC_CMOVNE:
			instr->excRegMask = AXP_CMOVNE(cpu, instr);
			break;

		case AXP_FUNC_ORNOT:
			instr->excRegMask = AXP_ORNOT(cpu, instr);
			break;

		case AXP_FUNC_XOR:
			instr->excRegMask = AXP_XOR(cpu, instr);
			break;

		case AXP_FUNC_CMOVLT:
			instr->excRegMask = AXP_CMOVLT(cpu, instr);
			break;

		case AXP_FUNC_CMOVGE:
			instr->excRegMask = AXP_CMOVGE(cpu, instr);
			break;

		case AXP_FUNC_EQV:
			instr->excRegMask = AXP_EQV(cpu, instr);
			break;

		case AXP_FUNC_AMASK:
			instr->excRegMask = AXP_AMASK(cpu, instr);
			break;

		case AXP_FUNC_CMOVLE:
			instr->excRegMask = AXP_CMOVLE(cpu, instr);
			break;

		case AXP_FUNC_CMOVGT:
			instr->excRegMask = AXP_CMOVGT(cpu, instr);
			break;

		case AXP_FUNC_IMPLVER:
			instr->excRegMask = AXP_IMPLVER(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_MSKBL:		/* OPCODE: 0x11 */
	switch (instr->function)
	{
		case AXP_FUNC_MSKBL:
			instr->excRegMask = AXP_MSKBL(cpu, instr);
			break;

		case AXP_FUNC_EXTBL:
			instr->excRegMask = AXP_EXTBL(cpu, instr);
			break;

		case AXP_FUNC_INSBL:
			instr->excRegMask = AXP_INSBL(cpu, instr);
			break;

		case AXP_FUNC_MSKWL:
			instr->excRegMask = AXP_MSKWL(cpu, instr);
			break;

		case AXP_FUNC_EXTWL:
			instr->excRegMask = AXP_EXTWL(cpu, instr);
			break;

		case AXP_FUNC_INSWL:
			instr->excRegMask = AXP_INSWL(cpu, instr);
			break;

		case AXP_FUNC_MSKLL:
			instr->excRegMask = AXP_MSKLL(cpu, instr);
			break;

		case AXP_FUNC_EXTLL:
			instr->excRegMask = AXP_EXTLL(cpu, instr);
			break;

		case AXP_FUNC_INSLL:
			instr->excRegMask = AXP_INSLL(cpu, instr);
			break;

		case AXP_FUNC_ZAP:
			instr->excRegMask = AXP_ZAP(cpu, instr);
			break;

		case AXP_FUNC_ZAPNOT:
			instr->excRegMask = AXP_ZAPNOT(cpu, instr);
			break;

		case AXP_FUNC_MSKQL:
			instr->excRegMask = AXP_MSKQL(cpu, instr);
			break;

		case AXP_FUNC_SRL:
			instr->excRegMask = AXP_SRL(cpu, instr);
			break;

		case AXP_FUNC_EXTQL:
			instr->excRegMask = AXP_EXTQL(cpu, instr);
			break;

		case AXP_FUNC_SLL:
			instr->excRegMask = AXP_SLL(cpu, instr);
			break;

		case AXP_FUNC_INSQL:
			instr->excRegMask = AXP_INSQL(cpu, instr);
			break;

		case AXP_FUNC_SRA:
			instr->excRegMask = AXP_SRA(cpu, instr);
			break;

		case AXP_FUNC_MSKWH:
			instr->excRegMask = AXP_MSKWH(cpu, instr);
			break;

		case AXP_FUNC_INSWH:
			instr->excRegMask = AXP_INSWH(cpu, instr);
			break;

		case AXP_FUNC_EXTWH:
			instr->excRegMask = AXP_EXTWH(cpu, instr);
			break;

		case AXP_FUNC_MSKLH:
			instr->excRegMask = AXP_MSKLH(cpu, instr);
			break;

		case AXP_FUNC_INSLH:
			instr->excRegMask = AXP_INSLH(cpu, instr);
			break;

		case AXP_FUNC_EXTLH:
			instr->excRegMask = AXP_EXTLH(cpu, instr);
			break;

		case AXP_FUNC_MSKQH:
			instr->excRegMask = AXP_MSKQH(cpu, instr);
			break;

		case AXP_FUNC_INSQH:
			instr->excRegMask = AXP_INSQH(cpu, instr);
			break;

		case AXP_FUNC_EXTQH:
			instr->excRegMask = AXP_EXTQH(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_MULL:		/* OPCODE: 0x13 */
	switch (instr->function)
	{
		case AXP_FUNC_MULL:
			instr->excRegMask = AXP_MULL(cpu, instr);
			break;

		case AXP_FUNC_MULQ:
			instr->excRegMask = AXP_MULQ(cpu, instr);
			break;

		case AXP_FUNC_UMULH:
			instr->excRegMask = AXP_UMULH(cpu, instr);
			break;

		case AXP_FUNC_MULL_V:
			instr->excRegMask = AXP_MULL_V(cpu, instr);
			break;

		case AXP_FUNC_MULQ_V:
			instr->excRegMask = AXP_MULQ_V(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_ITOFS:		/* OPCODE: 0x14 */
	fpFunc = *(AXP_FP_FUNC *) &instr->function;
	goto *(&&FUNC_ITOF + opcode14Label[fpFunc.fnc]);

FUNC_ITOF:		/* FNC: 0x4 */
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_ITOFS(cpu, instr);
				break;

			case AXP_FP14_F:
				instr->excRegMask = AXP_ITOFF(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_ITOFT(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_SQRTFG:	/* FNC: 0xa */
		switch (fpFunc.src)
		{
			case AXP_FP14_F:
				instr->excRegMask = AXP_SQRTF(cpu, instr);
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_SQRTG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_SQRTST:	/* FNC: 0xb */
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_SQRTS(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_SQRTT(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

OP_ADDF:		/* OPCODE: 0x15 */
	fpFunc = *(AXP_FP_FUNC *) &instr->function;
	goto *(&&FUNC_VAX_ADD + opcode15Label[fpFunc.fnc]);

FUNC_VAX_ADD:
		switch (fpFunc.src)
		{
			case AXP_FP_F:
				instr->excRegMask = AXP_ADDF(cpu, instr);
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_ADDG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_SUB:
		switch (fpFunc.src)
		{
			case AXP_FP_F:
				instr->excRegMask = AXP_SUBF(cpu, instr);
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_SUBG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_MUL:
		switch (fpFunc.src)
		{
			case AXP_FP_F:
				instr->excRegMask = AXP_MULF(cpu, instr);
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_MULG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_DIV:
		switch (fpFunc.src)
		{
			case AXP_FP_F:
				instr->excRegMask = AXP_DIVF(cpu, instr);
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_DIVG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_CMPEQ:
		if (fpFunc.src == AXP_FP_G)
		{
			instr->excRegMask = AXP_CMPGEQ(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_VAX_CMPLT:
		switch (fpFunc.src)
		{
			case AXP_FP_F:
				break;

			case AXP_FP_G:
				instr->excRegMask = AXP_CMPGLT(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_CMPLE:
		if (fpFunc.src== AXP_FP_G)
		{
			instr->excRegMask = AXP_CMPGLE(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_VAX_CVTF:
		switch (fpFunc.src)
		{
			case AXP_FP_G:
				instr->excRegMask = AXP_CVTGF(cpu, instr);
				break;

			case AXP_FP_Q:
				instr->excRegMask = AXP_CVTQF(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_CVTD:
		if (fpFunc.src == AXP_FP_G)
		{
			instr->excRegMask = AXP_CVTGD(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_VAX_CVTG:
		switch (fpFunc.src)
		{
			case AXP_FP_D:
				instr->excRegMask = AXP_CVTDG(cpu, instr);
				break;

			case AXP_FP_Q:
				instr->excRegMask = AXP_CVTQG(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_VAX_CVTQ:
		if (fpFunc.src == AXP_FP_G)
		{
			instr->excRegMask = AXP_CVTGQ(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

OP_ADDS:		/* OPCODE: 0x16 */
	fpFunc = *(AXP_FP_FUNC *) &instr->function;
	goto *(&&FUNC_IEEE_ADD + opcode16Label[fpFunc.fnc]);

FUNC_IEEE_ADD:
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_ADDS(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_ADDT(cpu, instr);
				break;

			case AXP_FP_Q:
			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_SUB:
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_SUBS(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_SUBT(cpu, instr);
				break;

			case AXP_FP_Q:
			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_MUL:
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_MULS(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_MULT(cpu, instr);
				break;

			case AXP_FP_Q:
			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_DIV:
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_DIVS(cpu, instr);
				break;

			case AXP_FP_T:
				instr->excRegMask = AXP_DIVT(cpu, instr);
				break;

			case AXP_FP_Q:
			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_CMPUN:
		if (fpFunc.src == AXP_FP_T)
		{
			instr->excRegMask = AXP_CMPTUN(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_IEEE_CMPEQ:
		if (fpFunc.src == AXP_FP_T)
		{
			instr->excRegMask = AXP_CMPTEQ(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_IEEE_CMPLT:
		if (fpFunc.src == AXP_FP_T)
		{
			instr->excRegMask = AXP_CMPTLT(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_IEEE_CMPLE:
		if (fpFunc.src == AXP_FP_T)
		{
			instr->excRegMask = AXP_CMPTLE(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

FUNC_IEEE_CVTS:
		switch (fpFunc.src)
		{
			case AXP_FP_T:
				instr->excRegMask = AXP_CVTTS(cpu, instr);
				break;

			case AXP_FP_Q:
				instr->excRegMask = AXP_CVTQS(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_CVTT:
		switch (fpFunc.src)
		{
			case AXP_FP_S:
				instr->excRegMask = AXP_CVTST(cpu, instr);
				break;

			case AXP_FP_Q:
				instr->excRegMask = AXP_CVTQT(cpu, instr);
				break;

			default:
				goto RESERVED_OP;
				break;
		}

		/*
		 * These instructions complete immediately.
		 */
		instr->state = WaitingRetirement;
		goto COMPLETION_OP;

FUNC_IEEE_CVTQ:
		if (fpFunc.src == AXP_FP_T)
		{
			instr->excRegMask = AXP_CVTTQ(cpu, instr);

			/*
			 * This instruction completes immediately.
			 */
			instr->state = WaitingRetirement;
			goto COMPLETION_OP;
		}
		goto RESERVED_OP;

OP_CVTLQ:		/* OPCODE: 0x17 */
	switch (instr->function)
	{
		case AXP_FUNC_CVTLQ:
			instr->excRegMask = AXP_CVTLQ(cpu, instr);
			break;

		case AXP_FUNC_CPYS:
			instr->excRegMask = AXP_CPYS(cpu, instr);
			break;

		case AXP_FUNC_CPYSN:
			instr->excRegMask = AXP_CPYSN(cpu, instr);
			break;

		case AXP_FUNC_CPYSE:
			instr->excRegMask = AXP_CPYSE(cpu, instr);
			break;

		case AXP_FUNC_MT_FPCR:
			instr->excRegMask = AXP_MT_FPCR(cpu, instr);
			break;

		case AXP_FUNC_MF_FPCR:
			instr->excRegMask = AXP_MF_FPCR(cpu, instr);
			break;

		case AXP_FUNC_FCMOVEQ:
			instr->excRegMask = AXP_FCMOVEQ(cpu, instr);
			break;

		case AXP_FUNC_FCMOVNE:
			instr->excRegMask = AXP_FCMOVNE(cpu, instr);
			break;

		case AXP_FUNC_FCMOVLT:
			instr->excRegMask = AXP_FCMOVLT(cpu, instr);
			break;

		case AXP_FUNC_FCMOVGE:
			instr->excRegMask = AXP_FCMOVGE(cpu, instr);
			break;

		case AXP_FUNC_FCMOVLE:
			instr->excRegMask = AXP_FCMOVLE(cpu, instr);
			break;

		case AXP_FUNC_FCMOVGT:
			instr->excRegMask = AXP_FCMOVGT(cpu, instr);
			break;

		case AXP_FUNC_CVTQL:
		case AXP_FUNC_CVTQL_V:
		case AXP_FUNC_CVTQL_SV:
			instr->excRegMask = AXP_CVTQL(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_TRAPB:		/* OPCODE: 0x18 */
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

OP_HW_MFPR:		/* OPCODE: 0x19 */
	if ((instr->pc.pal != AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWMFPR(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_JMP:			/* OPCODE: 0x1a */
	instr->excRegMask = AXP_JMP(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_HW_LD:		/* OPCODE: 0x1b */
	if ((instr->pc.pal != AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWLD(cpu, instr);
	goto COMPLETION_OP;

OP_SEXTB:		/* OPCODE: 0x1c */
	switch (instr->function)
	{
		case AXP_FUNC_SEXTB:
			instr->excRegMask = AXP_SEXTB(cpu, instr);
			break;

		case AXP_FUNC_SEXTW:
			instr->excRegMask = AXP_SEXTW(cpu, instr);
			break;

		case AXP_FUNC_CTPOP:
			instr->excRegMask = AXP_CTPOP(cpu, instr);
			break;

		case AXP_FUNC_PERR:
			instr->excRegMask = AXP_PERR(cpu, instr);
			break;

		case AXP_FUNC_CTLZ:
			instr->excRegMask = AXP_CTLZ(cpu, instr);
			break;

		case AXP_FUNC_CTTZ:
			instr->excRegMask = AXP_CTTZ(cpu, instr);
			break;

		case AXP_FUNC_UNPKBW:
			instr->excRegMask = AXP_UNPKBW(cpu, instr);
			break;

		case AXP_FUNC_UNPKBL:
			instr->excRegMask = AXP_UNPKBL(cpu, instr);
			break;

		case AXP_FUNC_PKWB:
			instr->excRegMask = AXP_PKWB(cpu, instr);
			break;

		case AXP_FUNC_PKLB:
			instr->excRegMask = AXP_PKLB(cpu, instr);
			break;

		case AXP_FUNC_MINSB8:
			instr->excRegMask = AXP_MINSB8(cpu, instr);
			break;

		case AXP_FUNC_MINSW4:
			instr->excRegMask = AXP_MINSW4(cpu, instr);
			break;

		case AXP_FUNC_MINUB8:
			instr->excRegMask = AXP_MINUB8(cpu, instr);
			break;

		case AXP_FUNC_MINUW4:
			instr->excRegMask = AXP_MINUW4(cpu, instr);
			break;

		case AXP_FUNC_MAXUB8:
			instr->excRegMask = AXP_MAXUB8(cpu, instr);
			break;

		case AXP_FUNC_MAXUW4:
			instr->excRegMask = AXP_MAXUW4(cpu, instr);
			break;

		case AXP_FUNC_MAXSB8:
			instr->excRegMask = AXP_MAXSB8(cpu, instr);
			break;

		case AXP_FUNC_MAXSW4:
			instr->excRegMask = AXP_MAXSW4(cpu, instr);
			break;

		case AXP_FUNC_FTOIT:
			instr->excRegMask = AXP_FTOIT(cpu, instr);
			break;

		case AXP_FUNC_FTOIS:
			instr->excRegMask = AXP_FTOIS(cpu, instr);
			break;

		default:
			goto RESERVED_OP;
			break;
	}

	/*
	 * These instructions complete immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_HW_MTPR:		/* OPCODE: 0x1d */
	if ((instr->pc.pal != AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWMTPR(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_HW_RET:		/* OPCODE: 0x1e */
	if ((instr->pc.pal != AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWRET(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_HW_ST:		/* OPCODE: 0x1f */
	if ((instr->pc.pal != AXP_PAL_MODE) && (cpu->iCtl.hwe != 1))
		goto RESERVED_OP;
	instr->excRegMask = AXP_HWST(cpu, instr);
	goto COMPLETION_OP;

OP_LDF:			/* OPCODE: 0x20 */
	instr->excRegMask = AXP_LDF(cpu, instr);
	goto COMPLETION_OP;

OP_LDG:			/* OPCODE: 0x21 */
	instr->excRegMask = AXP_LDG(cpu, instr);
	goto COMPLETION_OP;

OP_LDS:			/* OPCODE: 0x22 */
	instr->excRegMask = AXP_LDS(cpu, instr);
	goto COMPLETION_OP;

OP_LDT:			/* OPCODE: 0x23 */
	instr->excRegMask = AXP_LDT(cpu, instr);
	goto COMPLETION_OP;

OP_STF:			/* OPCODE: 0x24 */
	instr->excRegMask = AXP_STF(cpu, instr);
	goto COMPLETION_OP;

OP_STG:			/* OPCODE: 0x25 */
	instr->excRegMask = AXP_STG(cpu, instr);
	goto COMPLETION_OP;

OP_STS:			/* OPCODE: 0x26 */
	instr->excRegMask = AXP_STS(cpu, instr);
	goto COMPLETION_OP;

OP_STT:			/* OPCODE: 0x27 */
	instr->excRegMask = AXP_STT(cpu, instr);
	goto COMPLETION_OP;

OP_LDL:			/* OPCODE: 0x28 */
	instr->excRegMask = AXP_LDL(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ:			/* OPCODE: 0x29 */
	instr->excRegMask = AXP_LDQ(cpu, instr);
	goto COMPLETION_OP;

OP_LDL_L:		/* OPCODE: 0x2a */
	instr->excRegMask = AXP_LDL_L(cpu, instr);
	goto COMPLETION_OP;

OP_LDQ_L:		/* OPCODE: 0x2b */
	instr->excRegMask = AXP_LDQ_L(cpu, instr);
	goto COMPLETION_OP;

OP_STL:			/* OPCODE: 0x2c */
	instr->excRegMask = AXP_STL(cpu, instr);
	goto COMPLETION_OP;

OP_STQ:			/* OPCODE: 0x2d */
	instr->excRegMask = AXP_STQ(cpu, instr);
	goto COMPLETION_OP;

OP_STL_C:		/* OPCODE: 0x2e */
	instr->excRegMask = AXP_STL_C(cpu, instr);
	goto COMPLETION_OP;

OP_STQ_C:		/* OPCODE: 0x2f */
	instr->excRegMask = AXP_STQ_C(cpu, instr);
	goto COMPLETION_OP;

OP_BR:			/* OPCODE: 0x30 */
	instr->excRegMask = AXP_BR(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBEQ:		/* OPCODE: 0x31 */
	instr->excRegMask = AXP_FBEQ(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBLT:		/* OPCODE: 0x32 */
	instr->excRegMask = AXP_FBLT(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBLE:		/* OPCODE: 0x33 */
	instr->excRegMask = AXP_FBLE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BSR:			/* OPCODE: 0x34 */
	instr->excRegMask = AXP_BSR(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBNE:		/* OPCODE: 0x35 */
	instr->excRegMask = AXP_FBNE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBGE:		/* OPCODE: 0x36 */
	instr->excRegMask = AXP_FBGE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_FBGT:		/* OPCODE: 0x37 */
	instr->excRegMask = AXP_FBGT(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BLBC:		/* OPCODE: 0x38 */
	instr->excRegMask = AXP_BLBC(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BEQ:			/* OPCODE: 0x39 */
	instr->excRegMask = AXP_BEQ(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BLT:			/* OPCODE: 0x3a */
	instr->excRegMask = AXP_BLT(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BLE:			/* OPCODE: 0x3b */
	instr->excRegMask = AXP_BLE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BLBS:		/* OPCODE: 0x3c */
	instr->excRegMask = AXP_BLBS(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BNE:			/* OPCODE: 0x3d */
	instr->excRegMask = AXP_BNE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BGE:			/* OPCODE: 0x3e */
	instr->excRegMask = AXP_BGE(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

OP_BGT:			/* OPCODE: 0x3f */
	instr->excRegMask = AXP_BGT(cpu, instr);

	/*
	 * This instruction completes immediately.
	 */
	instr->state = WaitingRetirement;
	goto COMPLETION_OP;

RESERVED_OP:
	instr->excRegMask = IllegalOperand;
	AXP_21264_Ibox_Event(
					cpu,
					AXP_OPCDEC,
					instr->pc,
					*(u64 *) &instr->pc,
					instr->opcode,
					AXP_UNMAPPED_REG,
					false,
					false);
	instr->state = WaitingRetirement;

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
	if (inst.pal.opcode <= 0x3f)
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
	if (opcode <= 0x3f)
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
	if (opcode <= 0x3f)
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
	if (opcode <= 0x3f)
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
