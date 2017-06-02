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
 */
#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_ICache.h"
#include "AXP_21264_Ibox.h"

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
								AXP_IBOX_INS_LINE *next)
{
	AXP_CACHE_FETCH retVal = WayMiss;
	AXP_ICACHE_TAG_IDX addr;
	u32 ii, jj;
	u32 index, tag, setStart, setEnd;
	u32 offset;

	/*
	 * First, get the information from the supplied parameters we need to
	 * search the Icache correctly.
	 */
	addr.pc = pc;
	index = addr.insAddr.index;
	tag = addr.insAddr.tag;
	offset = addr.insAddr.offset / AXP_INSTRUCTION_SIZE;
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
	 * Now, search through the Icache for the information we have been asked to
	 * return.
	 */
	for (ii = setStart; ii < setEnd; ii++)
	{
		if ((cpu->iCache[index][ii].tag == tag) &&
			(cpu->iCache[index][ii].vb == 1))
		{

			/*
			 * Extract out the next 4 instructions and return these to the
			 * caller.  While we are here will do some predecoding of the
			 * instructions.
			 */
			for (jj = 0; jj < AXP_IBOX_INS_FETCHED; jj++)
			{
				next->instructions[jj] =
					cpu->iCache[index][ii].instructions[offset + jj];
				next->instrType[jj] = AXP_InstructionType(next->instructions[jj]);
			}

			/*
			 * The return value of the next call is not needed.  Since the
			 * instructions were already in the Icache, the entry for the
			 * Least Recently Used (LRU) index/set is already there.  The
			 * only reason a false would be returned was if an attempt was
			 * made to actually add an item that was not there and there was
			 * no room.  Therefore, we can ignore the return value.
			 */
			(void) AXP_LRUAdd(cpu->iCacheLRU,
							  AXP_21264_ICACHE_SIZE,
							  &cpu->iCacheLRUIdx,
							  index,
							  ii);
			retVal = Hit;
		}
	}

	/*
	 * If we had an Icache miss, go look in the ITB.  If we get an ITB miss,
	 * this will cause an exception to be generated.
	 */
	if (retVal == WayMiss)
	{
		u64 tagITB;
		u32 pages;
		AXP_IBOX_ITB_TAG *itbTag = (AXP_IBOX_ITB_TAG *) &pc;

		tag = itbTag->tag;

		/*
		 * Search through the ITB for the address we are looking to get,
		 */
		for (ii = cpu->itbStart; ii < cpu->itbEnd; ii++)
		{
			tagITB = cpu->itb[ii].tag.tag;
			pages = cpu->itb[ii].mapped;

			/*
			 * The ITB can map 1, 8, 64 or 512 contiguous 8KB pages, so the
			 * ITB.tag is the base address and ITB.tag + ITB.mapped is the
			 * address after the last byte mapped.
			 */
			if ((tagITB >= tag) &&
				((tagITB + pages) < tag) &&
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
 *	prot:
 *		A value containing the memory protections associated with the memory
 *		location from where these instructions were copied.
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
				   AXP_MEMORY_PROTECTION prot)
{
	AXP_ICACHE_TAG_IDX addr;
	u32 ii, jj;
	u32 index, tag, setStart, setEnd;
	bool inserted = false;

	/*
	 * First, get the information from the supplied parameters we need to
	 * search the Icache correctly.
	 */
	addr.pc = pc;
	index = addr.insAddr.index;
	tag = addr.insAddr.tag;
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
	 * There are only 2 ways out of this function, successfully added the
	 * Icache line/block or aborted the program.  Loop until we have
	 * successfully added the line/block (aborting will end the entire
	 * program).
	 */
	while (inserted == false)
	{

		/*
		 * Now, search through the Icache for the information we have been
		 * asked to locate (a free Icache location to receive another
		 * line/block.
		 */
		for (ii = setStart; ((ii < setEnd) && (inserted == false)); ii++)
		{

			/*
		 	 * See if we found a place to hold this line/block.
		 	 */
			if (cpu->iCache[index][ii].vb == 0)
			{
				inserted =  AXP_LRUAdd(cpu->iCacheLRU,
								   	   AXP_21264_ICACHE_SIZE,
							  	   	   &cpu->iCacheLRUIdx,
							  	   	   index,
							  	   	   ii);

				/*
			 	 * If we were able to find an Icache location to place then
			 	 * line/block, then we should have been able to add an LRU
			 	 * record.  If not, then print an error message and abort the
			 	 * program.
			 	 */
				if (inserted == false)
				{
					printf("%%DECEMU-F-BUGCHK, Bugcheck in the Icache LRU addition.\n");
					abort();
				}
				cpu->iCache[index][ii].kre = prot.kre;
				cpu->iCache[index][ii].ere = prot.ere;
				cpu->iCache[index][ii].sre = prot.sre;
				cpu->iCache[index][ii].ure = prot.ure;
				cpu->iCache[index][ii]._asm = 0;	// TODO
				cpu->iCache[index][ii].asn = 0;		// TODO
					cpu->iCache[index][ii].pal = pc.pal;
				cpu->iCache[index][ii].vb = 1;
				cpu->iCache[index][ii].tag = tag;
				for (jj = 0; jj < AXP_ICACHE_LINE_INS; jj++)
					cpu->iCache[index][ii].instructions[jj] = nextInst[jj];
			}
		}

		/*
	 	 * If we did not insert a record into the Icache, then the Icache is
	 	 * full and someone needs to be evicted before we can reinsert.  Since
	 	 * all the sets at the current index are valid, we need to determine
	 	 * which one is the LRU item, remove it and try adding again.
	 	 */
		if (inserted == false)
		{
			u32 set;
			bool removed;

			removed = AXP_LRUReturnIdx(cpu->iCacheLRU,
								   	   cpu->iCacheLRUIdx,
								   	   index,
								   	   &set);
			if (removed == false)
			{
				printf("%%DECEMU-F-BUGCHK, Bugcheck in the Icache LRU retrieval.\n");
				abort();
			}
			removed = AXP_LRURemove(cpu->iCacheLRU,
									&cpu->iCacheLRUIdx,
									index,
									set);

			/*
		 	 * We successfully removed a record from the LRU list, we now need
		 	 * to invalidate (evict) the associated Icache record. Otherwise,
		 	 * we have a should never happen condition.  Display a message and
		 	 * a abort the program.
		 	 */
			if (removed == true)
				cpu->iCache[index][set].vb = 0;
			else
			{
				printf("%%DECEMU-F-BUGCHK, Bugcheck in the Icache LRU removal.\n");
				abort();
			}
		}
	}

	/*
	 * Return what we did or did not find back to the caller.
	 */
	return;
}

/*
 * AXP_LRUAdd
 * 	This  function should only be called as the result of an ITB Miss.  As such
 * 	we just have to find the next location to enter the next item.  If we are
 * 	using a currently used field, we'll need to evict all the associated Icache
 * 	items with the same tag.
 */
void AXP_ITBAdd(AXP_21264_CPU *cpu,
				AXP_IBOX_ITB_TAG itbTag,
				AXP_IBOX_ITB_PTE *itbPTE)
{
	int ii, jj;
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
					 * We need to invalidate this Icache entry, but don't
					 * forget to remove the LRU for this cache entry.
					 */
					(void) AXP_LRURemove(cpu->iCacheLRU,
										 &cpu->iCacheLRUIdx,
										 ii,
										 jj);

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
	cpu->itb[cpu->itbEnd].mapped = 1;	// TODO: How do we handle more than 1 page?
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
