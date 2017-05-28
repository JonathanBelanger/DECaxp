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
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_ICache.h"

/*
 * The following module specific variable contains a list of instruction types
 * that are used to decode the Alpha AXP instructions.  The opcode is the index
 * into this array.
 */
static AXP_INS_TYPE instructionType[] =
{
/* Format	   Opcode	Mnemonic	Description 							*/
	Pcd,	/* 00		CALL_PAL	Trap to PALcode							*/
	Res,	/* 01					Reserved for Digital					*/
	Res,	/* 02					Reserved for Digital					*/
	Res,	/* 03					Reserved for Digital					*/
	Res,	/* 04					Reserved for Digital					*/
	Res,	/* 05					Reserved for Digital					*/
	Res,	/* 06					Reserved for Digital					*/
	Res,	/* 07					Reserved for Digital					*/
	Mem,	/* 08		LDA			Load address							*/
	Mem,	/* 09		LDAH		Load address high						*/
	Mem,	/* 0A		LDBU		Load zero-extended byte					*/
	Mem,	/* 0B		LDQ_U		Load unaligned quadword					*/
	Mem,	/* 0C		LDWU		Load zero-extended word					*/
	Mem,	/* 0D		STW			Store word								*/
	Mem,	/* 0E		STB			Store byte								*/
	Mem,	/* 0F		STQ_U		Store unaligned quadword				*/
	Opr,	/* 10		ADDL		Add longword							*/
	Opr,	/* 11		AND			Logical product							*/
	Opr,	/* 12		MSKBL		Mask byte low							*/
	Opr,	/* 13		MULL		Multiply longword						*/
	FP,		/* 14		ITOFS		Integer to floating move, S_floating	*/
	FP,		/* 15		ADDF		Add F_floating							*/
	FP,		/* 16		ADDS		Add S_floating							*/
	FP,		/* 17		CVTLQ		Convert longword to quadword			*/
	Mfc,	/* 18		TRAPB		Trap barrier							*/
	PAL,	/* 19		HW_MFPR		Reserved for PALcode					*/
	Mbr,	/* 1A		JMP			Jump									*/
	PAL,	/* 1B		HW_LD		Reserved for PALcode					*/
	Cond,	/* 1C		SEXTB		Sign extend byte						*/
	PAL,	/* 1D		HW_MTPR		Reserved for PALcode					*/
	PAL,	/* 1E		HW_REI		Reserved for PALcode					*/
	PAL,	/* 1F		HW_ST		Reserved for PALcode					*/
	Mem,	/* 20 		LDF			Load F_floating							*/
	Mem,	/* 21		LDG			Load G_floating							*/
	Mem,	/* 22		LDS			Load S_floating							*/
	Mem,	/* 23		LDT			Load T_floating							*/
	Mem,	/* 24		STF			Store F_floating						*/
	Mem,	/* 25		STG			Store G_floating						*/
	Mem,	/* 26		STS			Store S_floating						*/
	Mem,	/* 27		STT			Store T_floating						*/
	Mem,	/* 28		LDL			Load sign-extended longword				*/
	Mem,	/* 29		LDQ			Load quadword							*/
	Mem,	/* 2A		LDL_L		Load sign-extended longword locked		*/
	Mem,	/* 2B		LDQ_L		Load quadword locked					*/
	Mem,	/* 2C		STL			Store longword							*/
	Mem,	/* 2D		STQ			Store quadword							*/
	Mem,	/* 2E		STL_C		Store longword conditional				*/
	Mem,	/* 2F		STQ_C		Store quadword conditional				*/
	Bra,	/* 30		BR			Unconditional branch					*/
	Bra,	/* 31		FBEQ		Floating branch if = zero				*/
	Bra,	/* 32		FBLT		Floating branch if < zero				*/
	Bra,	/* 33		FBLE		Floating branch if <= zero				*/
	Mbr,	/* 34		BSR			Branch to subroutine					*/
	Bra,	/* 35		FBNE		Floating branch if != zero				*/
	Bra,	/* 36		FBGE		Floating branch if >=zero				*/
	Bra,	/* 37		FBGT		Floating branch if > zero				*/
	Bra,	/* 38		BLBC		Branch if low bit clear					*/
	Bra,	/* 39		BEQ			Branch if = zero						*/
	Bra,	/* 3A		BLT			Branch if < zero						*/
	Bra,	/* 3B		BLE			Branch if <= zero						*/
	Bra,	/* 3C		BLBS		Branch if low bit set					*/
	Bra,	/* 3D		BNE			Branch if != zero						*/
	Bra,	/* 3E		BGE			Branch if >= zero						*/
	Bra		/* 3F		BGT			Branch if > zero						*/
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
		retVal = instructionType[inst.pal.opcode];
	
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
 * AXP_InitializeICache
 *	This function is called to initialize the instruction cache..
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_InitializeICache(AXP_21264_CPU *cpu)
{
	int ii, jj, kk;

	for (ii = 0; ii < AXP_21264_ICACHE_SIZE; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_ICACHE; jj++)
		{
			cpu->iCache[ii][jj].kre = 0;
			cpu->iCache[ii][jj].ere = 0;
			cpu->iCache[ii][jj].sre = 0;
			cpu->iCache[ii][jj].ure = 0;
			cpu->iCache[ii][jj]._asm = 0;
			cpu->iCache[ii][jj].asn = 0;
			cpu->iCache[ii][jj].pal = 0;
			cpu->iCache[ii][jj].replace = jj;
			cpu->iCache[ii][jj].tag = 0;
			for (kk = 0; kk < AXP_ICACHE_LINE_INS; kk++)
				cpu->iCache[ii][jj].instructions[kk].instr = 0;	/* HALT */
		}
	}
	return;
}

/*
 * AXP_ICacheLookup
 *	This function is called to look up an instruction in the instruction cache
 *	using the address of the instruction we are looking for to determine the
 *	index into the cache and the instruction.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing all the fields needed to
 *		emulate an Alpha AXP 21264 CPU.
 *	pc:
 *		A value that represents the program counter of the instruction being
 *		requested.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	TRUE if the instruction is found in the instruction cache.
 *	FALSE if the instruction was not found.
 *
 * TODO:	This is going to have to be broken up.  The way the code is now, it
 *			inserts a cache entry into and probably should not.  Also, this
 *			should return the next 4 pre-decoded instructions to the caller, not
 *			just a true/false.
 */
bool AXP_ICacheLookup(AXP_21264_CPU *cpu, AXP_PC pc)
{
	AXP_ICACHE_TAG_IDX address;
	AXP_ICACHE_TAG_IDX currAddr;
	int ii, jj;
	int index, tag;
	bool retVal = false;	/* Assume Cache Miss */

	/*
	 * We want to extract the index and tag information from the instruction
	 * virtual address (PC).
	 */
	address.pc = pc;
	index = address.insAddr.index;
	tag = address.insAddr.tag;

	/*
	 * Search the icache for the instruction for which we are looking.  If we
	 * find it, then we have a Cache Hit.
	 */
	for (ii = 0; ii < AXP_2_WAY_ICACHE; ii++)
		if ((cpu->iCache[index][ii].tag == tag) &&
			(cpu->iCache[index][ii].vb == 1))
			{
				for (jj = ii + 1; jj < AXP_2_WAY_ICACHE; jj++)
					cpu->iCache[index][jj - 1].replace =
						cpu->iCache[index][jj].replace;
				cpu->iCache[index][AXP_2_WAY_ICACHE - 1].replace = ii;
				retVal = true;	/* Cache Hit */
				break;
			}

	/*
	 * Cache Miss (we did not find it in the above search)
	 */
	if (retVal == false)
	{

		/*
		 * Item 0 at the current index, is the least recently used, so it is
		 * the one to be replaced.  Find the first place in the cache where the
		 * valid bit is clear and replace that slot with our new information.
		 */
		for (ii = 0; ii < AXP_2_WAY_ICACHE; ii++)
			if (cpu->iCache[index][ii].vb == 0)
			{
				for (jj = 1; jj < ii; jj++)
				{
					currAddr.insAddr.tag = cpu->iCache[index][jj].tag;
					cpu->iCache[index][jj - 1].tag = currAddr.insAddr.tag;
				}
				if (ii == AXP_2_WAY_ICACHE)
				{
					cpu->iCache[index][ii - 1].tag = address.insAddr.tag;
					cpu->iCache[index][ii - 1].vb = 1;
				}
				else
				{
					cpu->iCache[index][ii].tag = address.insAddr.tag;
					cpu->iCache[index][ii].vb = 1;
				}
				break;
			}
	}

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}
