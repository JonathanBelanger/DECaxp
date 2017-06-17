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
 *	TODO:	We need a retirement function to put the destination value into the
 *			physical register and indicate that the register value is Valid.
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_ICache.h"
#include "AXP_21264_Ibox.h"

/*
 * Prototypes for local functions
 *
 * Functions to decode which instruction registers are used for which purpose
 * for the complex instructions (opcodes that have different ways to use
 * registers).
 */
inline u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT);
inline u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT);
inline u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT);
inline u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT);
inline u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT);
inline u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT);
static void AXP_RenameRegisters(AXP_21264_CPU *, AXP_INSTRUCTION *, u16);

/*
 * Functions that manage the instruction queue free entries.
 */
static AXP_QUEUE_ENTRY *AXP_GetNextIQEntry(AXP_21264_CPU *);
static void AXP_ReturnIQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);
static AXP_QUEUE_ENTRY *AXP_GetNextFQEntry(AXP_21264_CPU *);
static void AXP_ReturnFQEntry(AXP_21264_CPU *, AXP_QUEUE_ENTRY *);


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
	u16				res;	/* 32-bit align 								*/
};

static struct instructDecode insDecode[] =
{
/* Format	Type	Registers   									Opcode	Mnemonic	Description 					*/
	{Pcd,	Other,	{ .raw = 0}, 0},								/* 00		CALL_PAL	Trap to PALcode				*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 01					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 02					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 03					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 04					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 05					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 06					Reserved for Digital		*/
	{Res,	Other,	{ .raw = 0}, 0},								/* 07					Reserved for Digital		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 08		LDA			Load address				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 09		LDAH		Load address high			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 0A		LDBU		Load zero-extended byte		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 0B		LDQ_U		Load unaligned quadword		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 0C		LDWU		Load zero-extended word		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 0D		STW			Store word					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 0E		STB			Store byte					*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 0F		STQ_U		Store unaligned quadword	*/
	{Opr,	Other,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, 0},/* 10		ADDL		Add longword				*/
	{Opr,	Other,	{ .raw = AXP_OPCODE_11}, 0},					/* 11		AND			Logical product				*/
	{Opr,	Other,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, 0},/* 12		MSKBL		Mask byte low				*/
	{Opr,	Other,	{ .raw = AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB}, 0},/* 13		MULL		Multiply longword			*/
	{FP,	Other,	{ .raw = AXP_OPCODE_14}, 0},					/* 14		ITOFS		Int to float move, S_float	*/
	{FP,	Other,	{ .raw = AXP_OPCODE_15}, 0},					/* 15		ADDF		Add F_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_16}, 0},					/* 16		ADDS		Add S_floating				*/
	{FP,	Other,	{ .raw = AXP_OPCODE_17}, 0},					/* 17		CVTLQ		Convert longword to quad	*/
	{Mfc,	Other,	{ .raw = AXP_OPCODE_18}, 0},					/* 18		TRAPB		Trap barrier				*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA}, 0},						/* 19		HW_MFPR		Reserved for PALcode		*/
	{Mbr,	Other,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 1A		JMP			Jump						*/
	{PAL,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 1B		HW_LD		Reserved for PALcode		*/
	{Cond,	Other,	{ .raw = AXP_OPCODE_1C}, 0},					/* 1C		SEXTB		Sign extend byte			*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RB}, 0},						/* 1D		HW_MTPR		Reserved for PALcode		*/
	{PAL,	Other,	{ .raw = AXP_SRC1_RB}, 0},						/* 1E		HW_REI		Reserved for PALcode		*/
	{PAL,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 1F		HW_ST		Reserved for PALcode		*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, 0},			/* 20 		LDF			Load F_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, 0},			/* 21		LDG			Load G_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, 0},			/* 22		LDS			Load S_floating				*/
	{Mem,	Load,	{ .raw = AXP_DEST_FA|AXP_SRC1_RB}, 0},			/* 23		LDT			Load T_floating				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, 0},			/* 24		STF			Store F_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, 0},			/* 25		STG			Store G_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, 0},			/* 26		STS			Store S_floating			*/
	{Mem,	Store,	{ .raw = AXP_SRC1_FA|AXP_SRC2_RB}, 0},			/* 27		STT			Store T_floating			*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 28		LDL			Load sign-extended long		*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 29		LDQ			Load quadword				*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 2A		LDL_L		Load sign-extend long lock	*/
	{Mem,	Load,	{ .raw = AXP_DEST_RA|AXP_SRC1_RB}, 0},			/* 2B		LDQ_L		Load quadword locked		*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 2C		STL			Store longword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 2D		STQ			Store quadword				*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 2E		STL_C		Store longword conditional	*/
	{Mem,	Store,	{ .raw = AXP_SRC1_RA|AXP_SRC2_RB}, 0},			/* 2F		STQ_C		Store quadword conditional	*/
	{Bra,	Other,	{ .raw = AXP_DEST_RA}, 0},						/* 30		BR			Unconditional branch		*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 31		FBEQ		Floating branch if = zero	*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 32		FBLT		Floating branch if < zero	*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 33		FBLE		Floating branch if <= zero	*/
	{Mbr,	Other,	{ .raw = AXP_DEST_RA}, 0},						/* 34		BSR			Branch to subroutine		*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 35		FBNE		Floating branch if != zero	*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 36		FBGE		Floating branch if >=zero	*/
	{FPBra,	Other,	{ .raw = AXP_SRC1_FA}, 0},						/* 37		FBGT		Floating branch if > zero	*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 38		BLBC		Branch if low bit clear		*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 39		BEQ			Branch if = zero			*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 3A		BLT			Branch if < zero			*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 3B		BLE			Branch if <= zero			*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 3C		BLBS		Branch if low bit set		*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 3D		BNE			Branch if != zero			*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0},						/* 3E		BGE			Branch if >= zero			*/
	{Bra,	Other,	{ .raw = AXP_SRC1_RA}, 0}						/* 3F		BGT			Branch if > zero			*/
};

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
 * AXP_Branch_Prediction
 *
 *	This function is called to determine if a branch should be taken or not.
 *	It uses past history, locally and globally, to determine this.
 *
 *	The Local History Table is indexed by bits 2-11 of the VPC.  This entry
 *	contains a 10-bit value (0-1023), which is generated by indicating when
 *	a branch is taken(1) versus not taken(0).  This value is used as an index
 *	into a Local Predictor Table.  This table contains a 3-bit saturation
 *	counter, which is incremented when a branch is actually taken and
 *	decremented when a branch is not taken.
 *
 *	The Global History Path, which is generated by the set of taken(1)/not
 *	taken(0) branches.  This is used as an index into a Global Predictor Table,
 *	which contains a 2-bit saturation counter.
 *
 *	The Global History Path is also used as an index into the Choice Predictor
 *	Table.  This table contains a 2-bit saturation counter that is incremented
 *	when the Global Predictor is correct, and decremented when the Local
 *	Predictor is correct.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	vpc:
 *		A 64-bit value of the Virtual Program Counter.
 *
 * Output Parameters:
 *	localTaken:
 *		A location to receive a value of true when the local predictor
 *		indicates that a branch should be taken.
 *	globalTaken:
 *		A location to receive a value of true when the global predictor
 *		indicates that a branch should be taken.
 *	choice:
 *		A location to receive a value of true when the global predictor should
 *		be selected, and false when the local predictor should be selected.
 *		This parameter is only used when the localPredictor and GlobalPredictor
 *		do not match.
 *
 * Return Value:
 *	None.
 */
bool AXP_Branch_Prediction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool *localTaken,
				bool *globalTaken,
				bool *choice)
{
	LPTIndex	lpt_index;
	int			lcl_history_idx;
	int			lcl_predictor_idx;
	bool		retVal;

	/*
	 * Determine how branch prediction should be performed based on the value
	 * of the BP_MODE field of the I_CTL register.
	 * 	1x = All branches to be predicted to fall through
	 * 	0x = Dynamic prediction is used
	 * 	01 = Local history prediction is used
	 * 	00 = Chooser selects Local or Global history based on its state
	 */
	if ((cpu->iCtl.bp_mode & AXP_I_CTL_BP_MODE_FALL) == AXP_I_CTL_BP_MODE_DYN)
	{

		/*
		 * Need to extract the index into the Local History Table from the VPC, and
		 * use this to determine the index into the Local Predictor Table.
		 */
		lpt_index.vpc = vpc;
		lcl_history_idx = lpt_index.index.index;
		lcl_predictor_idx = cpu->localHistoryTable.lcl_history[lcl_history_idx];

		/*
		 * Return the take(true)/don't take(false) for each of the Predictor
		 * Tables.  The choice is determined and returned, but my not be used by
		 * the caller.
		 */
		*localTaken = AXP_3BIT_TAKE(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
		if (cpu->iCtl.bp_mode == AXP_I_CTL_BP_MODE_CHOICE)
		{
			*globalTaken = AXP_2BIT_TAKE(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
			*choice = AXP_2BIT_TAKE(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
		}
		else
		{
			*globalTaken = false;
			*choice = false;	/* This will force choice to select Local */
		}
		if (*localTaken != *globalTaken)
			retVal = (*choice == true) ? *globalTaken : *localTaken;
		else
			retVal = *localTaken;
	}
	else
	{
		*localTaken = false;
		*globalTaken = false;
		*choice = false;
		retVal = false;
	}

	return(retVal);
}

/*
 * AXP_Branch_Direction
 *	This function is called when the branch instruction is retired to update the
 *	local, global, and choice prediction tables, and the local history table and
 *	global path history information.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *	vpc:
 *		A 64-bit value of the Virtual Program Counter.
 *	localTaken:
 *		A value of what was predicted by the local predictor.
 *	globalTaken:
 *		A value of what was predicted by the global predictor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_Branch_Direction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool taken,
				bool localTaken,
				bool globalTaken)
{
	LPTIndex	lpt_index;
	int			lcl_history_idx;
	int			lcl_predictor_idx;

	/*
	 * Need to extract the index into the Local History Table from the VPC, and
	 * use this to determine the index into the Local Predictor Table.
	 */
	lpt_index.vpc = vpc;
	lcl_history_idx = lpt_index.index.index;
	lcl_predictor_idx = cpu->localHistoryTable.lcl_history[lcl_history_idx];

	/*
	 * If the choice to take or not take a branch agreed with the local
	 * predictor, then indicate this for the choice predictor, by decrementing
	 * the saturation counter
	 */
	if ((taken == localTaken) && (taken != globalTaken))
	{
		AXP_2BIT_DECR(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
	}

	/*
	 * Otherwise, if the choice to take or not take a branch agreed with the
	 * global predictor, then indicate this for the choice predictor, by
	 * incrementing the saturation counter
	 *
	 * NOTE:	If the branch taken does not match both the local and global
	 *			predictions, then we don't update the choice at all (we had a
	 *			mis-prediction).
	 */
	else if ((taken != localTaken) && (taken == globalTaken))
	{
		AXP_2BIT_INCR(cpu->choicePredictor.choice_pred[cpu->globalPathHistory]);
	}

	/*
	 * If the branch was taken, then indicate this in the local and global
	 * prediction tables.  Additionally, indicate that the local and global
	 * paths were taken, when they agree.  Otherwise, decrement the appropriate
	 * predictions tables and indicate the local and global paths were not
	 * taken.
	 *
	 * NOTE:	If the local and global predictors indicated that the branch
	 *			should be taken, then both predictor are correct and should be
	 *			accounted for.
	 */
	if (taken == true)
	{
		AXP_3BIT_INCR(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
		AXP_2BIT_INCR(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
		AXP_LOCAL_PATH_TAKEN(cpu->localHistoryTable.lcl_history[lcl_history_idx]);
		AXP_GLOBAL_PATH_TAKEN(cpu->globalPathHistory);
	}
	else
	{
		AXP_3BIT_DECR(cpu->localPredictor.lcl_pred[lcl_predictor_idx]);
		AXP_2BIT_DECR(cpu->globalPredictor.gbl_pred[cpu->globalPathHistory]);
		AXP_LOCAL_PATH_NOT_TAKEN(cpu->localHistoryTable.lcl_history[lcl_history_idx]);
		AXP_GLOBAL_PATH_NOT_TAKEN(cpu->globalPathHistory);
	}
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
 * AXP_ICacheFetch
 * 	The instruction pre-fetcher (pre-decode) reads an octaword (16 bytes),
 * 	containing up to four naturally aligned instructions per cycle from the
 * 	Icache.  Branch prediction and line prediction bits accompany the four
 * 	instructions.  The branch prediction scheme operates most efficiently when
 * 	there is only one branch instruction is contained in the four fetched
 * 	instructions.
 *
 * 	An entry from the subroutine prediction stack, together with set
 * 	prediction bits for use by the Icache stream controller, are fetched along
 * 	with the octaword.  The Icache stream controller generates fetch requests
 * 	for additional cache lines and stores the Istream data in the Icache. There
 * 	is no separate buffer to hold Istream requests.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *	pc:
 *		A value that represents the program counter of the instruction being
 *		requested.
 *
 * Output Parameters:
 *	next:
 *		A pointer to a location to receive the next 4 instructions to be
 *		processed.
 *
 * Return Value:
 *	TRUE if the instruction is found in the instruction cache.
 *	FALSE if there was an ITB miss.
 */
AXP_CACHE_FETCH AXP_ICacheFetch(AXP_21264_CPU *cpu,
								AXP_PC pc,
								AXP_INS_LINE *next)
{
	AXP_CACHE_FETCH retVal = WayMiss;
	AXP_ICACHE_TAG_IDX addr;
	u64 tag;
	u32 ii;
	u32 index, set, offset;

	/*
	 * First, get the information from the supplied parameters we need to
	 * search the Icache correctly.
	 */
	addr.pc = pc;
	index = addr.insAddr.index;
	set = addr.insAddr.set;
	tag = addr.insAddr.tag;
	offset = addr.insAddr.offset % AXP_ICACHE_LINE_INS;
	switch(cpu->iCtl.ic_en)
	{

		/*
		 * Just set 0
		 */
		case 1:
			set = 0;
			break;

		/*
		 * Just set 1
		 */
		case 2:
			set = 1;
			break;

		/*
		 * Both set 1 and 2
		 */
		default:
			break;
	}

	/*
	 * Now, search through the Icache for the information we have been asked to
	 * return.
	 */
	if ((cpu->iCache[index][set].tag == tag) &&
		(cpu->iCache[index][set].vb == 1))
	{
		AXP_PC tmpPC = pc;

		/*
		 * Extract out the next 4 instructions and return these to the
		 * caller.  While we are here will do some predecoding of the
		 * instructions.
		 */
		for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
		{
			next->instructions[ii] =
				cpu->iCache[index][set].instructions[offset + ii];
			next->instrType[ii] = AXP_InstructionFormat(next->instructions[ii]);
			next->instrPC[ii] = tmpPC;
			tmpPC.pc++;
		}

		/*
		 * Line (index) and Set prediction, at this point, should indicate the
		 * next instruction to be read from the cache (it could be the current
		 * list and set).  The following logic is used:
		 *
		 * If there are instructions left in the current cache line, then we
		 * 		use the same line and set
		 * Otherwise,
		 * 	If we are only utilizing a single set, then we go to the next line
		 * 		and the same set
		 * 	Otherwise,
		 * 		If we are at the first set, then we go to the next set on the
		 * 			same line
		 * 		Otherwise,
		 * 			We go to the next line and the first set.
		 *
		 * NOTE: When the prediction code is run, it may recalculate these
		 * 		 values.
		 */
		if ((offset + AXP_NUM_FETCH_INS + 1) < AXP_ICACHE_LINE_INS)
		{
			next->linePrediction = index;			/* same line */
			next->setPrediction = set;				/* same set */
		}
		else
		{
			if ((cpu->iCtl.ic_en == 1) || (cpu->iCtl.ic_en == 2))
			{
				next->linePrediction = index + 1;	/* next line */
				next->setPrediction = set;			/* only set */
			}
			else if (set == 0)
			{
				next->linePrediction = index;		/* same line */
				next->setPrediction = 1;			/* last set */
			}
			else
			{
				next->linePrediction = index + 1;	/* next line */
				next->setPrediction = 0;			/* first set */
			}
		}
		retVal = Hit;
	}

	/*
	 * If we had an Icache miss, go look in the ITB.  If we get an ITB miss,
	 * this will cause an exception to be generated.
	 */
	if (retVal == WayMiss)
	{
		u64 tagITB;
		u64 pages;
		AXP_IBOX_ITB_TAG *itbTag = (AXP_IBOX_ITB_TAG *) &pc;

		tag = itbTag->tag;

		/*
		 * Search through the ITB for the address we are looking to see if
		 * there is an ITB entry that maps the current PC.  If so, then we
		 * have a Miss.  Otherwise, it is a WayMiss (this will cause the CPU
		 * to have to add a new ITB entry (with matching PTE entry) so that
		 * the physical memory location can be mapped to the virtual and the
		 * instructions loaded into the instruction cache for execution.
		 *
		 * NOTE:	The gh field is a 2 bit field that represents the
		 * 			following:
		 *
		 * 					System Page Size (SPS)
		 * 		gh		8KB		16KB	32KB	64KB	From SPS
		 * 		-------------------------------------	-----------------------
		 * 		00 (0)	  8KB	 16KB	 32KB	 64KB	  1x [8^0 = 1 << (0*3)]
		 * 		01 (1)	 64KB	128KB	256KB	  2MB	  8x [8^1 = 1 << (1*3)]
		 * 		10 (2)	512KB	  1MB	  2MB	 64MB	 64x [8^2 = 1 << (2*3)]
		 * 		11 (3)	  4MB	  8MB	 16MB	512MB	512x [8^3 = 1 << (3*3)]
		 */
		for (ii = cpu->itbStart; ii < cpu->itbEnd; ii++)
		{
			tagITB = cpu->itb[ii].tag.tag;
			pages = AXP_21264_PAGE_SIZE * (1 << (cpu->itb[ii].pfn.gh * 3));

			/*
			 * The ITB can map 1, 8, 64 or 512 contiguous 8KB pages, so the
			 * ITB.tag is the base address and ITB.tag + ITB.mapped is the
			 * address after the last byte mapped.
			 */
			if ((tagITB <= tag) &&
				((tagITB + pages) > tag) &&
				(cpu->itb[ii].vb == 1))
			{

				/*
				 * OK, the page is mapped in the ITB, but not in the Icache.
				 * We need to ask the Cbox to load the next set of pages
				 * into the Icache.
				 */
				// AXP_IcacheFill(cpu, pc);
				retVal = Miss;
				break;
			}
		}
	}

	/*
	 * Return what we did or did not find back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_ICacheAdd
 *  When a entry needs to be added to the Icache, this function is called.  If
 *  all the sets currently have  a valid reecord in them, then we need to look
 *  at the Least Recently Used (LRU) record with the same index and evict that
 *  one.  If one of them is not vvalid, then insert this record into that
 *  location and update the LRU list.
 *
 * Input Parameters:
 * 	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *	pc:
 *		A value that represents the program counter of the instruction being
 *		requested.
 *	nextInst:
 *		A pointer to an array off the next set  of instructions to insert into
 *		the Icache.
 *	itb:
 *		A pointer to the ITB entry associated with the instructions to be added
 *		to the Icache.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_ICacheAdd(AXP_21264_CPU *cpu,
				   AXP_PC pc,
				   AXP_INS_FMT *nextInst,
				   AXP_ICACHE_ITB *itb)
{
	AXP_ICACHE_TAG_IDX addr;
	u32 ii;
	u32 index, set, tag;

	/*
	 * First, get the information from the supplied parameters we need to
	 * search the Icache correctly.
	 */
	addr.pc = pc;
	set = addr.insAddr.set;
	index = addr.insAddr.index;
	tag = addr.insAddr.tag;
	switch(cpu->iCtl.ic_en)
	{

		/*
		 * Just set 0
		 */
		case 1:
			set = 0;
			break;

		/*
		 * Just set 1
		 */
		case 2:
			set = 1;
			break;

		/*
		 * Anything else, we are using both sets.
		 */
		default:
			break;
	}

	/*
	 * If there is something in the cache, we need to evict it first.  There is
	 * not need to evict the ITB entry.  For one, the ITB entries are allocated
	 * round-robin.  As a result, entries come and go as needed.  When an ITB
	 * entry gets overwritten, then this may evict Icache entries.  Finally,
	 * ITB entries map more than one Icache entry.  Evicting one Icache entry
	 * should not effect the ITB entry.
	 */
	if (cpu->iCache[index][set].vb == 1)
		cpu->iCache[index][set].vb = 0;

	/*
	 * See if we found a place to hold this line/block.
	 */
	cpu->iCache[index][set].kre = itb->pfn.kre;
	cpu->iCache[index][set].ere = itb->pfn.ere;
	cpu->iCache[index][set].sre = itb->pfn.sre;
	cpu->iCache[index][set].ure = itb->pfn.ure;
	cpu->iCache[index][set]._asm = itb->pfn._asm;
	cpu->iCache[index][set].asn = 0;				// TODO: Need ASN
	cpu->iCache[index][set].pal = pc.pal;
	cpu->iCache[index][set].vb = 1;
	cpu->iCache[index][set].tag = tag;
	for (ii = 0; ii < AXP_ICACHE_LINE_INS; ii++)
		cpu->iCache[index][set].instructions[ii] = nextInst[ii];

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_ITBAdd
 * 	This  function should only be called as the result of an ITB Miss.  As such
 * 	we just have to find the next location to enter the next item.  If we are
 * 	using a currently used field, we'll need to evict all the associated Icache
 * 	items with the same tag.
 */
void AXP_ITBAdd(AXP_21264_CPU *cpu,
				AXP_IBOX_ITB_TAG itbTag,
				AXP_IBOX_ITB_PTE *itbPTE)
{
	unsigned int ii, jj;
	u32 setStart, setEnd;

	/*
	 * First, determine which cache sets are in use (0, 1, or both).
	 */
	switch(cpu->iCtl.ic_en)
	{

		/*
		 * Just set 0
		 */
		case 1:
			setStart = 0;
			setEnd = 1;
			break;

		/*
		 * Just set 1
		 */
		case 2:
			setStart = 1;
			setEnd = AXP_2_WAY_ICACHE;
			break;

		/*
		 * Both set 1 and 2
		 */
		case 0:		/* This is an invalid value, but... */
		case 3:
			setStart = 0;
			setEnd = AXP_2_WAY_ICACHE;
			break;
	}

	/*
	 * The ITB array is utilized in a round-robin fashion.  See if the next
	 * entry in the array is already used.  If so, evict the associate Icache
	 * entries.
	 */
	if (cpu->itb[cpu->itbEnd].vb == 1)
	{
		for (ii = 0; ii < AXP_21264_ICACHE_SIZE; ii++)
			for (jj = setStart; jj <setEnd; jj++)
			{
				if ((cpu->iCache[ii][jj].vb == 1) &&
					(cpu->iCache[ii][jj].tag == itbTag.tag))
				{

					/*
					 * Now invalidate the iCache entry;
					 */
					cpu->iCache[ii][jj].vb = 0;
				}
			}
	}

	/*
	 * We are now able to add the ITB entry.
	 */
	cpu->itb[cpu->itbEnd].vb = 1;
	memcpy(&cpu->itb[cpu->itbEnd].tag, &itbTag, sizeof(AXP_IBOX_ITB_TAG));
	memcpy(&cpu->itb[cpu->itbEnd].pfn, itbPTE, sizeof(AXP_IBOX_ITB_PTE));
	cpu->itbEnd++;

	/*
	 * If we have reached the end of the array, then reset it to the beginning.
	 */
	if (cpu->itbEnd == AXP_TB_LEN)
		cpu->itbEnd = 0;

	/*
	 * The itbEnd equals itbStart only in 2 instances.  One, when there is
	 * nothing in the itb array.  And two, when an entry was added onto an
	 * existing entry (which we just removed above).
	 */
	if (cpu->itbEnd == cpu->itbStart)
		cpu->itbStart++;

	/*
	 * If we have reached the end of the array, then wrap to the beginning.
	 */
	if (cpu->itbStart == AXP_TB_LEN)
		cpu->itbStart = 0;

	/*
	 * Return back to the caller.
	 */
	return;
}

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
	AXP_REG_DECODE decodeRegisters;

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
	decodedInstr->type = insDecode[decodedInstr->opcode].type;
	decodeRegisters = insDecode[decodedInstr->opcode].registers;
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
			break;

		case AXP_REG_FB:
			decodedInstr->aDest = next->instructions[nextInstr].fp.fb;
			break;

		case AXP_REG_FC:
			decodedInstr->aDest = next->instructions[nextInstr].fp.fc;
			break;

		default:
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
			break;

		case AXP_REG_FB:
			decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fb;
			break;

		case AXP_REG_FC:
			decodedInstr->aSrc1 = next->instructions[nextInstr].fp.fc;
			break;

		default:
			decodedInstr->aSrc1 = AXP_UNMAPPED_REG;
			break;
	}

	/*
	 * Decode destination register
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
			break;

		case AXP_REG_FB:
			decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fb;
			break;

		case AXP_REG_FC:
			decodedInstr->aSrc2 = next->instructions[nextInstr].fp.fc;
			break;

		default:
			decodedInstr->aSrc2 = AXP_UNMAPPED_REG;
			break;
	}

	/*
	 * We need to rename the architectural registers to physical
	 * registers, now that we know which one, if any, is the
	 * destination register and which one(s) is(are) the source
	 * register(s).
	 */
	AXP_RenameRegisters(cpu, decodedInstr, decodeRegisters.raw);
	decodedInstr->pc = next->instrPC[nextInstr];

	/*
	 * Return back to the caller.
	 */
	return;
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
inline u16 AXP_RegisterDecodingOpcode11(AXP_INS_FMT instr)
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
inline u16 AXP_RegisterDecodingOpcode14(AXP_INS_FMT instr)
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
inline u16 AXP_RegisterDecodingOpcode15_16(AXP_INS_FMT instr)
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
inline u16 AXP_RegisterDecodingOpcode17(AXP_INS_FMT instr)
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
inline u16 AXP_RegisterDecodingOpcode18(AXP_INS_FMT instr)
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
inline u16 AXP_RegisterDecodingOpcode1c(AXP_INS_FMT instr)
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
 *	the freelist.  We also differentiate between integer and floating point
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
static void AXP_RenameRegisters(AXP_21264_CPU *cpu, AXP_INSTRUCTION *decodedInstr, u16 decodedRegs)
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
 * 			number of entrys that can be in the IQ, this is not necessarily a
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
 * 			number of entrys that can be in the FQ, this is not necessarily a
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
