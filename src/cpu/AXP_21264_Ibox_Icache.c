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
 *	Icache functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		22-Jun-2017	Jonathan D. Belanger
 *	Initially written, migrated from the AXP_21264_Ibox.c module.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Ibox_Icache.h"

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
 *	Hit, if the instruction is found in the instruction cache.
 *	WayMiss, if there was an ITB miss.
 *	Miss, if there was an ITB miss.
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
 * AXP_ICacheValid
 * 	This function is called to determine if a specific VPC is already in the
 * 	Icache.  It just returns the same Hit/Miss/WayMiss.
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
 * 	index:
 * 		A pointer to a location to receive the extracted index for the PC.
 * 	set:
 * 		A pointer to a location to receive the extracted set for the PC.
 *
 * Return Value:
 *	Hit, if the instruction is found in the instruction cache.
 *	WayMiss, if there was an ITB miss.
 *	Miss, if there was an ITB miss.
 */
AXP_CACHE_FETCH AXP_ICacheValid(AXP_21264_CPU *cpu,
								AXP_PC pc,
								u32 *index,
								u32 *set)
{
	AXP_CACHE_FETCH retVal = WayMiss;
	AXP_ICACHE_TAG_IDX addr;
	u64 tag;
	u32 ii;

	/*
	 * First, get the information from the supplied parameters we need to
	 * search the Icache correctly.
	 */
	addr.pc = pc;
	*index = addr.insAddr.index;
	*set = addr.insAddr.set;
	tag = addr.insAddr.tag;
	switch(cpu->iCtl.ic_en)
	{

		/*
		 * Just set 0
		 */
		case 1:
			*set = 0;
			break;

		/*
		 * Just set 1
		 */
		case 2:
			*set = 1;
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
	if ((cpu->iCache[*index][*set].tag == tag) &&
		(cpu->iCache[*index][*set].vb == 1))
	{
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
	cpu->iCache[index][set].asn = cpu->pCtx.asn;		// TODO: Need to verify
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

	/*
	 * Increment the ITB, but not past the end of the list.
	 */
	cpu->itbEnd = (cpu->itbEnd + 1) % AXP_TB_LEN;

	/*
	 * The itbEnd equals itbStart only in 2 instances.  One, when there is
	 * nothing in the itb array.  And two, when an entry was added onto an
	 * existing entry (which we just removed above).
	 */
	if (cpu->itbEnd == cpu->itbStart)
		cpu->itbStart = (cpu->itbStart + 1) % AXP_TB_LEN;

	/*
	 * Return back to the caller.
	 */
	return;
}
