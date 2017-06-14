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
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_ICache.h"
#include "AXP_21264_Ibox.h"

struct instructDecode
{
	AXP_INS_TYPE	format;
	AXP_OPER_TYPE	type;
	u16				registers;
	u16				res;	/* 32-bit align 								*/
};

/*
 * The following module specific variable contains a list of instruction and
 * operation types that are used to decode the Alpha AXP instructions.  The
 * opcode is the index into this array.
 */
static struct instructDecode insDecode[] =
{
/* Format	Type	Registers   					Opcode	Mnemonic	Description 					*/
	{Pcd,	Other,	0},								/* 00		CALL_PAL	Trap to PALcode					*/
	{Res,	Other,	0},								/* 01					Reserved for Digital			*/
	{Res,	Other,	0},								/* 02					Reserved for Digital			*/
	{Res,	Other,	0},								/* 03					Reserved for Digital			*/
	{Res,	Other,	0},								/* 04					Reserved for Digital			*/
	{Res,	Other,	0},								/* 05					Reserved for Digital			*/
	{Res,	Other,	0},								/* 06					Reserved for Digital			*/
	{Res,	Other,	0},								/* 07					Reserved for Digital			*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},		/* 08		LDA			Load address					*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},		/* 09		LDAH		Load address high				*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},		/* 0A		LDBU		Load zero-extended byte			*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},		/* 0B		LDQ_U		Load unaligned quadword			*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},		/* 0C		LDWU		Load zero-extended word			*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},		/* 0D		STW			Store word						*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},		/* 0E		STB			Store byte						*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},		/* 0F		STQ_U		Store unaligned quadword		*/
	{Opr,	Other,	AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB},	/* 10		ADDL		Add longword					*/
// TODO: AMASK (opcode: 11) has 1 source (Rb) and 1 destination (Rc)
// TODO: IMPLVER (opcode: 11) has 1 destination (Rc)
	{Opr,	Other,	AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB},	/* 11		AND			Logical product					*/
	{Opr,	Other,	AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB},	/* 12		MSKBL		Mask byte low					*/
	{Opr,	Other,	AXP_DEST_RC|AXP_SRC1_RA|AXP_SRC2_RB},	/* 13		MULL		Multiply longword				*/
// TODO: SQRTx (opcode: 14) has 1 source (Fb) and 1 destination (Fc)
	{FP,	Other,	AXP_DEST_FC|AXP_SRC1_RA},	/* 14		ITOFS		Int to float move, S_floating	*/
// TODO: CVTGQ (opcode: 15) only has 1 source (Fb) and one destination (Fc)
// TODO: CVTQx (opcode: 15) only has 1 source (Fb) and one destination (Fc)
// TODO: CVTxy (opcode: 15) only has 1 source (Fb) and one destination (Fc)
	{FP,	Other,	AXP_DEST_FC|AXP_SRC1_FA|AXP_SRC2_FB},	/* 15		ADDF		Add F_floating					*/
// TODO: CVTTQ (opcode: 16) only has 1 source (Fb) and one destination (Fc)
// TODO: CVTQy (opcode: 16) only has 1 source (Fb) and one destination (Fc)
// TODO: CVTST (opcode: 16) only has 1 source (Fb) and one destination (Fc)
// TODO: CVTTS (opcode: 16) only has 1 source (Fb) and one destination (Fc)
	{FP,	Other,	AXP_DEST_FC|AXP_SRC1_FA|AXP_SRC2_FB},	/* 16		ADDS		Add S_floating					*/
// TODO: CVTxy (opcode: 17) only has sources (Fb, Fc) and no destination
// TODO: MF_FPCR (opcode: 17) only has a destination (Fa)
// TODO: MT_FPCR (opcode: 17) only as a source (Fa)
	{FP,	Other,	AXP_DEST_FC|AXP_SRC1_FA|AXP_SRC2_FB},	/* 17		CVTLQ		Convert longword to quadword	*/
// TODO: EXCB (opcode 18) does not have any registers
// TODO: MB (opcode: 18) does not have any registers
// TODO: WMB (opcode: 18) does not have any registers
// TODO: RPCC (opcode: 18) has 1 destination Ra and 1 source Rb
// TODO: Rx (opcode: 18) has 1 destination Ra
// TODO: TRAPB (opcode: 18) does not have any registers
	{Mfc,	Other,	AXP_SRC1_RB},	/* 18		TRAPB		Trap barrier					*/
	{PAL,	Load},	/* 19		HW_MFPR		Reserved for PALcode			*/
	{Mbr,	Other,	AXP_DEST_RA|AXP_SRC1_RB},	/* 1A		JMP			Jump							*/
	{PAL,	Load},	/* 1B		HW_LD		Reserved for PALcode			*/
// TODO: FTOIx (opcode: 1C has 1 destination (Rc) and 1 source (Fa)
	{Cond,	Other,	AXP_DEST_RC|AXP_SRC1_RB},	/* 1C		SEXTB		Sign extend byte				*/
	{PAL,	Store},	/* 1D		HW_MTPR		Reserved for PALcode			*/
	{PAL,	Other},	/* 1E		HW_REI		Reserved for PALcode			*/
	{PAL,	Store},	/* 1F		HW_ST		Reserved for PALcode			*/
	{Mem,	Load,	AXP_DEST_FA|AXP_SRC1_RB},	/* 20 		LDF			Load F_floating					*/
	{Mem,	Load,	AXP_DEST_FA|AXP_SRC1_RB},	/* 21		LDG			Load G_floating					*/
// TODO: PREFETCH_M (opcode: 22) has 1 source (Rb)
	{Mem,	Load,	AXP_DEST_FA|AXP_SRC1_RB},	/* 22		LDS			Load S_floating					*/
// TODO: PREFETCH_EN (opcode: 23) has 1 source (Rb)
	{Mem,	Load,	AXP_DEST_FA|AXP_SRC1_RB},	/* 23		LDT			Load T_floating					*/
	{Mem,	Store,	AXP_SRC1_FA|AXP_SRC2_RB},	/* 24		STF			Store F_floating				*/
	{Mem,	Store,	AXP_SRC1_FA|AXP_SRC2_RB},	/* 25		STG			Store G_floating				*/
	{Mem,	Store,	AXP_SRC1_FA|AXP_SRC2_RB},	/* 26		STS			Store S_floating				*/
	{Mem,	Store,	AXP_SRC1_FA|AXP_SRC2_RB},	/* 27		STT			Store T_floating				*/
// TODO: PREFETCH (opcode: 28) has 1 source (Rb)
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},	/* 28		LDL			Load sign-extended longword		*/
// TODO: PREFETCH_EN (opcode: 29) has 1 source (Rb)
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},	/* 29		LDQ			Load quadword					*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},	/* 2A		LDL_L		Load sign-extended long locked	*/
	{Mem,	Load,	AXP_DEST_RA|AXP_SRC1_RB},	/* 2B		LDQ_L		Load quadword locked			*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},	/* 2C		STL			Store longword					*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},	/* 2D		STQ			Store quadword					*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},	/* 2E		STL_C		Store longword conditional		*/
	{Mem,	Store,	AXP_SRC1_RA|AXP_SRC2_RB},	/* 2F		STQ_C		Store quadword conditional		*/
	{Bra,	Other,	AXP_DEST_RA},	/* 30		BR			Unconditional branch			*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 31		FBEQ		Floating branch if = zero		*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 32		FBLT		Floating branch if < zero		*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 33		FBLE		Floating branch if <= zero		*/
	{Mbr,	Other,	AXP_DEST_RA},	/* 34		BSR			Branch to subroutine			*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 35		FBNE		Floating branch if != zero		*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 36		FBGE		Floating branch if >=zero		*/
	{FPBra,	Other,	AXP_SRC1_FA},	/* 37		FBGT		Floating branch if > zero		*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 38		BLBC		Branch if low bit clear			*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 39		BEQ			Branch if = zero				*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 3A		BLT			Branch if < zero				*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 3B		BLE			Branch if <= zero				*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 3C		BLBS		Branch if low bit set			*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 3D		BNE			Branch if != zero				*/
	{Bra,	Other,	AXP_SRC1_RA},	/* 3E		BGE			Branch if >= zero				*/
	{Bra,	Other,	AXP_SRC1_RA}	/* 3F		BGT			Branch if > zero				*/
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
 * AXP_InstructionType
 *	This function is called to determine what type of instruction is specified
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
AXP_INS_TYPE AXP_InstructionType(AXP_INS_FMT inst)
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
			next->instrType[ii] = AXP_InstructionType(next->instructions[ii]);
			next->instrPC[ii] = tmpPC;
			tmpPC.pc++;
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
		 * 						System Page Size (SPS)
		 * 			gh		8KB		16KB	32KB	64KB	From SPS
		 * 			-------------------------------------	-----------------------
		 * 			00 (0)	  8KB	 16KB	 32KB	 64KB	  1x [8^0 = 1 << (0*3)]
		 * 			01 (1)	 64KB	128KB	256KB	  2MB	  8x [8^1 = 1 << (1*3)]
		 * 			10 (2)	512KB	  1MB	  2MB	 64MB	 64x [8^2 = 1 << (2*3)]
		 * 			11 (3)	  4MB	  8MB	 16MB	512MB	512x [8^3 = 1 << (3*3)]
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
 *
 * TODO: Do we want to put the instructions in the cache as well?
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
 *	cpu.[i|f]q[[i|f]qEnd]:	-- implicit
 *		Each of the 4 instructions get queued to the end of the appropriate
 *		instruction queue (IQ or FQ).
 *
 * Return value:
 * 	true:
 * 		Instructions successfully queued up.
 * 	false:
 * 		At least one of the instruction queues is full.  Stall.
 */
bool AXP_Decode_Rename(AXP_21264_CPU *cpu, AXP_INS_LINE *next)
{
	AXP_INSTRUCTION decodedInstr[AXP_NUM_FETCH_INS];
	int ii;
	u32 intCnt, fpCnt;
	bool retVal = true;

	/*
	 * Determine if there  is enough space to queue up the next set of
	 * instructions.
	 */
	intCnt = fpCnt = 0;
	for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
	{
		if ((next->instrType[ii] == FP) || (next->instrType[ii] == FPBra))
			fpCnt++;
		else
			intCnt++;
	}
	if (intCnt > (AXP_IQ_LEN - AXP_CQUE_COUNT(cpu->iq)))
	{
		printf("DECEMU-W-IQINSUF, insufficient space in Integer Instruction Queue.\n");
		retVal = false;
	}
	else if (fpCnt > (AXP_IQ_LEN - AXP_CQUE_COUNT(cpu->fq)))
	{
		printf("DECEMU-W-FQINSUF, insufficient space in Floating-point Instruction Queue.\n");
		retVal = false;
	}

	/*
	 * Loop through each of the instructions.  Instructions are inserted
	 * into the queue at the bottom and
	 */
	if (retVal == true)
	{
		for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
		{
			/*
			 * Assign a unique ID to this instruction.
			 */
			decodedInstr[ii].uniqueID = cpu->instrCounter++;

			/*
			 * First, decode the instruction.
			 */
			decodedInstr[ii].format = next->instrType[ii];
			decodedInstr[ii].opcode = next->instructions[ii].pal.opcode;
			decodedInstr[ii].type = insDecode[decodedInstr[ii].opcode].type;
			switch (decodedInstr[ii].format)
			{
				case Pcd:	/* PAL_CODE instruction */
					decodedInstr[ii].function = next->instructions[ii].pal.palcode_func;
					decodedInstr[ii].displacement = 0;
					decodedInstr[ii].aSrc1 = AXP_UNMAPPED_REG;
					decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
					decodedInstr[ii].aDest = AXP_UNMAPPED_REG;
					break;

				case Opr:	/* Operation instruction */
					decodedInstr[ii].function = next->instructions[ii].oper1.func;
					decodedInstr[ii].displacement = 0;
					if (next->instructions[ii].oper1.fmt == 0)
					{
						decodedInstr[ii].literal = 0;
						decodedInstr[ii].useLiteral = false;
						decodedInstr[ii].aSrc1 = next->instructions[ii].oper1.rb;
						decodedInstr[ii].aSrc2 = next->instructions[ii].oper1.rc;
						decodedInstr[ii].aDest = next->instructions[ii].oper1.ra;
					}
					else
					{
						decodedInstr[ii].literal = next->instructions[ii].oper2.lit;
						decodedInstr[ii].useLiteral = true;
						decodedInstr[ii].aSrc1 = next->instructions[ii].oper1.rc;
						decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
						decodedInstr[ii].aDest = next->instructions[ii].oper1.ra;
					}
					break;

				case Mem:
					decodedInstr[ii].function = 0;
					decodedInstr[ii].displacement = next->instructions[ii].mem.mem.disp;
					if ((decodedInstr[ii].type == Load) ||
						(decodedInstr[ii].type == Jump))
					{
						decodedInstr[ii].aSrc1 = next->instructions[ii].mem.rb;
						decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
						decodedInstr[ii].aDest = next->instructions[ii].mem.ra;
					}
					else	/* Store operations */
					{
						decodedInstr[ii].aSrc1 = next->instructions[ii].mem.ra;
						decodedInstr[ii].aSrc2 = next->instructions[ii].mem.ra;
						decodedInstr[ii].aDest = AXP_UNMAPPED_REG;
					}
					break;

				case Mfc:	/* Memory with function-code instruction */
					decodedInstr[ii].function = next->instructions[ii].mem.mem.func;
					decodedInstr[ii].displacement = 0;
					decodedInstr[ii].aSrc1 = next->instructions[ii].mem.rb;
					decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
					decodedInstr[ii].aDest = next->instructions[ii].mem.ra;
					break;

				case FP:	/* Floating-point instruction */
					decodedInstr[ii].function = next->instructions[ii].fp.func;
					decodedInstr[ii].displacement = 0;
					decodedInstr[ii].aSrc1 = next->instructions[ii].fp.fb;
					decodedInstr[ii].aSrc2 = next->instructions[ii].fp.fc;
					decodedInstr[ii].aDest = next->instructions[ii].fp.fa;
					break;

				case Bra:
				case FPBra:
					decodedInstr[ii].function = 0;
					decodedInstr[ii].displacement = next->instructions[ii].br.branch_disp;
					if ((decodedInstr[ii].opcode == BR) ||
						(decodedInstr[ii].opcode == BSR))
					{
						decodedInstr[ii].aSrc1 = AXP_UNMAPPED_REG;
						decodedInstr[ii].aDest = next->instructions[ii].br.ra;
					}
					else
					{
						decodedInstr[ii].aSrc1 = next->instructions[ii].br.ra;
						decodedInstr[ii].aDest = AXP_UNMAPPED_REG;
					}
					decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
					break;

				default:	/* No displacement or function */
					decodedInstr[ii].displacement = 0;
					decodedInstr[ii].function = 0;
					decodedInstr[ii].aSrc1 = AXP_UNMAPPED_REG;
					decodedInstr[ii].aSrc2 = AXP_UNMAPPED_REG;
					decodedInstr[ii].aDest = AXP_UNMAPPED_REG;
					break;
			}
			decodedInstr[ii].pc = next->instrPC[ii];

			/*
			 * Finally, we have to rename the architectural registers to the
			 * physical registers.
			 */
		}
	}
	return(retVal);
}
