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
 *	This source file contains the functions needed to implement the
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Cleaned up some compiler errors.  Ran some tests, and corrected some coding
 *	mistakes.  The prediction code correctly predicts a branch instruction
 *	between 95.0% and 99.1% of the time.
 *
 *	V01.002		20-May-2017	Jonathan D. Belanger
 *	Did a bit of reorganization.  This module will contain all the
 *	functionality that is described as being part of the Ibox.  I move the
 *	prediction code here.
 *
 *	V01.003		22-May-2017	Jonathan D. Belanger
 *	Moved the main() function out of this module and into its own.
 *
 *	V01.004		24-May-2017	Jonathan D. Belanger
 *	Added some instruction cache (icache) code and definitions into this code.
 *	The iCache structure should be part of the CPU structure and the iCache
 *	code should not be adding a cache entry on a miss.  There should be a
 *	separate fill cache function.  Also, the iCache look-up code should return
 *	the requested instructions on a hit.
 *
 *	V01.005		01-Jun-2017	Jonathan D. Belanger
 *	Added a function to add an Icache line/block.  There are a couple to do
 *	items for ASM and ASN in this function.
 *
 *	V01.006		11-Jun-2017	Jonathan D. Belanger
 *	Started working on the instruction decode and register renaming code.
 *
 *	V01.007		12-Jun-2017	Jonathan D. Belanger
 *	Forogot to include the generation of a unique ID (8-bits long) to associate
 *	with each decoded instruction.
 *	BTW: We need a better way to decode an instruction.  The decoded
 *	instruction does not require the architectural register number, so we
 *	should not save them.  Also, renaming can be done while we are decoding the
 *	instruction.  We are also going to have to special calse some instructions,
 *	such as PAL instructions, CMOVE, and there are exceptions for instruction
 *	types (Ra is a destination register for a Load, but a source register for a
 *	store.
 *
 *	V01.008		15-Jun-2017	Jonathan D. Belanger
 *	Finished defining structure array to be able to normalize how a register is
 *	used in an instruction.  Generally speaking, register 'c' (Rc or Fc) is a
 *	destination register.  In load operations, register 'a' (Ra or Fa) is the
 *	destination register.  Register 'b' (Rb or Fb) is never used as a
 *	desination (through I have implemented provisions for this).
 *	Unforunately, sometimes a register is not used, or 'b' may be source 1 and
 *	other times it is source 2.  Sometimes not register is utilized (e.g. MB).
 *	The array structures assist in these differences so that in the end we end
 *	up with a destingation (not not), a source 1 (or not), and/or a source 2
 *	(or not) set of registers.  If we have a destination register, then
 *	register renaming needs to allocate a new register.  IF we have source
 *	register, then renaming needs to assocaite the current register mapping to
 *	thos register.
 *
 *	V01.009		16-Jun-2017	Jonathan D. Belanger
 *	Finished the implementation of normalizing registers, as indicated above,
 *	and then mapping them from architectural to physical registers, generating
 *	a new mapping for the destination register.
 *
 *	V01.010		23-Jun-2017	Jonathan D. Belanger
 *	The way I'm handling the Program Counter (PC/VPC) is not correct.  When
 *	instructions are decoded, queued, and executed, their results are not
 *	realized until the instruction is retired.  The Integer Control
 *	instructions currently determine the next PC and set it immediately. not
 *	are retirement.  This is because I tried to do to much in the code that
 *	handles the PC/VPC.  The next PC should only be updated as a part of normal
 *	instruction queuing or instruction retirement.  We should only have one
 *	way to "set" the PC, but other functions to calculate one, based on various
 *	criteria.
 *
 *	V01.011		18-Jul-2017
 *	Updated the instruction decoding code to replace instruction indicated
 *	registers for PALshadow registers when we are in, or going to be in,
 *	PALmode.  Also, do not include floating point registers, as there are no
 *	PALshadow registers for these.
 *
 *	TODO:	We need a retirement function to put the destination value into the
 *			physical register and indicate that the register value is Valid.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ibox.h"

/*
 * A local structure used to calculate the PC for a CALL_PAL function.
 */
struct palBaseBits21264
{
	u64	res : 15;
	u64	highPC : 49;
};
struct palBaseBits21164
{
	u64	res : 14;
	u64	highPC : 50;
};

typedef union
{
	 struct palBaseBits21164	bits21164;
	 struct palBaseBits21264	bits21264;
	 u64						palBaseAddr;
	 
}AXP_IBOX_PALBASE_BITS;

struct palPCBits21264
{
	u64	palMode : 1;
	u64 mbz_1 : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 mbz_2 : 1;
	u64 highPC : 49;
};
struct palPCBits21164
{
	u64	palMode : 1;
	u64 mbz : 5;
	u64 func_5_0 : 6;
	u64 func_7 : 1;
	u64 mbo : 1;
	u64 highPC : 50;
};

typedef union
{
	struct palPCBits21164	bits21164;
	struct palPCBits21264	bits21264;
	AXP_PC					vpc;
} AXP_IBOX_PAL_PC;

struct palFuncBits
{
	u32 func_5_0 : 6;
	u32 res_1 : 1;
	u32 func_7 : 1;
	u32 res_2 : 24;
};

typedef union
{
	struct palFuncBits	bits;
	u32					func;
} AXP_IBOX_PAL_FUNC_BITS;

/*
 * Prototypes for local functions
 */
static AXP_OPER_TYPE AXP_DecodeOperType(u8, u32);

/* Functions to decode which instruction registers are used for which purpose
 * for the complex instructions (opcodes that have different ways to use
 * registers).
 */
static u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT);
static u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT);
static void AXP_RenameRegisters(AXP_21264_CPU *, AXP_INSTRUCTION *, u16);

/*
 * Functions that manage the instruction queue free entries.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextIQEntry(AXP_21264_CPU *);
static void AXP_ReturnIQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);
static AXP_QUEUE_ENTRY *AXP_GetNextFQEntry(AXP_21264_CPU *);
static void AXP_ReturnFQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);

/*
 * The following module specific structure and variable are used to be able to
 * decode opcodes that have differing ways register are utilized.  The index
 * into this array is the opcodeRegDecode fiels in the bits field of the
 * register mapping in the above table.  There is no entry [0], so that is
 * just NULL and should never get referenced.
 */
 typedef u16 (*regDecodeFunc)(AXP_INS_FMT);
regDecodeFunc decodeFuncs[] =
{
	NULL,
	AXP_RegisterDecodingOpcode11,
	AXP_RegisterDecodingOpcode14,
	AXP_RegisterDecodingOpcode15_16,
	AXP_RegisterDecodingOpcode15_16,
	AXP_RegisterDecodingOpcode17,
	AXP_RegisterDecodingOpcode18,
	AXP_RegisterDecodingOpcode1c
};

/*
 * AXP_Decode_Rename
 * 	This function is called to take a set of 4 instructions and decode them
 * 	and then rename the architectural registers to physical ones.  The results
 * 	are put onto either the Integer Queue or Floating-point Queue (FQ) for
 * 	execution.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *	next:
 *		A pointer to a location to receive the next 4 instructions to be
 *		processed.
 *
 * Output Parameters:
 * 	decodedInsr:
 * 		A pointer to the decoded version of the instruction.
 *
 * Return value:
 * 	None.
 */
void AXP_Decode_Rename(AXP_21264_CPU *cpu,
					   AXP_INS_LINE *next,
					   int nextInstr,
					   AXP_INSTRUCTION *decodedInstr)
{
	AXP_REG_DECODE	decodeRegisters;
	bool			callingPAL = false;
	bool			src1Float = false;
	bool			src2Float = false;
	bool			destFloat = false;

	/*
	 * Decode the next instruction.
	 *
	 * First, Assign a unique ID to this instruction (the counter should
	 * auto-wrap).
	 */
	decodedInstr->uniqueID = cpu->instrCounter++;

	/*
	 * Let's, decode the instruction.
	 */
	decodedInstr->format = next->instrType[nextInstr];
	decodedInstr->opcode = next->instructions[nextInstr].pal.opcode;
	switch (decodedInstr->format)
	{
		case Bra:
		case FPBra:
			decodedInstr->displacement = next->instructions[nextInstr].br.branch_disp;
			break;

		case FP:
			decodedInstr->function = next->instructions[nextInstr].fp.func;
			break;

		case Mem:
		case Mbr:
			decodedInstr->displacement = next->instructions[nextInstr].mem.mem.disp;
			break;

		case Mfc:
			decodedInstr->function = next->instructions[nextInstr].mem.mem.func;
			break;

		case Opr:
			decodedInstr->function = next->instructions[nextInstr].oper1.func;
			break;

		case Pcd:
			decodedInstr->function = next->instructions[nextInstr].pal.palcode_func;
			callingPAL = true;
			break;

		case PAL:
			switch (decodedInstr->opcode)
			{
				case HW_LD:
				case HW_ST:
					decodedInstr->displacement = next->instructions[nextInstr].hw_ld.disp;
					decodedInstr->type_hint_index = next->instructions[nextInstr].hw_ld.type;
					decodedInstr->len_stall = next->instructions[nextInstr].hw_ld.len;
					break;

				case HW_RET:
					decodedInstr->displacement = next->instructions[nextInstr].hw_ret.disp;
					decodedInstr->type_hint_index = next->instructions[nextInstr].hw_ret.hint;
					decodedInstr->len_stall = next->instructions[nextInstr].hw_ret.stall;
					break;

				case HW_MFPR:
				case HW_MTPR:
					decodedInstr->type_hint_index = next->instructions[nextInstr].hw_mxpr.index;
					decodedInstr->scbdMask = next->instructions[nextInstr].hw_mxpr.scbd_mask;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	decodedInstr->type = AXP_OperationType(decodedInstr->opcode);
	if ((decodedInstr->type == Other) && (decodedInstr->format != Res))
		decodedInstr->type = AXP_DecodeOperType(
				decodedInstr->opcode,
				decodedInstr->function);
	decodeRegisters = AXP_RegisterDecoding(decodedInstr->opcode);
	if (decodeRegisters.bits.opcodeRegDecode != 0)
		decodeRegisters.raw =
			decodeFuncs[decodeRegisters.bits.opcodeRegDecode]
				(next->instructions[nextInstr]);

	/*
	 * Decode destination register
	 */
	switch (decodeRegisters.bits.dest)
	{
		case AXP_REG_RA:
			decodedInstr->aDest = next->instructions[nextInstr].oper1.ra;
			break;

		case AXP_REG_RB:
			decodedInstr->aDest = next->instructions[nextInstr].oper1.rb;
			break;

		case AXP_REG_RC:
			decodedInstr->aDest = next->instructions[nextInstr].oper1.rc;
			break;

		case AXP_REG_FA:
			decodedInstr->aDest = next->instructions[nextInstr].fp.fa;
			destFloat = true;
			break;

		case AXP_REG_FB:
			decodedInstr->aDest = next->instructions[nextInstr].fp.fb;
			destFloat = true;
			break;

		case AXP_REG_FC:
			decodedInstr->aDest = next->instructions[nextInstr].fp.fc;
			destFloat = true;
			break;

		default:

			/*
			 *  If the instruction being decoded is a CALL_PAL, then there is a
			 *  linkage register (basically a return address after the CALL_PAL
			 *  has completed).  For Jumps, the is usually specified in the
			 *  register fields of the instruction.  For CALL_PAL, this is
			 *  either R23 or R27, depending upon the setting of the
			 *  call_pal_r23 in the I_CTL IPR.
			 */
			if (decodedInstr->opcode == PAL00)
			{
				if (cpu->iCtl.call_pal_r23 == 1)
					decodedInstr->aDest = 23;
				else
					decodedInstr->aDest = 27;
			}
			else
				decodedInstr->aDest = AXP_UNMAPPED_REG;
			break;
	}

	/*
	 * Decode source1 register
	 */
	switch (decodeRegisters.bits.src1)
	{
		case AXP_REG_RA:
			decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.ra;
			break;

		case AXP_REG_RB:
			decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.rb;
			break;

		case AXP_REG_RC:
			decodedInstr->aSrc1 = next->instructions[nextInstr].oper1.rc;
			break;

		case AXP_REG_FA:
			decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fa;
			src1Float = true;
			break;

		case AXP_REG_FB:
			decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fb;
			src1Float = true;
			break;

		case AXP_REG_FC:
			decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fc;
			src1Float = true;
			break;

		default:
			decodedInstr->aSrc1 = AXP_UNMAPPED_REG;
			break;
	}

	/*
	 * Decode source2 register
	 */
	switch (decodeRegisters.bits.src2)
	{
		case AXP_REG_RA:
			decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.ra;
			break;

		case AXP_REG_RB:
			decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.rb;
			break;

		case AXP_REG_RC:
			decodedInstr->aSrc2 = next->instructions[nextInstr].oper1.rc;
			break;

		case AXP_REG_FA:
			decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fa;
			src2Float = true;
			break;

		case AXP_REG_FB:
			decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fb;
			src2Float = true;
			break;

		case AXP_REG_FC:
			decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fc;
			src2Float = true;
			break;

		default:
			decodedInstr->aSrc2 = AXP_UNMAPPED_REG;
			break;
	}

	/*
	 * When running in PALmode, the shadow registers may come into play.  If we
	 * are in PALmode, then the PALshadow registers may come into play.  If so,
	 * we need to replace the specified register with the PALshadow one.
	 *
	 * There is no such thing as Floating Point PALshadow registers, so the
	 * register specified on the instruction is the one to use.  Now need to
	 * check.
	 */
	decodedInstr->pc = next->instrPC[nextInstr];
	callingPAL |= (decodedInstr->pc.pal == AXP_PAL_MODE);
	if (src1Float == false)
		decodedInstr->aSrc1 = AXP_REG(decodedInstr->aSrc1, callingPAL);
	if (src2Float == false)
		decodedInstr->aSrc2 = AXP_REG(decodedInstr->aSrc2, callingPAL);
	if (destFloat == false)
		decodedInstr->aDest = AXP_REG(decodedInstr->aDest, callingPAL);

	/*
	 * We need to rename the architectural registers to physical
	 * registers, now that we know which one, if any, is the
	 * destination register and which one(s) is(are) the source
	 * register(s).
	 */
	AXP_RenameRegisters(cpu, decodedInstr, decodeRegisters.raw);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_DecodeOperType
 * 	This function is called to convert an operation type of 'Other' to a more
 * 	usable value.  The opcode and funcCode are used in combination to determine
 * 	the operation type.
 *
 * Input Parameters:
 * 	opCode:
 * 		A byte value specifying the opcode to be used to determine the
 * 		operation type.
 * 	funcCode:
 * 		A 32-bit value specifying the function code used to determine the
 * 		operation type.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The operation type value for the opCode/FuncCode pair.
 */
static AXP_OPER_TYPE AXP_DecodeOperType(u8 opCode, u32 funcCode)
{
	AXP_OPER_TYPE retVal = Other;

	switch (opCode)
	{
		case INTA:		/* OpCode == 0x10 */
			if (funcCode == AXP_FUNC_CMPBGE)
				retVal = Logic;
			else
				retVal = Arith;
			break;

		case INTL:		/* OpCode == 0x11 */
			if ((funcCode == AXP_FUNC_AMASK) ||
				(funcCode == AXP_FUNC_IMPLVER))
				retVal = Oper;
			else
				retVal = Logic;
			break;

		case FLTV:		/* OpCode == 0x15 */
			if ((funcCode == AXP_FUNC_CMPGEQ) ||
				(funcCode == AXP_FUNC_CMPGLT) ||
				(funcCode == AXP_FUNC_CMPGLE) ||
				(funcCode == AXP_FUNC_CMPGEQ_S) ||
				(funcCode == AXP_FUNC_CMPGLT_S) ||
				(funcCode == AXP_FUNC_CMPGLE_S))
				retVal = Logic;
			else
				retVal = Arith;
			break;

		case FLTI:		/* OpCode == 0x16 */
			if ((funcCode == AXP_FUNC_CMPTUN) ||
				(funcCode == AXP_FUNC_CMPTEQ) ||
				(funcCode == AXP_FUNC_CMPTLT) ||
				(funcCode == AXP_FUNC_CMPTLE) ||
				(funcCode == AXP_FUNC_CMPTUN_SU) ||
				(funcCode == AXP_FUNC_CMPTEQ_SU) ||
				(funcCode == AXP_FUNC_CMPTLT_SU) ||
				(funcCode == AXP_FUNC_CMPTLE_SU))
				retVal = Logic;
			else
				retVal = Arith;
			break;

		case FLTL:		/* OpCode == 0x17 */
			if (funcCode == AXP_FUNC_MT_FPCR)
				retVal = Load;
			else if (funcCode == AXP_FUNC_MF_FPCR)
				retVal = Store;
			else
				retVal = Arith;
			break;

		case MISC:		/* OpCode == 0x18 */
			if ((funcCode == AXP_FUNC_RPCC) ||
				(funcCode == AXP_FUNC_RC) ||
				(funcCode == AXP_FUNC_RS))
				retVal = Load;
			else
				retVal = Store;
			break;
	}

	/*
	 * Return back to the caller with what we determined.
	 */
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode11
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x11.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT instr)
{
	u16 retVal = 0;

	switch (instr.oper1.func)
	{
		case 0x61:		/* AMASK */
			retVal = AXP_DEST_RC|AXP_SRC1_RB;
			break;

		case 0x6c:		/* IMPLVER */
			retVal = AXP_DEST_RC;
			break;

		default:		/* All others */
			retVal = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB;
			break;
	}
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode14
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x14.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT instr)
{
	u16 retVal = AXP_DEST_FC;

	if ((instr.oper1.func & 0x00f) != 0x004)
		retVal |= AXP_SRC1_FB;
	else
		retVal |= AXP_SRC1_RB;
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode15_16
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcodes associated with this instruction
 * 	are 0x15 and 0x16.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT instr)
{
	u16 retVal = AXP_DEST_FC;

	if ((instr.fp.func & 0x008) == 0)
		retVal |= (AXP_SRC1_FA|AXP_SRC2_FB);
	else
		retVal |= AXP_SRC1_FB;
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode17
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x17.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT instr)
{
	u16 retVal = 0;

	switch (instr.fp.func)
	{
		case 0x010:
		case 0x030:
		case 0x130:
		case 0x530:
			retVal = AXP_DEST_FC|AXP_SRC1_FB;
			break;

		case 0x024:
			retVal = AXP_DEST_FA;
			break;

		case 0x025:
			retVal = AXP_SRC1_FA;
			break;

		default:		/* All others */
			retVal = AXP_DEST_FC|AXP_SRC1_FA|AXP_SRC2_FB;
			break;
	}
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode18
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x18.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT instr)
{
	u16 retVal = 0;

	if ((instr.mem.mem.func & 0x8000) != 0)
	{
		if ((instr.mem.mem.func == 0xc000) ||
			(instr.mem.mem.func == 0xe000) ||
			(instr.mem.mem.func == 0xf000))
			retVal = AXP_DEST_RA;
		else
			retVal = AXP_SRC1_RB;
	}
	return(retVal);
}

/*
 * AXP_RegisterDecodingOpcode1c
 * 	This function is called to determine which registers in the instruction are
 * 	the destination and source.  It returns a proper mask to be used by the
 * 	register renaming process.  The Opcode associated with this instruction is
 * 	0x1c.
 *
 * Input Parameters:
 * 	instr:
 * 		A valus of the instruction being parsed.
 *
 * Output Parameters:
 * 	None.
 *
 * Return value:
 * 	The register mask to be used to rename the registers from architectural to
 * 	physical.
 */
static u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT instr)
{
	u16 retVal = AXP_DEST_RC;

	switch (instr.oper1.func)
	{
		case 0x31:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			retVal |= (AXP_SRC1_RA|AXP_SRC2_RB);
			break;

		case 0x70:
		case 0x78:
			retVal |= AXP_SRC1_FA;
			break;

		default:		/* All others */
			retVal |= AXP_SRC1_RB;
			break;
	}
	return(retVal);
}

/*
 * AXP_RenameRegisters
 *	This function is called to map the instruction registers from architectural
 *	to physical ones.  For the destination register, we get the next one off
 *	the free list.  We also differentiate between integer and floating point
 *	registers at this point (previously, we just noted it).
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	decodedInstr:
 *		A pointer to the structure containing a decoded representation of the
 *		Alpha AXP instruction.
 *	decodedRegs:
 *		A value containing the flags indicating which registers are being
 *		utilized.  We use this information to differentiate between integer
 *		and floating-point registers.
 *
 * Output Parameters:
 *	decodedInstr:
 *		A pointer to structure that will be updated to indicate which physical
 *		registers are being used for this particular instruction.
 *
 * Return Value:
 *	None.
 */
static void AXP_RenameRegisters(
		AXP_21264_CPU *cpu,
		AXP_INSTRUCTION *decodedInstr,
		u16 decodedRegs)
{
	bool src1Float = ((decodedRegs & 0x0008) == 0x0008);
	bool src2Float = ((decodedRegs & 0x0080) == 0x0080);
	bool destFloat = ((decodedRegs & 0x0800) == 0x0800);

	/*
	 * The source registers just use the current register mapping (integer or
	 * floating-point).  If the register number is 31, it is not mapped.
	 */
	decodedInstr->src1 = src1Float ? cpu->pfMap[decodedInstr->aSrc1].pr :
			cpu->prMap[decodedInstr->aSrc1].pr;
	decodedInstr->src2 = src2Float ? cpu->pfMap[decodedInstr->aSrc2].pr :
			cpu->prMap[decodedInstr->aSrc2].pr;

	/*
	 * The destination register needs a little more work.  If the register
	 * number is 31, it is not mapped.
	 */
	if (decodedInstr->aDest != AXP_UNMAPPED_REG)
	{

		/*
		 * Is this a floating-point register or an integer register?
		 */
		if (destFloat)
		{

			/*
			 * Get the next register off of the free-list.
			 */
			decodedInstr->dest = cpu->pfFreeList[cpu->pfFlStart];

			/*
			 * If the register for the previous mapping was not R31 or F31
			 * then, put this previous register back on the free-list, then
			 * make the previous mapping the current one and the current
			 * mapping the register we just took off the free-list.
			 */
			if (cpu->pfMap[decodedInstr->aDest].prevPr != AXP_UNMAPPED_REG)
			{
				cpu->pfFreeList[cpu->pfFlEnd] = cpu->pfMap[decodedInstr->aDest].prevPr;
				cpu->pfFlEnd = (cpu->pfFlEnd + 1) % AXP_F_FREELIST_SIZE;
			}
			cpu->pfMap[decodedInstr->aDest].prevPr = cpu->pfMap[decodedInstr->aDest].pr;
			cpu->pfMap[decodedInstr->aDest].pr = decodedInstr->dest;

			/*
			 * Until the instruction executes, the newly mapped register is
			 * pending a value.  After execution, the state will be waiting to
			 * retire.  After retirement, the value will be written to the
			 * physical register.
			 */
			cpu->pfState[decodedInstr->aDest] = Pending;

			/*
			 * Compute the next free physical register on the free-list.  Wrap
			 * the counter to the beginning of the list, if we are at the end.
			 */
			cpu->pfFlStart = (cpu->pfFlStart + 1) % AXP_F_FREELIST_SIZE;
		}
		else
		{

			/*
			 * Get the next register off of the free-list.
			 */
			decodedInstr->dest = cpu->prFreeList[cpu->prFlStart++];

			/*
			 * If the register for the previous mapping was not R31 or F31
			 * then, put this previous register back on the free-list, then
			 * make the previous mapping the current one and the current
			 * mapping the register we just took off the free-list.
			 */
			if (cpu->prMap[decodedInstr->aDest].prevPr != AXP_UNMAPPED_REG)
			{
				cpu->prFreeList[cpu->prFlEnd] = cpu->prMap[decodedInstr->aDest].prevPr;
				cpu->prFlEnd = (cpu->prFlEnd + 1) % AXP_I_FREELIST_SIZE;
			}
			cpu->prMap[decodedInstr->aDest].prevPr = cpu->pfMap[decodedInstr->aDest].pr;
			cpu->prMap[decodedInstr->aDest].pr = decodedInstr->dest;

			/*
			 * Until the instruction executes, the newly mapped register is
			 * pending a value.  After execution, the state will be waiting to
			 * retire.  After retirement, the value will be written to the
			 * physical register.
			 */
			cpu->prState[decodedInstr->aDest] = Pending;

			/*
			 * Compute the next free physical register on the free-list.  Wrap
			 * the counter to the beginning of the list, if we are at the end.
			 */
			cpu->prFlStart = (cpu->prFlStart + 1) % AXP_I_FREELIST_SIZE;
		}
	}
	else
	{

		/*
		 * No need to map R31 or F31 to a physical register on the free-list.
		 * R31 and F31 always equal zero and writes to them do not occur.
		 */
		decodedInstr->dest = src2Float ? cpu->pfMap[decodedInstr->aDest].pr :
				cpu->prMap[decodedInstr->aDest].pr;
	}
	return;
}

/*
 * AXP_GetNextIQEntry
 * 	This function is called to get the next available entry for the IQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A pointer to the next available pre-allocated queue entry for the IQ.
 *
 * NOTE:	This function assumes that there is always at least one free entry.
 * 			Since the number of entries pre-allocated is equal to the maximum
 * 			number of entries that can be in the IQ, this is not necessarily a
 * 			bad assumption.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextIQEntry(AXP_21264_CPU *cpu)
{
	AXP_QUEUE_ENTRY *retVal = NULL;

	retVal = &cpu->iqEntries[cpu->iqEFlStart];
	cpu->iqEFlStart = (cpu->iqEFlStart + 1) % AXP_IQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ReturnIQEntry
 * 	This function is called to get the next available entry for the IQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	entry:
 *		A pointer to the entry being returned to the free-list.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
static void AXP_ReturnIQEntry(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{

	/*
	 * Enter the index of the IQ entry onto the end of the free-list.
	 */
	cpu->iqEFreelist[cpu->iqEFlEnd] = entry->index;

	/*
	 * Increment the counter, in a round-robin fashion, for the entry just
	 * after end of the free-list.
	 */
	cpu->iqEFlEnd = (cpu->iqEFlEnd + 1) % AXP_IQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_GetNextFQEntry
 * 	This function is called to get the next available entry for the FQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A pointer to the next available pre-allocated queue entry for the FQ.
 *
 * NOTE:	This function assumes that there is always at least one free entry.
 * 			Since the number of entries pre-allocated is equal to the maximum
 * 			number of entries that can be in the FQ, this is not necessarily a
 * 			bad assumption.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextFQEntry(AXP_21264_CPU *cpu)
{
	AXP_QUEUE_ENTRY *retVal = NULL;

	retVal = &cpu->fqEntries[cpu->fqEFlStart];
	cpu->fqEFlStart = (cpu->fqEFlStart + 1) % AXP_FQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ReturnFQEntry
 * 	This function is called to get the next available entry for the FQ queue.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	entry:
 *		A pointer to the entry being returned to the free-list.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
static void AXP_ReturnFQEntry(AXP_21264_CPU *cpu, AXP_QUEUE_ENTRY *entry)
{

	/*
	 * Enter the index of the IQ entry onto the end of the free-list.
	 */
	cpu->fqEFreelist[cpu->fqEFlEnd] = entry->index;

	/*
	 * Increment the counter, in a round-robin fashion, for the entry just
	 * after end of the free-list.
	 */
	cpu->fqEFlEnd = (cpu->fqEFlEnd + 1) % AXP_FQ_LEN;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_AddVPC
 * 	This function is called to add a Virtual Program Counter (VPC) to the list
 * 	of VPCs.  This is a round-robin list.  The End points to the next entry to
 * 	be written to.  The Start points to the least recent VPC, which is the one
 * 	immediately after the End.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	vpc:
 *		A value of the next VPC to be entered into the VPC list.
 *
 * Output Parameters:
 * 	cpu:
 * 		The vpc list will be updated with the newly added VPC and the Start and
 * 		End indexes will be updated appropriately.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_AddVPC(AXP_21264_CPU *cpu, AXP_PC vpc)
{
	cpu->vpc[cpu->vpcEnd] = vpc;
	cpu->vpcEnd = (cpu->vpcEnd + 1) % AXP_INFLIGHT_MAX;
	if (cpu->vpcEnd == cpu->vpcStart)
		cpu->vpcStart = (cpu->vpcStart + 1) % AXP_INFLIGHT_MAX;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_GetPALFuncVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
 * 	specific PAL function which is an offset from the address specified in the
 *	PAL_BASE register.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	func:
 *		The value of the function field in the PALcode Instruction Format.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value that the PC should be set to to call the requested function.
 */
AXP_PC AXP_21264_GetPALFuncVPC(AXP_21264_CPU *cpu, u32 func)
{
	AXP_IBOX_PAL_PC pc;
	AXP_IBOX_PALBASE_BITS palBase;
	AXP_IBOX_PAL_FUNC_BITS palFunc;

	palBase.palBaseAddr = cpu->palBase.pal_base_pc;
	palFunc.func = func;
	
	/*
	 * We assume that the function supplied follows any of the following
	 * criteria:
	 *
	 *		Is in the range of 0x40 and 0x7f, inclusive
	 *		Is greater than 0xbf
	 *		Is between 0x00 and 0x3f, inclusive, and IER_CM[CM] is not equal to
	 *			the kernel mode value (0).
	 *
	 * Now, let's compose the PC for the PALcode function we are being
	 * requested to call.
	 */
	if (cpu->majorType >= EV6)
	{
		pc.bits21264.highPC = palBase.bits21264.highPC;
		pc.bits21264.mbz_2 = 0;
		pc.bits21264.mbo = 1;
		pc.bits21264.func_7 = palFunc.bits.func_7;
		pc.bits21264.func_5_0 = palFunc.bits.func_5_0;
		pc.bits21264.mbz_1 = 0;
		pc.bits21264.palMode = AXP_PAL_MODE;
	}
	else
	{
		pc.bits21164.highPC = palBase.bits21164.highPC;
		pc.bits21164.mbo = 1;
		pc.bits21164.func_7 = palFunc.bits.func_7;
		pc.bits21164.func_5_0 = palFunc.bits.func_5_0;
		pc.bits21164.mbz = 0;
		pc.bits21164.palMode = AXP_PAL_MODE;
	}

	/*
	 * Return the composed VPC it back to the caller.
	 */
	return(pc.vpc);
}

/*
 * AXP_21264_GetPALBaseVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
 * 	specific offset from the address specified in the PAL_BASE register.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	offset:
 *		An offset value, from PAL_BASE, of the next VPC to be entered into the
 *		VPC list.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value that the PC should be set to to call the requested offset.
 */
AXP_PC AXP_21264_GetPALBaseVPC(AXP_21264_CPU *cpu, u64 offset)
{
	u64 pc;

	pc = cpu->palBase.pal_base_pc + offset;

	/*
	 * Get the VPC set with the correct PALmode bit and return it back to the
	 * caller.
	 */
	return(AXP_21264_GetVPC(cpu, pc, AXP_PAL_MODE));
}

/*
 * AXP_21264_GetVPC
 * 	This function is called to get the Virtual Program Counter (VPC) to a
 * 	specific value.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	addr:
 *		A value of the next VPC to be entered into the VPC list.
 *	palMode:
 *		A value to indicate if we will be running in PAL mode.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The calculated VPC, based on a target virtual address.
 */
AXP_PC AXP_21264_GetVPC(AXP_21264_CPU *cpu, u64 pc, u8 pal)
{
	union
	{
		u64		pc;
		AXP_PC	vpc;
	} vpc;

	vpc.pc = pc;
	vpc.vpc.res = 0;
	vpc.vpc.pal = pal & AXP_PAL_MODE;

	/*
	 * Return back to the caller.
	 */
	return(vpc.vpc);
}

/*
 * AXP_21264_GetNextVPC
 * 	This function is called to retrieve the VPC for the next set of
 * 	instructions to be fetched.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	The VPC of the next set of instructions to fetch from the cache..
 */
AXP_PC AXP_21264_GetNextVPC(AXP_21264_CPU *cpu)
{
	AXP_PC retVal;
	u32	prevVPC;

	/*
	 * The End, points to the next location to be filled.  Therefore, the
	 * previous location is the next VPC to be executed.
	 */
	prevVPC = ((cpu->vpcEnd != 0) ? cpu->vpcEnd : AXP_INFLIGHT_MAX) - 1;
	retVal = cpu->vpc[prevVPC];

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_IncrementVPC
 * 	This function is called to increment Virtual Program Counter (VPC).
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value of the incremented PC.
 */
AXP_PC AXP_21264_IncrementVPC(AXP_21264_CPU *cpu)
{
	AXP_PC vpc;

	/*
	 * Get the PC for the instruction just executed.
	 */
	vpc = AXP_21264_GetNextVPC(cpu);

	/*
	 * Increment it.
	 */
	vpc.pc++;

	/*
	 * Store it on the VPC List and return to the caller.
	 */
	return(vpc);
}

/*
 * AXP_21264_DisplaceVPC
 * 	This function is called to add a displacement value to the VPC.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to emulate
 *		an Alpha AXP 21264 CPU.
 *	displacement:
 *		A signed 64-bit value to be added to the VPC.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 * 	The value of the PC with the displacement.
 */
AXP_PC AXP_21264_DisplaceVPC(AXP_21264_CPU *cpu, i64 displacement)
{
	AXP_PC vpc;

	/*
	 * Get the PC for the instruction just executed.
	 */
	vpc = AXP_21264_GetNextVPC(cpu);

	/*
	 * Increment and then add the displacement.
	 */
	vpc.pc = vpc.pc + 1 + displacement;

	/*
	 * Return back to the caller.
	 */
	return(vpc);
}

/*
 * AXP_21264_IboxMain
 * 	This function is called to perform the emulation for the Ibox within the
 * 	Alpha AXP 21264 CPU.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the structure holding the  fields required to emulate an
 * 		Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_IboxMain(AXP_21264_CPU *cpu)
{
	AXP_PC nextPC, branchPC;
	AXP_INS_LINE nextCacheLine;
	AXP_INSTRUCTION *decodedInstr;
	AXP_QUEUE_ENTRY *xqEntry;
	u32 ii;
	bool local, global, choice;
	u16 whichQueue;

	/*
	 * Here we'll loop starting at the current PC and working our way through
	 * all the instructions.  We will do the following steps.
	 *
	 *	1) Fetch the next set of instructions.
	 *	2) If step 1 returns a Miss, then get the Cbox to fill the Icache with
	 *		the next set of instructions.
	 *	3) If step 1 returns a WayMiss, then we need to generate an ITB Miss
	 *		exception, with the PC address we were trying to step to as the
	 *		return address.
	 *	4) If step 1 returns a Hit, then process the next set of instructions.
	 *		a) Decode and rename the registers in each instruction into the ROB.
	 *		b) If the decoded instruction is a branch, then predict if this
	 *			branch will be taken.
	 *		c) If step 4b is true, then adjust the line and set predictors
	 *			appropriately.
	 *		d) Fetch and insert an instruction entry into the appropriate
	 *			instruction queue (IQ or FQ).
	 *	5) If the branch predictor indicated a branch, then determine if
	 *		we have to load an ITB entry and ultimately load the iCache
	 *	6) Loop back to step 1.
	 */

	/*
	 * We keep looping while the CPU is in a running state.
	 */
	while (cpu->cpuState == Run)
	{

		/*
		 * Get the PC for the next set of instructions to be fetched from the
		 * Icache and Fetch those instructions.
		 */
		nextPC = AXP_21264_GetNextVPC(cpu);

		/*
		 * The cache fetch will return true or false.  If true, we received the
		 * next four instructions.  If false, we need to to determine if we
		 * need to call the PALcode to add a TLB entry to the ITB and/or then
		 * get the Cbox to fill the iCache.  If the former, store the faulting
		 * PC and generate an exception.
		 */
		if (AXP_IcacheFetch(cpu, nextPC, &nextCacheLine) == true)
		{
			for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
			{
				decodedInstr = &cpu->rob[cpu->robEnd];
				cpu->robEnd = (cpu->robEnd + 1) % AXP_INFLIGHT_MAX;
				if (cpu->robEnd == cpu->robStart)
					cpu->robStart = (cpu->robStart + 1) % AXP_INFLIGHT_MAX;
				AXP_Decode_Rename(cpu, &nextCacheLine, ii, decodedInstr);
				if (decodedInstr->type == Branch)
				{
					decodedInstr->branchPredict = AXP_Branch_Prediction(
								cpu,
								nextPC,
								&local,
								&global,
								&choice);

					/*
					 * TODO:	First, we can use the PC handling functions
					 *			to calculate the branch PC.
					 * TODO:	Second, we need to make sure that we are
					 *			calculating the branch address correctly.
					 * TODO:	Third, We need to be able to handle
					 *			returns, and utilization of the, yet to be
					 *			implemented, prediction stack.
					 * TODO:	Finally, we need to flush the remaining
					 *			instructions to be decoded and go get the
					 *			predicted instructions.
					 */
					if (decodedInstr->branchPredict == true)
					{
						/* bool cacheStatus; TODO: uncomment*/

						branchPC.pc = nextPC.pc + 1 + decodedInstr->displacement;
						/* cacheStatus = */ AXP_IcacheValid(
												cpu,
												branchPC);

						/*
						 * TODO:	If we get a Hit, there is nothing else
						 * 			to do.  If we get a Miss, we probably
						 * 			should have someone fill the Icache
						 * 			with the next set of instructions.  If
						 * 			a WayMiss, we don't do anything either.
						 * 			In this last case, we will end up
						 * 			generating an ITB_MISS event to be
						 * 			handled by the PALcode.
						 */
					}
				}
				whichQueue = AXP_InstructionQueue(decodedInstr->opcode);
				if(whichQueue == AXP_COND)
				{
					if (decodedInstr->opcode == ITFP)
					{
						if ((decodedInstr->function == AXP_FUNC_ITOFS) ||
							(decodedInstr->function == AXP_FUNC_ITOFF) ||
							(decodedInstr->function == AXP_FUNC_ITOFT))
							whichQueue = AXP_IQ;
						else
							whichQueue = AXP_FQ;
					}
					else	/* FPTI */
					{
						if ((decodedInstr->function == AXP_FUNC_FTOIT) ||
							(decodedInstr->function == AXP_FUNC_FTOIS))
							whichQueue = AXP_FQ;
						else
							whichQueue = AXP_IQ;
						}
				}
				if (whichQueue == AXP_IQ)
				{
					xqEntry = AXP_GetNextIQEntry(cpu);
					xqEntry->ins = decodedInstr;
					AXP_InsertCountedQueue(&cpu->iq.header, &xqEntry->header);
				}
				else	/* FQ */
				{
					xqEntry = AXP_GetNextFQEntry(cpu);
					xqEntry->ins = decodedInstr;
					AXP_InsertCountedQueue(&cpu->fq.header, &xqEntry->header);
				}
				decodedInstr->state = Queued;
				nextPC = AXP_21264_IncrementVPC(cpu);
			}
		}
		else
		{
			AXP_21264_TLB *itb;

			itb = AXP_findTLBEntry(cpu, *((u64 *) &nextPC), false);

			/*
			 * If we didn't get an ITB, then we got to a virtual address that
			 * has not yet to be mapped.  We need to call the PALcode to get
			 * this mapping for us, at which time we'll attempt to fetch the
			 * instructions again, which will cause us to get here again, but
			 * this time the ITB will be found.
			 */
			if (itb == NULL)
			{

				/*
				 * TODO:	Calling PALcode implies waiting for outstanding
				 * 			instructions top complete, then performing a
				 * 			branch/jump to the appropriate PC.  We need to get
				 * 			this working, as well.
				 */
				cpu->excAddr.exc_pc = nextPC;
				nextPC = AXP_21264_GetPALBaseVPC(cpu, AXP_ITB_MISS);
			}

			/*
			 * We failed to get the next set of instructions from the Icache.
			 * We need to request the Cbox to get them and put them into the
			 * cache.  We are going to have some kind of pending Cbox indicator
			 * to know when the Cbox has actually filled in the cache block.
			 *
			 * TODO:	We may want to try and utilize the branch predictor to
			 * 			"look ahead" and request the Cbox fill in the Icache
			 * 			before we have a cache miss, in oder to avoid this
			 * 			waiting on the Cbox (at least for too long a period of
			 * 			time).
			 */
			else
			{
				AXP_ReturnIQEntry(cpu, xqEntry);	/* TODO: Remove - keep gcc happy */
				AXP_ReturnFQEntry(cpu, xqEntry);	/* TODO: Remove - keep gcc happy */
			}
		}
	}
	return;
}
