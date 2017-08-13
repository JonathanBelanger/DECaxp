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
	 * We'll just quitely continue on.
	 */
	if (tlb != NULL)
		tlb->valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

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
						retVal = ((tlb->kre == 1) && (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->kwe == 1) && (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->kre == 1) && (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = ((tlb->kwe == 1) && (tlb->kre) &&
								  (tlb->faultOnWrite == 1) && (tlb->faultOnRead == 1));
						break;
				}
				break;

			case AXP_CM_EXEC:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ere == 1) && (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->ewe == 1) && (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->ere == 1) && (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = ((tlb->ewe == 1) && (tlb->ere) &&
								  (tlb->faultOnWrite == 1) && (tlb->faultOnRead == 1));
						break;
				}
				break;

			case AXP_CM_SUPER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->sre == 1) && (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->swe == 1) && (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->sre == 1) && (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = ((tlb->swe == 1) && (tlb->sre) &&
								  (tlb->faultOnWrite == 1) && (tlb->faultOnRead == 1));
						break;
				}
				break;

			case AXP_CM_USER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ure == 1) && (tlb->faultOnRead == 1));
						break;

					case Write:
						retVal = ((tlb->uwe == 1) && (tlb->faultOnWrite == 1));
						break;

					case Execute:
						retVal = ((tlb->ure == 1) && (tlb->faultOnExecute == 1));
						break;

					case Modify:
						retVal = ((tlb->uwe == 1) && (tlb->ure) &&
								  (tlb->faultOnWrite == 1) && (tlb->faultOnRead == 1));
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
 *	If a tlb cannot be located, or access is not allowed, then a fault is
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

/*
 * AXP_DcacheAdd
 * 	This function is called to add a cache entry into the Data Cache.  If the
 * 	entry is already there, or in one of the four possible other locations,
 * 	then there is nothing to do.
 *
 *	This function is called as a result of a request to the Cbox to perform a
 *	Dcache fill.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
 * 	va:
 * 		The virtual address of the data in virtual memory.
 * 	pa:
 * 		The physical address, as stored in the DTB (Data TLB).
 * 	data:
 * 		A pointer to the 64-bytes of data to be stored in the cache.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_DcacheAdd(AXP_21264_CPU *cpu, u64 va, u64 pa, u8 *data)
{
	AXP_VA		virtAddr = {.va = va};
	u32			oneChecked = virtAddr.vaIdxCntr.counter;
	int			found = AXP_CACHE_ENTRIES;
	int			ii;
	int			sets;

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
			(cpu->dCache[virtAddr.vaIdx.index][ii].physTag == pa))
		{
			found = virtAddr.vaIdx.index;
			break;
		}
	}

	/*
	 * If we did not find it, then check the other 3 possible places.
	 * TODO:	I'm not sure I'm selecting the cache line correct based on the
	 *			2 extra bits at the top of the index that are beyond the 8K
	 *			page.
	 */
	if (found == AXP_CACHE_ENTRIES)
	{
		for (virtAddr.vaIdxCntr.counter = 0;
			 ((virtAddr.vaIdxCntr.counter < 4) && (found == AXP_CACHE_ENTRIES));
			 virtAddr.vaIdxCntr.counter++)
		{
			if (virtAddr.vaIdxCntr.counter != oneChecked)
			{
				for (ii = 0; ((ii < sets) && (found == AXP_CACHE_ENTRIES)); ii++)
				{
					if ((cpu->dCache[virtAddr.vaIdx.index][ii].valid == true) &&
						(cpu->dCache[virtAddr.vaIdx.index][ii].physTag == pa))
						found = virtAddr.vaIdx.index;
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
		int setToUse;

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

		/*
		 * We are re-using a cache entry.  IF the modified bit it set,
		 * then write the existing value to memory.
		 */
		if (cpu->dCache[found][setToUse].modified)
		{
			/* Send to the Cbox to copy into memory. */
			cpu->dCache[found][setToUse].modified = false;
			/* TODO:	Need to determine what to do, if anything, with the */
			/*			dirty bit											*/
		}

		/*
		 * OK, we now have the index and the set, check to see if we are
		 * evicting an existing cache entry that also needs to be pushed out to
		 * memory, store the data and set the bits.
		 */
		if ((cpu->dCache[found][setToUse].valid == true) &&
			(cpu->dCache[found][setToUse].dirty ||
			 cpu->dCache[found][setToUse].modified))
		{
			/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
			cpu->dCache[ii][0].modified = false;
		}
		memcpy(cpu->dCache[found][setToUse].data, data, AXP_DCACHE_DATA_LEN);
		cpu->dCache[found][setToUse].physTag = pa;
		cpu->dCache[found][setToUse].dirty = false;
		cpu->dCache[found][setToUse].modified = false;
		cpu->dCache[found][setToUse].shared = false;
		cpu->dCache[found][setToUse].valid = true;
	}

	/*
	 * Return back to the caller.
	 */
	return;
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
	int			ii;

	/*
	 * Go through each cache item and invalidate and reset it.
	 */
	for (ii = 0; ii < AXP_CACHE_ENTRIES; ii++)
	{

		/*
		 * Set Zero.
		 */
		if (cpu->dCache[ii][0].dirty || cpu->dCache[ii][0].modified)
		{
			/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
			cpu->dCache[ii][0].modified = false;
		}
		cpu->dCache[ii][0].set_0_1 = false;
		cpu->dCache[ii][0].physTag = 0;
		cpu->dCache[ii][0].dirty = false;
		cpu->dCache[ii][0].modified = false;
		cpu->dCache[ii][0].shared = false;
		cpu->dCache[ii][0].valid = false;

		/*
		 * Set One.
		 *
		 * NOTE:	We are not going to avoid clearing the second cache set
		 *			based on the Dcache control field.  Not doing this
		 *			simplifies the code.
		 */
		if (cpu->dCache[ii][1].modified)
		{
			/* Send to the Cbox to copy into memory. */
#pragma message "Write-back Cache code needs to be completed."
			cpu->dCache[ii][1].modified = false;
			/* TODO:	Need to determine what to do, if anything, with the */
			/*			dirty bit											*/
		}
		cpu->dCache[ii][1].physTag = 0;
		cpu->dCache[ii][1].dirty = false;
		cpu->dCache[ii][1].modified = false;
		cpu->dCache[ii][1].shared = false;
		cpu->dCache[ii][1].valid = false;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_DcacheFetch
 * 	This function is called to fetch a cache entry from the Data Cache.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
 * 	va:
 * 		The virtual address of the data in virtual memory.
 * 	pa:
 * 		The physical address, as stored in the DTB (Data TLB).
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	NULL:	Entry not found.
 * 	~NULL:	The entry that was requested.
 */
u8 *AXP_DcacheFetch(AXP_21264_CPU *cpu, u64 va, u64 pa, u8 *data)
{
	u8			*retVal = NULL;
	AXP_VA		virtAddr = {.va = va};
	u32			oneChecked = virtAddr.vaIdxCntr.counter;
	int			ii;
	int			sets;

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
			(cpu->dCache[virtAddr.vaIdx.index][ii].physTag == pa))
		{
			retVal = cpu->dCache[virtAddr.vaIdx.index][ii].data;
			break;
		}
	}

	/*
	 * If we did not find it, then check the other 3 possible places.
	 */
	if (retVal == NULL)
	{
		for (virtAddr.vaIdxCntr.counter = 0;
			 ((virtAddr.vaIdxCntr.counter < 4) && (retVal == NULL));
			 virtAddr.vaIdxCntr.counter++)
		{
			if (virtAddr.vaIdxCntr.counter != oneChecked)
			{
				for (ii = 0; ((ii < sets) && (retVal == NULL)); ii++)
				{
					if ((cpu->dCache[virtAddr.vaIdx.index][ii].valid == true) &&
						(cpu->dCache[virtAddr.vaIdx.index][ii].physTag == pa))
						retVal = cpu->dCache[virtAddr.vaIdx.index][ii].data;
				}
			}
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
	int			ii;
	int			sets, whichSet;

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
	int			ii, jj;

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
	int			offset = vpc.vpcFields.offset % AXP_ICACHE_LINE_INS;
	int			ii;
	int			sets, whichSet;

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
	int			ii;
	int			sets;

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
