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
 *	This module file contains the code to implemented the management of ITB,
 *	DTB, Icache and Dcache components within the Digital Alpha AXP 21264
 *	Processor.
 *
 * Revision History:
 *
 *	V01.000		04-Aug-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		24-Aug-2017	Jonathan D. Belanger
 *	I'm chancing the way the Dcache Add and Update functionality works.  These
 *	functions are going to be combined into a single Dcache Write function.
 *	The Dcache Fetch function will be changed to Dcache Read.  The Icache
 *	functions will remain the same, as there is only one writter (the Cbox) and
 *	only one reader, the Ibox.  Where as, the Dcache as potentially multiple
 *	writers (the Cbox and the Mbox) and only one reader (the Mbox).
 *
 *	V01.002		11-Oct-2017	Jonathan D. Belanger
 *	Added code to keep the Dcache Duplicate Tag (DTAG) in synch with the
 *	Dcache entries.
 *
 *	V01.003		12-Oct-2017	Jonathan D. Belanger
 *	Added code to anti-alias the possibility that a particular virtual/physical
 *	address pair could be in up to four possible locations.  We use the obvious
 *	address and just flush the other ones, if in use.
 */
#include "AXP_21264_Cache.h"

/*
 * TODO:	When updating the DTB, we also need to inform the Cbox, as it has
 *			a duplicate copy of the DTB (or we update it ourselves on behalf of
 *			the Cbox).  The difference between the two is that the DTB is
 *			virtually indexed and physically tagged and the Cbox version is
 *			physically indexed and virtually tagged.
 */

/*
 * Union to hold 2 32-bit values as a 64-bit value for the purposes of saving
 * the index and set value for a Dcache item.
 */
typedef union
{
	u64		idxSet;
	struct
	{
		u32	index;
		u32	set;
	}		idxOrSet;
} AXP_DCACHE_IDXSET;

/****************************************************************************/
/*																			*/
/*	The following code handles both the ITB and DTB lists for the Digital	*/
/*	Alpha AXP processors.  This implementation is consistent with the 21264	*/
/*	generation of this processor.  It may also be valid for other			*/
/*	generations.															*/
/*																			*/
/****************************************************************************/

/*
 * AXP_findTLBEntry
 *	This function is called to locate a TLB entry in either the Data or
 *	Instruction TLB, based off of the virtual address.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the ITB and DTB are located.
 *	virtAddr:
 *		The virtual address associated with the TLB entry for which we are
 *		looking.
 *	dtb:
 *		A boolean indicating whether we are looking in the DTB.  If not, then
 *		we are looking in the ITB.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	NULL:		The entry was not found.
 *	Not NULL:	A pointer to the TLB requested.
 */
AXP_21264_TLB *AXP_findTLBEntry(AXP_21264_CPU *cpu, u64 virtAddr, bool dtb)
{
	AXP_21264_TLB	*retVal = NULL;
	AXP_21264_TLB	*tlbArray = (dtb ? cpu->dtb : cpu->itb);
	u8				asn = (dtb ? cpu->dtbAsn0.asn : cpu->pCtx.asn);
	int				ii;

	/*
	 * Search through all valid TLB entries until we find the one we are being
	 * asked to return.
	 */
	for (ii = 0; ii < AXP_TB_LEN; ii++)
	{
		if (tlbArray[ii].valid == true)
		{
			if ((tlbArray[ii].virtAddr ==
				(virtAddr & tlbArray[ii].matchMask)) &&
			   (tlbArray[ii].asn == asn))
			{
				retVal = &tlbArray[ii];
				break;
			}
		}
	}

	/*
	 * Return what we found, or did not find, back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_getNextFreeTLB
 *	This function is called to find the first TLB entry that is not being used
 *	(invalid).  Unlike the 21264 ARM indicates, we do not do this as a
 *	round-robin list.  We just find the first available, starting from the
 *	first entry.
 *
 *	NOTE:	This function will find the next available entry (valid == false)
 *			or the next entry (valid == true) in the TLB array.  The way we
 *			perform our search, in all likelihood where valid == true, this is
 *			the oldest TLB entry (or close enough).
 *
 * Input Parameters:
 *	tlbArray:
 *		A pointer to the array to be searched.
 *	nextTLB:
 *		A pointer to a 32-bit value that is the next TLB entry to be selected.
 *
 * Output Parameters:
 *	nextTLB:
 *		A pointer to a 32-bit value that points to the next TLB entry to select
 *		on the next call.
 *
 * Return Value:
 *	NULL:		An available entry was not found.
 *	Not NULL:	A pointer to an unused TLB entry.
 */
AXP_21264_TLB *AXP_getNextFreeTLB(AXP_21264_TLB *tlbArray, u32 *nextTLB)
{
	AXP_21264_TLB 	*retVal = NULL;
	int				ii;
	int				start1, start2;
	int				end1, end2;

	/*
	 * The nextTLB index always points to the TLB entry to be selected (even if
	 * it is already in-use (valid).
	 */
	retVal = &tlbArray[*nextTLB];
	*nextTLB = (*nextTLB >= (AXP_TB_LEN - 1) ? 0 : *nextTLB + 1);

	/*
	 * If the next TLB items is marked in-use (valid), then see if there is one
	 * somewhere in the array that is not in-use, and let's select that one.
	 */
	if (tlbArray[*nextTLB].valid == true)
	{

		/*
		 * We start looking at the entry the TLB index was just moved to.
		 */
		start1 = *nextTLB;

		/*
		 * If we are starting at the first entry in the index, then we scan the
		 * entire array starting at 0 and stopping at the end of the array.
		 * Otherwise, we start from the current location to the end of the
		 * array, and then go back to the start of the array and search until
		 * the current location.
		 */
		if (start1 > 0)
		{
			start2 = 0;
			end1 = AXP_TB_LEN;
			end2 = start1;
		}
		else
			start2 = -1;	/* Searching 0-end, no need to do second search. */

		/*
		 * Perform the first, and possibly the only, search for a not-in-use
		 * entry.
		 */
		for (ii = start1; ii < end1; ii ++)
			if (tlbArray[ii].valid == false)
			{
				*nextTLB = ii;
				start2 = -1;	/* Don't do second search (either way). */
				break;
			}

		/*
		 * The second start entry is -1, either when we are searching from the
		 * start of the array, or when we found a not-in-use entry.
		 */
		if (start2 == 0)
			for (ii = start2; ii < end2; ii ++)
				if (tlbArray[ii].valid == false)
				{
					*nextTLB = ii;
					break;
				}
	}

	/*
	 * Return what we found, or did not find, back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_addTLBEntry
 *	This function is called to add a TLB entry into either the Data or
 *	Instruction TLB lists.  An available TLB entry will be used, if one is not
 *	already in the TLB list.  If the latter, then the entry will be updated.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the ITB and DTB are located.
 *	virtAddr:
 *		The virtual address to be associated with the TLB entry.
 *	dtb:
 *		A boolean indicating whether we are working in the DTB.  If not, then
 *		we are working in the ITB.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_addTLBEntry(AXP_21264_CPU *cpu, u64 virtAddr, u64 physAddr, bool dtb)
{
	AXP_21264_TLB	*tlbEntry;

	/*
	 * See if there already is an entry in the TLB.
	 */
	tlbEntry = AXP_findTLBEntry(cpu, virtAddr, dtb);

	/*
	 * If not, go locate an available TLB entry.
	 */
	if (tlbEntry == NULL)
	{
		if (dtb)
			tlbEntry = AXP_getNextFreeTLB(cpu->dtb, &cpu->nextDTB);
		else
			tlbEntry = AXP_getNextFreeTLB(cpu->itb, &cpu->nextITB);
	}

	/*
	 * Update the common fields for the TLB entry (for data and instruction).
	 */
	tlbEntry->matchMask = GH_MATCH(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);
	tlbEntry->keepMask = GH_KEEP(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);
	tlbEntry->virtAddr = virtAddr & tlbEntry->matchMask;
	tlbEntry->physAddr = physAddr & GH_PHYS(dtb ? cpu->dtbPte0.gh : cpu->itbPte.gh);

	/*
	 * Now update the specific fields from the correct PTE.
	 */
	if (dtb)
	{

		/*
		 * We use the DTE_PTE0 and DTE_ASN0 IPRs to initialize the TLB entry.
		 */
		tlbEntry->faultOnRead = cpu->dtbPte0._for;
		tlbEntry->faultOnWrite = cpu->dtbPte0.fow;
		tlbEntry->faultOnExecute = 0;
		tlbEntry->kre = cpu->dtbPte0.kre;
		tlbEntry->ere = cpu->dtbPte0.ere;
		tlbEntry->sre = cpu->dtbPte0.sre;
		tlbEntry->ure = cpu->dtbPte0.ure;
		tlbEntry->kwe = cpu->dtbPte0.kwe;
		tlbEntry->ewe = cpu->dtbPte0.ewe;
		tlbEntry->swe = cpu->dtbPte0.swe;
		tlbEntry->uwe = cpu->dtbPte0.uwe;
		tlbEntry->_asm = cpu->dtbPte0._asm;
		tlbEntry->asn = cpu->dtbAsn0.asn;
	}
	else
	{

		/*
		 * We use the ITB_PTE and PCTX IPRs to initialize the TLB entry.
		 *
		 * The following fault-on-read/write/execute are hard coded to keep the
		 * rest of the code happy.
		 */
		tlbEntry->faultOnRead = 1;
		tlbEntry->faultOnWrite = 0;
		tlbEntry->faultOnExecute = 1;
		tlbEntry->kre = cpu->itbPte.kre;
		tlbEntry->ere = cpu->itbPte.ere;
		tlbEntry->sre = cpu->itbPte.sre;
		tlbEntry->ure = cpu->itbPte.ure;
		tlbEntry->kwe = 0;
		tlbEntry->ewe = 0;
		tlbEntry->swe = 0;
		tlbEntry->uwe = 0;
		tlbEntry->_asm = cpu->itbPte._asm;
		tlbEntry->asn = cpu->pCtx.asn;
	}
	tlbEntry->valid = true;				/* Mark the TLB entry as valid. */

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_tbia
 *	This function is called to Invalidate all TLB entries as the result of an
 *	instruction writing to the ITB_IA or DTB_IA IPR.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the ITB and DTB are located.
 *	dtb:
 *		A boolean indicating whether we are invalidating entries in the DTB.
 *		If not, then we are doing this in the ITB.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_tbia(AXP_21264_CPU *cpu, bool dtb)
{
	AXP_21264_TLB	*tlbArray = (dtb ? cpu->dtb : cpu->itb);
	int				ii;

	/*
	 * Go through the entire TLB array and invalidate everything (even those
	 * entries that are already invalidated).
	 */
	for (ii = 0; ii < AXP_TB_LEN; ii++)
		tlbArray[ii].valid = false;

	/*
	 * Reset the next TLB entry to select to the start of the list.
	 */
	if (dtb)
		cpu->nextDTB = 0;
	else
		cpu->nextITB = 0;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_tbiap
 * 	This function is called to invalidate all process-specific TLB entries (TLB
 *	entries that do not have the ASM bit set).
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the ITB and DTB are located.
 *	dtb:
 *		A boolean indicating whether we are invalidating the DTB.  If not, then
 *		we are invalidating in ITB.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_tbiap(AXP_21264_CPU *cpu, bool dtb)
{
	AXP_21264_TLB	*tlbArray = (dtb ? cpu->dtb : cpu->itb);
	int				ii;

	/*
	 * Loop through all the TLB entries and if the ASM bit is not set, then
	 * invalidate the entry.  Leaving the entries with the ASM bit set alone,
	 * valid or otherwise.
	 */
	for (ii = 0; ii < AXP_TB_LEN; ii++)
		if (tlbArray[ii]._asm == 0)
			tlbArray[ii].valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_tbis
 *	This function is called to invalidate a Single TLB Entry.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the ITB and DTB are located.
 *	virtAddr:
 *		The virtual address to be associated with the TLB entry to be
 *		invalidated.
 *	dtb:
 *		A boolean indicating whether we are looking in the DTB.  If not, then
 *		we are looking in the ITB.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_tbis(AXP_21264_CPU *cpu, u64 va, bool dtb)
{
	AXP_21264_TLB	*tlb = AXP_findTLBEntry(cpu, va, dtb);

	/*
	 * If we did not find the entry, then there is nothing to invalidate.
	 * We'll just quietly continue on.
	 */
	if (tlb != NULL)
		tlb->valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/****************************************************************************/
/*																			*/
/*	The following code handles the virtual address to physical address		*/
/*	translation and the memory access checking associated with that			*/
/*	translation.  It utilizes the ITB and DTB structures to perform this	*/
/*	this translation, as well as the setting of the PALmode bit in the PC,	*/
/*	and the super page settings within the Ibox Contol Register or the Mbox	*/
/*	Dcache Control Register.												*/
/*																			*/
/****************************************************************************/

/*
 * AXP_21264_check_memoryAccess
 *	This function is caller to determine if the process has the access needed
 *	to the memory location the are trying to use (read/write/modify/execute).
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the current process mode is
 *		located.
 *	tlb:
 *		A pointer to the Translation Look-aside Buffer that has the access
 *		information for each of the processing modes.
 *	acc:
 *		An enumerated value, indicating the type of access being requested.
 *		This parameter can have any of the following enumerated values:
 *			None	- No access
 *			Read	- Read Access
 *			Write	- Write Access
 *			Execute	- Read Access - For the 21264 CPU, there is no execute bit
 *									to check.  This is because it is assumed
 *									that all addresses in the Icache have
 *									execute access.
 *			Modify 	- Read & Write
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	false:	If the process does not have the requested access.
 *	true:	If the process does have the access requested.
 */
bool AXP_21264_checkMemoryAccess(
				AXP_21264_CPU *cpu,
				AXP_21264_TLB *tlb,
				AXP_21264_ACCESS acc)
{
	bool	retVal = false;

	/*
	 * If the valid bit is not set, then by default, the process does not have
	 * access.
	 */
	if (tlb->valid == true)
	{

		/*
		 * Determine access based on the current mode.  Then within each mode,
		 * check that the requested access is allowed.
		 */
		switch(cpu->ierCm.cm)
		{
			case AXP_CM_KERNEL:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->kre == 0) || (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->kwe == 0) || (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->kre == 0) || (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = (((tlb->kwe == 0) ||
								   (tlb->kre == 0)) ||
								  ((tlb->faultOnWrite == 1) ||
								   (tlb->faultOnRead == 1)));
						break;
				}
				break;

			case AXP_CM_EXEC:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ere == 0) || (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->ewe == 0) || (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->ere == 0) || (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = (((tlb->ewe == 0) ||
								   (tlb->ere == 0)) ||
								  ((tlb->faultOnWrite == 1) ||
								   (tlb->faultOnRead == 1)));
						break;
				}
				break;

			case AXP_CM_SUPER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->sre == 0) || (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->swe == 0) || (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->sre == 0) || (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = (((tlb->swe == 0) ||
								   (tlb->sre == 0)) ||
								  ((tlb->faultOnWrite == 1) ||
								   (tlb->faultOnRead == 1)));
						break;
				}
				break;

			case AXP_CM_USER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ure == 0) || (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->uwe == 0) || (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->ure == 0) || (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = (((tlb->uwe == 0) ||
								   (tlb->ure == 0)) ||
								  ((tlb->faultOnWrite == 1) ||
								   (tlb->faultOnRead == 1)));
						break;
				}
				break;
		}	/* switch(cpu->ierCm.cm) */
	}		/* if (tlb->valid == true) */

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_va2pa
 *	This function is called to convert a virtual address to a physical address.
 *	It does this in 3 stages.
 *		1)	If we are in PALmode, then the physical address is equal to the
 *			virtual address.
 *		2)	If this is a super page, the use the virtual to physical mapping
 *			defined for super pages.  If the virtual address does not have a
 *			specific value at a specific location within the virtual address,
 *			then normal virtual address translation is performed (Step 3).
 *		3)	A translation look-aside buffer (TLB) is located for the virtual
 *			address.  Information within the TLB is used to determined if the
 *			process has the needed access to the virtual address, and to also
 *			convert the virtual address to a physical address.
 *	If a TLB cannot be located, or access is not allowed, then a fault is
 *	returned back to the caller to be handled (it will cause a call to PALcode
 *	to handle the fault).
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the AXP 21264 CPU structure containing the current
 *		execution mode and the DTB (for Data) and ITB (for Instructions)
 *		arrays.
 *	va:
 *		The virtual address value to be converted to a physical address.
 *	pc:
 *		The current program counter.  This is used to determine if we are in
 *		PALmode or not.
 *	dtb:
 *		A boolean value indicating if we are using the DTB.  If not, then we
 *		are using the ITB.
 *	acc:
 *		An enumerated value, indicating the type of access being requested.
 *		This parameter can have any of the following enumerated values:
 *			None	- No access
 *			Read	- Read Access
 *			Write	- Write Access
 *			Execute	- Read Access - For the 21264 CPU, there is no execute bit
 *									to check.  This is because it is assumed
 *									that all addresses in the Icache have
 *									execute access.
 *			Modify 	- Read & Write
 *
 * Output Parameters:
 *	_asm:
 *		A pointer to a boolean location to receive an indicator of whether the
 *		ASB bit is set for the TLB associated with the virtual address or not.
 *	fault:
 *		A pointer to an unsigned 32-bit integer location to receive an
 *		indicator of whether a fault should occur.  This will cause the CPU to
 *		call the appropriate PALcode to handle the fault.
 *
 * Return Value:
 *	0:	The virtual address could not be converted (yet, usually because of
 *		a fault).
 *	~0:	The physical address associated with the virtual address.
 */
u64 AXP_va2pa(
		AXP_21264_CPU *cpu,
		u64 va,
		AXP_PC pc,
		bool dtb,
		AXP_21264_ACCESS acc,
		bool *_asm,
		u32 *fault)
{
	AXP_VA_SPE		vaSpe = {.va = va};
	AXP_21264_TLB	*tlb;
	u64				pa = 0x0ll;
	u8				spe = (dtb ? cpu->mCtl.spe : cpu->iCtl.spe);

	/*
	 * Initialize the output parameters.
	 */
	*_asm = false;
	*fault = 0;

	/*
	 * If we are in PALmode, then the virtual address and physical address are
	 * the same.
	 */
	if (pc.pal == AXP_PAL_MODE)
		pa = va;

	/*
	 * If we are using a super page and are in Kernel mode, then we need to go
	 * down that translation path.
	 */
	else if ((spe != 0) && (cpu->ierCm.cm == AXP_CM_KERNEL))
	{
		if ((spe & AXP_SPE2_BIT) && (vaSpe.spe2.spe2 == AXP_SPE2_VA_VAL))
		{
			pa = va & AXP_SPE2_VA_MASK;
			*_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE1_BIT) && (vaSpe.spe1.spe1 == AXP_SPE1_VA_VAL))
		{
			pa = ((va & AXP_SPE1_VA_MASK) |
				  (va & AXP_SPE1_VA_40 ? AXP_SPE1_PA_43_41 : 0));
			*_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE0_BIT) && (vaSpe.spe0.spe0 == AXP_SPE0_VA_VAL))
		{
			pa = va & AXP_SPE0_VA_MASK;
			*_asm = false;
			return(pa);
		}
	}

	/*
	 * We need to see if we can find a TLB entry for this virtual address.  We
	 * get here, either when we are not in PALmode, not using a Super page, or
	 * the virtual address did not contain the expected Super page values.
	 */
	tlb = AXP_findTLBEntry(cpu, va, dtb);

	/*
	 * We were unable to find a TLB entry for this virtual address.  We need to
	 * call the PALcode to fill in for the TLB Miss.
	 */
	if (tlb == NULL)
	{

		/*
		 * TODO: The caller needs to set the following.
		 *	cpu->excAddr = pc;
		 *	if (cpu->tbMissOutstanding == false)
		 *		cpu->mmStat.? = 1;
		 *	cpu->va = va;
		 *	cpu->excSum.? = 1;
		 */
		if (cpu->tbMissOutstanding == true)
		{
			if (cpu->iCtl.va_48 == 0)
				*fault = AXP_DTBM_DOUBLE_3;
			else
				*fault = AXP_DTBM_DOUBLE_4;
		}
		else if (dtb == true)
		{
			*fault = AXP_DTBM_SINGLE;
			cpu->tbMissOutstanding = true;
		}
		else
		{
			*fault = AXP_ITB_MISS;
			cpu->tbMissOutstanding = true;
		}
	}

	/*
	 * We found a TLB entry, so we can now check the memory access and convert
	 * the virtual address into a physical address (finally).
	 */
	else
	{
		cpu->tbMissOutstanding = false;
		if (AXP_21264_checkMemoryAccess(cpu, tlb, acc) == false)
		{
			/* TODO: The caller need to do this: cpu->excAddr = pc; */
			if (dtb == true)
			{

				/*
				 * TODO: The caller needs to set the following.
				 *	cpu->excSum.? = 1;
				 *	cpu->mmStat.? = 1;
				 *	cpu->va = va;
				 */
				*fault = AXP_DFAULT;
			}
			else
			{

				/*
				 * TODO: The caller needs to set the following.
				 *	cpu->excSum = 0;
				 */
				*fault = AXP_IACV;
			}
		}
		else
		{
			pa = tlb->physAddr | (va & tlb->keepMask);
			*_asm = tlb->_asm;
		}
	}

	/*
	 * Return the physical address (zero if we could not determine it) back to
	 * the caller.  If 0x0ll is returned, then the fault output parameter
	 * should have the fault reason for not returning a valid physical address.
	 */
	return(pa);
}

/****************************************************************************/
/*																			*/
/*	The following code is utilized to manage the Dcache within the Digital	*/
/*	Alpha AXP processor.  It is consistent with the 21264 generation of		*/
/*	this CPU.  It may also be applicable for other generations.				*/
/*																			*/
/****************************************************************************/

/*
 * AXP_DcacheWrite
 * 	This function is called to add/update a cache entry into the Data Cache.
 * 	If the entry is already there, or in one of the four possible other
 * 	locations, then there is nothing to do.
 *
 *	This function is called as a result of a request to the Cbox to perform a
 *	Dcache fill.
 *
 *	Refer to Section 2.8.3 in the HRM for more information about processing
 *	Memory Address Space Store Instructions from the Store Queue (SQ).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
 * 	va:
 * 		The virtual address of the data in virtual memory.
 * 	pa:
 * 		The physical address, as stored in the DTB (Data TLB).
 *	len:
 *		A value indicating the length of the input parameter 'data'.  This
 *		parameter must be one of the following values (assumed signed):
 *			 1	= byte
 *			 2	= word
 *			 4	= longword
 *			 8	= quadword
 *			64	= 64 bytes of data being copied from memory to the Dcache
 * 	data:
 * 		A pointer to the data to be stored in the Dcache, whose length is is
 *		specified by the 'len' parameter.
 *	idxSet:
 *		An optional pointer to a 64-bit value to indicating the actual index
 *		and set from where the data is written to in the Dcache.  If this is
 *		NULL, then the virtual address (va) parameter will be used to determine
 *		index and set, which we may have to search for.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	false:	Entry not found.
 * 	true:	Found the entry requested.
 */
bool AXP_DcacheWrite(
			AXP_21264_CPU *cpu,
			u64 va,
			u64 pa,
			u32 len,
			void *data,
			u64 *idxSet)
{
	bool		retVal = false;
	AXP_VA		virtAddr = {.va = va};
	AXP_VA		physAddr = {.va = pa};
	AXP_VA		workingVa;
	AXP_VA		workingPa;
	u64			oneChecked = va;
	u8			*src8 = (u8 *) data;
	u16			*src16 = (u16 *) data;
	u32			*src32 = (u32 *) data;
	u64			*src64 = (u64 *) data;
	i32			lenOver = ((va % AXP_DCACHE_DATA_LEN) + len - 1) -
						  (AXP_DCACHE_DATA_LEN - 1);
	u32 		setToUse;
	u32			found = AXP_CACHE_ENTRIES;
	u32			ii, jj, kk;
	u32			sets;
	u32			ctagIndex;

	/*
	 * Does this write cross the 64 byte boundary.  If so, we have to do this
	 * in 2 steps.
	 *
	 *		1) Write to the Dcache for the 'other' index.
	 *		2) Write to the Dcache for the 'current' index.
	 *
	 * If the data does not cross the 64 byte boundary, then just proceed
	 * normally.
	 *
	 * The steps needed to do this are as follows:
	 *
	 *		1) First determine if the number of bytes to be written cross that
	 *		   64 byte boundary.
	 *		2) If step 1 is true, then perform the following sub-steps:
	 *			a) Determine how many bytes are on this page.
	 *			b) Call this function with the subset of data and the length of
	 *			   this data to be written to the 'other'.
	 *			c) Subtract the length in step b from the length supplied on
	 *			   this call
	 *		3) Write the length of the data to be written to the 'current'
	 *		   index
	 *
	 * NOTE: We may have to consider the effect of big-endian vs little-endian.
	 * The code below assumes little-endian.
	 *
	 * NOTE: The length of 64 bytes is used to fill the cache.  The address is
	 * the base of this 64-byte data.  Therefore, this length and the data to
	 * be written will never cross from one index to the next.
	 */
	if ((len < 64) && (lenOver > 0))
	{
		retVal = AXP_DcacheWrite(
			cpu,
			va + AXP_DCACHE_DATA_LEN,
			pa + AXP_DCACHE_DATA_LEN,
			(u32) lenOver,
			(void *) (data + len - lenOver),
			NULL);
		len -= lenOver;
	}

	/*
	 * 5.3.10 Determine how many sets are enabled.
	 *
	 * The set_en field in the Dcache Control Register (DC_DTL) is used to
	 * determine the number of cache sets to use.  The description for
	 * SET_EN[1:0] says that at least 1 set must be enabled.  The equivalent
	 * field for the Icache, IC_EN[1:0], has the following more detailed
	 * description:
	 *
	 *		At least one set must be enabled. The entire cache may be
	 *		enabled by setting both bits. Zero, one, or two Icache sets
	 *		can be enabled.
	 *		This bit does not clear the Dcache, but only disables fills to
	 *		the affected set.
	 *
	 * I'm not sure why it first says that at least one set must be enabled,
	 * but the says that "Zero, one, or two Icache sets can be enabled."  I'm
	 * going to assume that if zero bits are set, the default is 2 (both sets).
	 */
	sets = (cpu->dcCtl.set_en == 1) ? 1 : 2;

	if (idxSet != NULL)
	{
		AXP_DCACHE_IDXSET	indexAndSet;

		indexAndSet.idxSet = *idxSet;
		found = indexAndSet.idxOrSet.index;
		setToUse = indexAndSet.idxOrSet.set;
	}

	/*
	 * Check the index based solely on the Virtual Address (both sets).
	 */
	for (ii = 0; ((ii < sets) && (found == AXP_CACHE_ENTRIES)); ii++)
	{
		if ((cpu->dCache[virtAddr.vaIdx.index][ii].valid == true) &&
			(cpu->dCache[virtAddr.vaIdx.index][ii].physTag == physAddr.vaIdxInfo.tag))
		{
			found = virtAddr.vaIdx.index;
			break;
		}
	}

	/*
	 * Because the index for the Dcache and DTAG includes 2-bits of the virtual
	 * Page Number and the index for the CTAG includes 2 translated bits from the
	 * Physical Page Number, it is possible for these 2 bits to clober either an
	 * entry in the Dcache/DTAG or the CTAG.  Therefore, we need to make sure that
	 * the combination of Dcache/DTAG index/physical tag and CTAG index/virtual
	 * tag will not cause an issue.  How we do this is is as follows:
	 *
	 *		for each of the possible virtual addresses
	 *		begin
	 *			for each of the possible translated (physical) addresses
	 *			begin
	 *				if (the physTag in the dtag entry using the new index =
	 *					new tag from loop physical address)
	 *				then
	 *					flush this entry
	 *				endif
	 *			end
	 *		end
	 *			where address = xxxxxxxxxxxxyyzz
	 *					   zz = 0-63	(2 bits if high order z)
	 *					  yyz = 0-512	(2 bits of z; 3 bits of low order y)
	 *			xxxxxxxxxxxxy =  tag	(1 bit of y)
	 */
	for (ii = (va & AXP_MASK_2_HIGH_INDEX_BITS);
			   ii < (va + AXP_2_HIGH_INDEX_BITS_MAX);
			   ii += AXP_2_HIGH_INDEX_BITS_INCR)
	{
		if (ii != oneChecked)
		{
			workingVa.va = ii;
			for (jj = (pa & AXP_MASK_2_HIGH_INDEX_BITS);
			     jj < (pa + AXP_2_HIGH_INDEX_BITS_MAX);
				 jj += AXP_2_HIGH_INDEX_BITS_INCR)
			{
				workingPa.va = jj;
				for (kk = 0; kk < sets; kk++)
				{
					if ((cpu->dtag[workingVa.vaIdx.index][kk].physTag ==
						 workingPa.vaIdxInfo.tag) &&
						(cpu->dtag[workingVa.vaIdx.index][kk].valid == true))
					{
						ctagIndex = cpu->dtag[workingVa.vaIdx.index][kk].ctagIndex;
						if ((cpu->dCache[workingVa.vaIdx.index][kk].modified ==
						     true) ||
							(cpu->dCache[workingVa.vaIdx.index][kk].dirty ==
							 true))
						{
							/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
							cpu->dCache[workingVa.vaIdx.index][kk].modified =
								false;
						}

						/*
						 * Now invalidate the Dcache, DTAG, and CTAG entries.
						 */
						cpu->dCache[workingVa.vaIdx.index][kk].valid = false;
						cpu->dtag[workingVa.vaIdx.index][kk].valid = false;
						cpu->ctag[ctagIndex][kk].valid = false;
					}
				}
			}
		}
	}

	/*
	 * If we did not find it, then use the first value and determine which set
	 * to use and store the information into the Dcache.
	 */
	if (found == AXP_CACHE_ENTRIES)
	{
		virtAddr.va = va;
		found = virtAddr.vaIdx.index;
		if ((cpu->dCache[found][0].valid == false) || (sets == 1))
		{
			setToUse = 0;
			cpu->dCache[found][0].set_0_1 = (sets == 1 ? false : true);
		}
		else if (cpu->dCache[found][1].valid == false)
		{
			setToUse = 1;
			cpu->dCache[found][0].set_0_1 = false;
		}
		else	/* We have to re-use one of the existing cache entries. */
		{
			if (cpu->dCache[found][0].set_0_1)
			{
				setToUse = 1;
				cpu->dCache[found][0].set_0_1 = false;
			}
			else
			{
				setToUse = 0;
				cpu->dCache[found][0].set_0_1 = true;
			}
		}
	}

	/*
	 * OK, we now have the index and the set, check to see if we are
	 * evicting an existing cache entry that also needs to be pushed out to
	 * memory, store the data and set the bits.
	 */
	if ((cpu->dCache[found][setToUse].valid == true) &&
		((cpu->dCache[found][setToUse].dirty == true) ||
		 (cpu->dCache[found][setToUse].modified == true)))
	{
		/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
		cpu->dCache[found][setToUse].modified = false;

		/*
		 * Now invalidate the Dcache, DTAG, and CTAG entries.
		 */
		cpu->dCache[found][setToUse].valid = false;
		cpu->dtag[found][setToUse].valid = false;
		cpu->ctag[found][setToUse].valid = false;
	}

	/*
	 * Write the supplied data to the Dcache (based on length).
	 */
	switch (len)
	{
		case 1:
			*((u8 *) cpu->dCache[found][setToUse].data) = *src8;
			break;

		case 2:
			*((u16 *) cpu->dCache[found][setToUse].data) = *src16;
			break;

		case 4:
			*((u32 *) cpu->dCache[found][setToUse].data) = *src32;
			break;

		case 8:
			*((u64 *) cpu->dCache[found][setToUse].data) = *src64;
			break;

		default:	/* for 64 bytes and any of the possible sub-lengths */
			memcpy(cpu->dCache[found][setToUse].data, src8, len);
			break;
	}
#pragma message "Need to determine when to use dirty/modified cache."

	cpu->dCache[found][setToUse].physTag = physAddr.vaIdxInfo.tag;
	cpu->dCache[found][setToUse].dirty = false;
	cpu->dCache[found][setToUse].modified = false;
	cpu->dCache[found][setToUse].shared = false;
	cpu->dCache[found][setToUse].valid = true;

	/*
	 * Duplicate the tag information in the DTAG array.
	 */
	ctagIndex = physAddr.vaIdxInfo.index;
	cpu->dtag[found][setToUse].physTag = physAddr.vaIdxInfo.tag;
	cpu->dtag[found][setToUse].ctagIndex = ctagIndex;
	cpu->dtag[found][setToUse].valid = false;

	/*
	 * At this point we do not have to worry about aliases clashing.  They
	 * have been taken are of above.
	 */
	cpu->ctag[ctagIndex][setToUse].virtTag = virtAddr.vaIdxInfo.tag;
	cpu->ctag[ctagIndex][setToUse].dtagIndex = found;
	cpu->ctag[ctagIndex][setToUse].valid = true;

	/*
	 * Return back to the caller.
	 */
	retVal = true;
	return(retVal);
}

/*
 * AXP_DcacheFlush
 * 	This function is called to flush the entire Data Cache.
 *
 *	NOTE:	There currently is no write to a register to flush the cache like
 *			there is for the Icache.  This code is here as an example, if there
 *			ever is a need to flush the Dcache.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_DcacheFlush(AXP_21264_CPU *cpu)
{
	int			ii, jj;

	/*
	 * Go through each cache item and invalidate and reset it.
	 */
	for (ii = 0; ii < AXP_CACHE_ENTRIES; ii++)
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			if (cpu->dCache[ii][jj].dirty || cpu->dCache[ii][jj].modified)
			{
				/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
				cpu->dCache[ii][jj].modified = false;
			}
			cpu->dCache[ii][jj].set_0_1 = false;
			cpu->dCache[ii][jj].physTag = 0;
			cpu->dCache[ii][jj].dirty = false;
			cpu->dCache[ii][jj].modified = false;
			cpu->dCache[ii][jj].shared = false;
			cpu->dCache[ii][jj].valid = false;
			cpu->dtag[ii][jj].valid = false;		/* Don't forget the DTAG */
		}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_DcacheRead
 * 	This function is called to read a cache entry from the Data Cache.  This
 *	function can read any of the natural values from the Dcache.  The natural
 *	values are, in order of size, byte (8-bits), word (16-bits), longword
 *	(32-bits), and quadword (64-bits).
 *
 *	Refer to Section 2.8.1 in the HRM for more information about processing
 *	Memory Address Space Load Instructions from the Load Queue (LQ).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
 * 	va:
 * 		The virtual address of the data in virtual memory.
 * 	pa:
 * 		The physical address, as stored in the DTB (Data TLB).
 *	len:
 *		A value indicating the length of the output parameter 'data'.  This
 *		parameter must be one of the following values (assumed signed):
 *			1	= byte
 *			2	= word
 *			4	= longword
 *			8	= quadword
 *
 * Output Parameters:
 * 	data:
 *		A pointer a location to receive the data from the data cache.  The size
 *		of this location is determined in the input parameter 'len'.
 *	idxSet:
 *		A pointer to a 64-bit value to receive the actual index and set from
 *		where the data was returned.
 *
 * Return Value:
 * 	false:	Entry not found.
 * 	true:	Found the entry requested.
 */
bool AXP_DcacheRead(
			AXP_21264_CPU *cpu,
			u64 va,
			u64 pa,
			u32 len,
			void *data,
			u64 *idxSet)
{
	bool				retVal = false;
	AXP_DCACHE_IDXSET	indexAndSet;
	AXP_VA				virtAddr = {.va = va};
	AXP_VA				physAddr = {.va = pa};
	u64					*dest64 = (u64 *) data;
	u32					*dest32 = (u32 *) data;
	u16					*dest16 = (u16 *) data;
	u8					*dest8 = (u8 *) data;
	i32					lenOver = ((va % AXP_DCACHE_DATA_LEN) + len - 1) -
						  	  	   (AXP_DCACHE_DATA_LEN - 1);
	u32					ii;
	u32					sets;

	/*
	 * Does this read cross the 64 byte boundary.  If so, we have to do this
	 * in 3 steps.
	 *
	 *		1) Read from the Dcache for the 'other' index.
	 *		2) Read from the Dcache for the 'current' index.
	 *		3) Merge the 2 reads
	 *
	 * If the data does not cross the 64 byte boundary, then just proceed
	 * normally.
	 *
	 * The steps needed to do this are as follows:
	 *
	 *		1) First determine if the number of bytes to be read cross that
	 *		   64 byte boundary.
	 *		2) If step 1 is true, then perform the following sub-steps:
	 *			a) Determine how many bytes are on this page.
	 *			b) Call this function with the subset of data and the length of
	 *			   this data to be read to the 'other'.
	 *			c) Subtract the length in step b from the length supplied on
	 *			   this call
	 *		3) Read the length of the data to be read to the 'current'
	 *		   index
	 *		4) Merge Anything from step 2 into what was read from step 3.
	 *
	 * NOTE: We may have to consider the effect of big-endian vs little-endian.
	 * The code below assumes little-endian.
	 */
	if ((len < 64) && (lenOver > 0))
	{
		retVal = AXP_DcacheRead(
			cpu,
			va + AXP_DCACHE_DATA_LEN,
			pa + AXP_DCACHE_DATA_LEN,
			(u32) lenOver,
			(void *) (data + len - lenOver),
			NULL);
		len -= lenOver;
	}

	/*
	 * 5.3.10 Determine how many sets are enabled.
	 *
	 * The set_en field in the Dcache Control Register (DC_DTL) is used to
	 * determine the number of cache sets to use.  The description for
	 * SET_EN[1:0] says that at least 1 set must be enabled.  The equivalent
	 * field for the Icache, IC_EN[1:0], has the following more detailed
	 * description:
	 *
	 *		At least one set must be enabled. The entire cache may be
	 *		enabled by setting both bits. Zero, one, or two Icache sets
	 *		can be enabled.
	 *		This bit does not clear the Icache, but only disables fills to
	 *		the affected set.
	 *
	 * I'm not sure why it first says that at least one set must be enabled,
	 * but the says that "Zero, one, or two Icache sets can be enabled."  I'm
	 * going to assume that if zero bits are set, the default is 2 (both sets).
	 */
	sets = (cpu->dcCtl.set_en == 1) ? 1 : 2;

	/*
	 * Check the index based solely on the Virtual Address (both sets).
	 */
	for (ii = 0; ii < sets; ii++)
	{
		if ((cpu->dCache[virtAddr.vaIdx.index][ii].valid == true) &&
			(cpu->dCache[virtAddr.vaIdx.index][ii].physTag ==
			 physAddr.vaIdxInfo.tag))
		{

			/*
			 * Combine the index and set associated with this data and return
			 * that into the caller supplied parameter.
			 */
			indexAndSet.idxOrSet.index = virtAddr.vaIdx.index;
			indexAndSet.idxOrSet.set = ii;
			*idxSet = indexAndSet.idxSet;

			/*
			 * Indicate that we are returning the data requested.
			 */
			retVal = true;
			break;
		}
	}

	/*
	 * If we found what we were looking for, so save the value requested
	 * into the location provided, based on size of the data.
	 */
	if (retVal == true)
	{
		u32		index = indexAndSet.idxOrSet.index;
		u32		set = indexAndSet.idxOrSet.set;

		switch (len)
		{
			case 1:
				*dest8 = *((u8 *) cpu->dCache[index][set].data);
				break;

			case 2:
				*dest16 = *((u16 *) cpu->dCache[index][set].data);
				break;

			case 4:
				*dest32 = *((u32 *) cpu->dCache[index][set].data);
				break;

			case 8:
				*dest64 = *((u64 *) cpu->dCache[index][set].data);
				break;

			default:
				memcpy(dest8, cpu->dCache[index][set].data, len);
				break;
		}
	}

	/*
	 * Return what we found, if anything, back to the caller.
	 *
	 * NOTE:	The caller will have to handle the condition where the cache
	 *			block they expected to be there, was not, and submit a request
	 *			to the Cbox to file the Dcache block.
	 */
	return(retVal);
}

/****************************************************************************/
/*																			*/
/*	The following code is utilized for managing the Icache for the Digital	*/
/*	Alpha AXP processor.  It is consistent with the 21264 generation of 	*/
/*	this processor.  It may also be applicable to other generations.		*/
/*																			*/
/****************************************************************************/

/*
 * AXP_IcacheAdd
 *	This function is called to add a set of instructions (16 32-bit
 *	instructions (64B)) to the Instruction Cache (Icache).
 *
 *	This function is called as a result of a request to the Cbox to perform an
 *	Icache fill.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Instruction Cache (iCache) array.
 *	pc:
 *		The Program Counter (PC) associated with the first instruction being
 *		loaded into the Icache.
 *	nextInst:
 *		A pointer to the next set of instructions (16 of them) to be loaded
 *		into the Icache.
 *	tlb:
 *		A pointer to the Instruction Translation Look-aside Buffer (ITB)
 *		associated with these instructions.
 *		NOTE:	We have this supplied by the caller, rather than looking it
 *				up in this function, because handing not finding the ITB
 *				should not be done here, but it better done in the caller even
 *				before calling this function.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_IcacheAdd(
				AXP_21264_CPU *cpu,
				AXP_PC pc,
				u32 *nextInst,
				AXP_21264_TLB *itb)
{
	AXP_VPC		vpc = {.pc = pc};
	u32			index = vpc.vpcFields.index;
	u64			tag = vpc.vpcFields.tag;
	u32			ii;
	u32			sets, whichSet;

	/*
	 * 5.3.10 Determine how many sets are enabled.
	 *
	 * The set_en field in the Dcache Control Register (DC_DTL) is used to
	 * determine the number of cache sets to use.  The description for
	 * SET_EN[1:0] says that at least 1 set must be enabled.  The equivalent
	 * field for the Icache, IC_EN[1:0], has the following more detailed
	 * description:
	 *
	 *		At least one set must be enabled. The entire cache may be
	 *		enabled by setting both bits. Zero, one, or two Icache sets
	 *		can be enabled.
	 *		This bit does not clear the Icache, but only disables fills to
	 *		the affected set.
	 *
	 * I'm not sure why it first says that at least one set must be enabled,
	 * but the says that "Zero, one, or two Icache sets can be enabled."  I'm
	 * going to assume that if zero bits are set, the default is 2 (both sets).
	 */
	sets = (cpu->iCtl.ic_en == 1) ? 1 : 2;
	if (sets == 2)
	{

		/*
		 * First see if set zero or set one are not currently in use.  If so,
		 * then use the first one found.  If both are currently in use, we need
		 * to evict one.  Use the set_0_1 bit in set zero to determine which
		 * one to select.  If set_0_1 equals 1, then use set zero.  Otherwise,
		 * use set one.  This is kind of a round-robin methodology.
		 */
		if (cpu->iCache[index][0].vb == 0)
		{
			whichSet = 0;
			cpu->iCache[index][0].set_0_1 = 1;
		}
		else if (cpu->iCache[index][1].vb == 0)
		{
			whichSet = 1;
			cpu->iCache[index][0].set_0_1 = 0;
		}
		else if (cpu->iCache[index][0].set_0_1 == 0)
		{
			whichSet = 0;
			cpu->iCache[index][0].set_0_1 = 1;
		}
		else
		{
			whichSet = 1;
			cpu->iCache[index][0].set_0_1 = 0;
		}
	}
	else
	{
		whichSet = 0;	/* just one set is in use. */
		cpu->iCache[index][0].set_0_1 = 0;
	}

	/*
	 * Initialize the cache entry with the supplied information.
	 */
	cpu->iCache[index][whichSet].kre = itb->kre;
	cpu->iCache[index][whichSet].ere = itb->ere;
	cpu->iCache[index][whichSet].sre = itb->sre;
	cpu->iCache[index][whichSet].ure = itb->ure;
	cpu->iCache[index][whichSet]._asm = itb->_asm;
	cpu->iCache[index][whichSet].asn = itb->asn;
	cpu->iCache[index][whichSet].pal = pc.pal;
	cpu->iCache[index][whichSet].vb = 1;
	cpu->iCache[index][whichSet].tag = tag;
	for (ii = 0; ii < AXP_ICACHE_LINE_INS; ii++)
		cpu->iCache[index][whichSet].instructions[ii].instr = nextInst[ii];

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_IcacheFlush
 * 	This function is called to flush the entire or specific Instruction Cache.
 *	Retiring a write to the IPRs IC_FLUSH or IC_FLUSH_ASM will call this
 *	function to purge the Instruction Cache.
 *
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Instruction Cache (iCache) array.
 *	purgeAsm:
 *		A boolean to indicate that only entries with the _asm bit not set will
 *		be purged.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_IcacheFlush(AXP_21264_CPU *cpu, bool purgeAsm)
{
	u32			ii, jj;

	for (ii = 0; ii < AXP_CACHE_ENTRIES; ii++)
	{

		/*
		 * Purge the cache entry based on the setting of the purgeAsm flag.
		 *
		 * Set Zero
		 */
		if (cpu->iCache[ii][0].vb == 1)
		{
			if (((purgeAsm == true) && (cpu->iCache[ii][0]._asm == 0)) ||
				(purgeAsm == false))
			{
				cpu->iCache[ii][0].kre = 0;
				cpu->iCache[ii][0].ere = 0;
				cpu->iCache[ii][0].sre = 0;
				cpu->iCache[ii][0].ure = 0;
				cpu->iCache[ii][0]._asm = 0;
				cpu->iCache[ii][0].asn = 0;
				cpu->iCache[ii][0].pal = 0;
				cpu->iCache[ii][0].vb = 0;
				cpu->iCache[ii][0].tag = 0;
				for (jj = 0; jj < AXP_ICACHE_LINE_INS; jj++)
					memset(
						&cpu->iCache[ii][0].instructions[jj],
						0,
						sizeof(AXP_INS_FMT));
			}
		}

		/*
		 * Set One
		 */
		if (cpu->iCache[ii][1].vb == 1)
		{
			if (((purgeAsm == true) && (cpu->iCache[ii][1]._asm == 0)) ||
				(purgeAsm == false))
			{
				cpu->iCache[ii][1].kre = 0;
				cpu->iCache[ii][1].ere = 0;
				cpu->iCache[ii][1].sre = 0;
				cpu->iCache[ii][1].ure = 0;
				cpu->iCache[ii][1]._asm = 0;
				cpu->iCache[ii][1].asn = 0;
				cpu->iCache[ii][1].pal = 0;
				cpu->iCache[ii][1].vb = 0;
				cpu->iCache[ii][1].tag = 0;
				for (jj = 0; jj < AXP_ICACHE_LINE_INS; jj++)
					memset(
						&cpu->iCache[ii][1].instructions[jj],
						0,
						sizeof(AXP_INS_FMT));
			}
		}
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_IcacheFetch
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
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Instruction Cache (iCache) array.
 *	pc:
 *		The Program Counter (PC) associated with the first instruction being
 *		loaded into the Icache.
 *
 * Output Parameters:
 *	next:
 *		A pointer to a location to receive the next 4 instructions to be
 *		executed.
 *
 * Return Value:
 *	True:	Instructions were returned.
 *	False:	Instructions were not found.
 */
bool AXP_IcacheFetch(AXP_21264_CPU *cpu, AXP_PC pc, AXP_INS_LINE *next)
{
	bool		retVal = false;
	AXP_VPC		vpc = {.pc = pc};
	AXP_PC		tmpPC;
	u32			index = vpc.vpcFields.index;
	u64			tag = vpc.vpcFields.tag;
	u32			offset = vpc.vpcFields.offset % AXP_ICACHE_LINE_INS;
	u32			ii;
	u32			sets, whichSet;

	/*
	 * 5.3.10 Determine how many sets are enabled.
	 *
	 * The set_en field in the Dcache Control Register (DC_DTL) is used to
	 * determine the number of cache sets to use.  The description for
	 * SET_EN[1:0] says that at least 1 set must be enabled.  The equivalent
	 * field for the Icache, IC_EN[1:0], has the following more detailed
	 * description:
	 *
	 *		At least one set must be enabled. The entire cache may be
	 *		enabled by setting both bits. Zero, one, or two Icache sets
	 *		can be enabled.
	 *		This bit does not clear the Icache, but only disables fills to
	 *		the affected set.
	 *
	 * I'm not sure why it first says that at least one set must be enabled,
	 * but the says that "Zero, one, or two Icache sets can be enabled."  I'm
	 * going to assume that if zero bits are set, the default is 2 (both sets).
	 */
	sets = (cpu->iCtl.ic_en == 1) ? 1 : 2;

	if ((cpu->iCache[index][next->setPrediction].vb == 1) &&
		(cpu->iCache[index][next->setPrediction].tag == tag))
	{
		whichSet = next->setPrediction;
		retVal = true;
	}
	else if ((cpu->iCache[index][(next->setPrediction + 1) & 1].vb == 1) &&
			 (cpu->iCache[index][(next->setPrediction + 1) & 1].tag == tag))
	{
		whichSet = (next->setPrediction + 1) & 1;
		retVal = true;
	}

	/*
	 * If we found what were were asked to find, then get the next set of
	 * instructions and return them to the caller
	 */
	if (retVal == true)
	{
		tmpPC = pc;
		for (ii = 0; ii < AXP_NUM_FETCH_INS; ii++)
		{
			next->instructions[ii] =
				cpu->iCache[index][whichSet].instructions[offset + ii];
			next->instrType[ii] =
				AXP_InstructionFormat(next->instructions[ii]);
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
 			next->setPrediction = whichSet;			/* same set */
 		}
 		else
 		{
 			if (sets == 1)
 			{
 				next->linePrediction = index + 1;	/* next line */
 				next->setPrediction = 0;			/* only set */
 			}
 			else if (whichSet == 0)
 			{
 				next->linePrediction = index + 1;	/* same line */
 				next->setPrediction = 1;			/* second set */
 			}
			else
 			{
 				next->linePrediction = index + 1;	/* next line */
 				next->setPrediction = 0;			/* first set */
 			}
 		}
	}

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_IcacheValid
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
 *	None.
 *
 * Return Value:
 *	True:	Instructions are in the Icache.
 *	False:	Instructions are not in the Icache.
 */
bool AXP_IcacheValid(AXP_21264_CPU *cpu, AXP_PC pc)
{
	bool		retVal = false;
	AXP_VPC		vpc = {.pc = pc};
	u32			index = vpc.vpcFields.index;
	u64			tag = vpc.vpcFields.tag;
	u32			ii;
	u32			sets;

	/*
	 * 5.3.10 Determine how many sets are enabled.
	 *
	 * The set_en field in the Dcache Control Register (DC_DTL) is used to
	 * determine the number of cache sets to use.  The description for
	 * SET_EN[1:0] says that at least 1 set must be enabled.  The equivalent
	 * field for the Icache, IC_EN[1:0], has the following more detailed
	 * description:
	 *
	 *		At least one set must be enabled. The entire cache may be
	 *		enabled by setting both bits. Zero, one, or two Icache sets
	 *		can be enabled.
	 *		This bit does not clear the Icache, but only disables fills to
	 *		the affected set.
	 *
	 * I'm not sure why it first says that at least one set must be enabled,
	 * but the says that "Zero, one, or two Icache sets can be enabled."  I'm
	 * going to assume that if zero bits are set, the default is 2 (both sets).
	 */
	sets = (cpu->iCtl.ic_en == 1) ? 1 : 2;

	for (ii = 1; ((ii < sets) & (retVal == false)); ii++)
	{
		if ((cpu->iCache[index][ii].vb == 1) &&
			(cpu->iCache[index][ii].tag == tag))
			retVal = true;
	}

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}
