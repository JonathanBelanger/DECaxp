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
};

static struct instructDecode insDecode[] =
{
/* Format	Type	Registers   									Opcode	Mnemonic	Description 					*/
	{Pcd,	Branch,	{ .raw = 0}, AXP_IQ},							/* 00		CALL_PAL	Trap to PALcode				*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 01					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 02					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 03					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 04					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 05					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 06					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, AXP_NONE},							/* 07					Reserved for Digital		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 08		LDA			Load address				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 09		LDAH		Load address high			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 0A		LDBU		Load zero-extended byte		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 0B		LDQ_U		Load unaligned quadword		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 0C		LDWU		Load zero-extended word		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 0D		STW			Store word					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 0E		STB			Store byte					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 0F		STQ_U		Store unaligned quadword	*/
	{Opr,	Other,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},/*10	ADDL		Add longword				*/
	{Opr,	Other,	{ .raw = AXP_OPCODE_11}, AXP_IQ},				/* 11		AND			Logical product				*/
	{Opr,	Logic,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},/*12	MSKBL		Mask byte low				*/
	{Opr,	Oper,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},/*13	MULL		Multiply longword			*/
	{FP,	Arith,	{ .raw = AXP_OPCODE_14}, AXP_COND},				/* 14		ITOFS		Int to float move, S_float	*/
	{FP,	Other,	{ .raw = AXP_OPCODE_15}, AXP_FQ},				/* 15		ADDF		Add F_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_16}, AXP_FQ},				/* 16		ADDS		Add S_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_17}, AXP_FQ},				/* 17		CVTLQ		Convert longword to quad	*/
	{Mfc,	Other,	{ .raw = AXP_OPCODE_18}, AXP_IQ},				/* 18		TRAPB		Trap barrier				*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA}, AXP_IQ},					/* 19		HW_MFPR		Reserved for PALcode		*/
	{Mbr,	Branch,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 1A		JMP			Jump						*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 1B		HW_LD		Reserved for PALcode		*/
	{Cond,	Arith,	{ .raw = AXP_OPCODE_1C}, AXP_COND},				/* 1C		SEXTB		Sign extend byte			*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RB}, AXP_IQ},					/* 1D		HW_MTPR		Reserved for PALcode		*/
	{PAL,	Branch,	{ .raw = AXP_SRC1_RB}, AXP_IQ},					/* 1E		HW_RET		Reserved for PALcode		*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 1F		HW_ST		Reserved for PALcode		*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ},		/* 20 		LDF			Load F_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ},		/* 21		LDG			Load G_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ},		/* 22		LDS			Load S_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, AXP_IQ},		/* 23		LDT			Load T_floating				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ},		/* 24		STF			Store F_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ},		/* 25		STG			Store G_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ},		/* 26		STS			Store S_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, AXP_FQ},		/* 27		STT			Store T_floating			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 28		LDL			Load sign-extended long		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 29		LDQ			Load quadword				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 2A		LDL_L		Load sign-extend long lock	*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, AXP_IQ},		/* 2B		LDQ_L		Load quadword locked		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 2C		STL			Store longword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 2D		STQ			Store quadword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 2E		STL_C		Store longword conditional	*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, AXP_IQ},		/* 2F		STQ_C		Store quadword conditional	*/
	{Bra,	Branch,	{ .raw = AXP_DEST_RA}, AXP_IQ},					/* 30		BR			Unconditional branch		*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 31		FBEQ		Floating branch if = zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 32		FBLT		Floating branch if < zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 33		FBLE		Floating branch if <= zero	*/
	{Mbr,	Branch,	{ .raw = AXP_DEST_RA}, AXP_IQ},					/* 34		BSR			Branch to subroutine		*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 35		FBNE		Floating branch if != zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 36		FBGE		Floating branch if >=zero	*/
	{FPBra,	Branch,	{ .raw = AXP_SRC1_FA}, AXP_FQ},					/* 37		FBGT		Floating branch if > zero	*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 38		BLBC		Branch if low bit clear		*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 39		BEQ			Branch if = zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 3A		BLT			Branch if < zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 3B		BLE			Branch if <= zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 3C		BLBS		Branch if low bit set		*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 3D		BNE			Branch if != zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ},					/* 3E		BGE			Branch if >= zero			*/
	{Bra,	Branch,	{ .raw = AXP_SRC1_RA}, AXP_IQ}					/* 3F		BGT			Branch if > zero			*/
};

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
		 * values, depending upon the funciton code.  If the function code is
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
