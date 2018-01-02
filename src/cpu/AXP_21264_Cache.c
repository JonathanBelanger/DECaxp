
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
 *	functions will remain the same, as there is only one writer (the Cbox) and
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
 *
 *	V01.004		18-Dec-2017	Jonathan D. Belanger
 *	The code that stores data into the caches does not use the offset like they
 *	should.  This needs to be fixed.
 *
 *	V01.005		20-Dec-2017	Jonathan D. Belanger
 *	The Dcache code needs to be changed and simplified.
 */
#include "AXP_21264_Cache.h"

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
/*	and the super page settings within the Ibox Control Register or the		*/
/*	Mbox Dcache Control Register.											*/
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
AXP_EXCEPTIONS AXP_21264_checkMemoryAccess(
					AXP_21264_CPU *cpu,
					AXP_21264_TLB *tlb,
					AXP_21264_ACCESS acc)
{
	AXP_EXCEPTIONS	retVal = NoException;

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
						if ((tlb->kre == 0) || (tlb->faultOnRead == 1))
							retVal = FaultOnRead;
						break;

					case Write:
						if ((tlb->kwe == 0) || (tlb->faultOnWrite == 1))
							retVal = FaultOnWrite;
						break;

					case Execute:
						if ((tlb->kre == 0) || (tlb->faultOnExecute == 1))
							retVal = FaultOnExecute;
						break;

					case Modify:	/* TODO: Is this correct? */
						if (((tlb->kwe == 0) || (tlb->kre == 0)) ||
							((tlb->faultOnWrite == 1) ||
							 (tlb->faultOnRead == 1)))
						   retVal = AccessControlViolation;
						break;
				}
				break;

			case AXP_CM_EXEC:
				switch(acc)
				{
					case None:
						break;

					case Read:
						if ((tlb->ere == 0) || (tlb->faultOnRead == 1))
							retVal = FaultOnRead;
						break;

					case Write:
						if ((tlb->ewe == 0) || (tlb->faultOnWrite == 1))
							retVal = FaultOnWrite;
						break;

					case Execute:
						if ((tlb->ere == 0) || (tlb->faultOnExecute == 1))
							retVal = FaultOnExecute;
						break;

					case Modify:
						if (((tlb->ewe == 0) || (tlb->ere == 0)) ||
							((tlb->faultOnWrite == 1) ||
							 (tlb->faultOnRead == 1)))
							retVal = AccessControlViolation;
						break;
				}
				break;

			case AXP_CM_SUPER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						if ((tlb->sre == 0) || (tlb->faultOnRead == 1))
							retVal = FaultOnRead;
						break;

					case Write:
						if ((tlb->swe == 0) || (tlb->faultOnWrite == 1))
							retVal = FaultOnWrite;
						break;

					case Execute:
						if ((tlb->sre == 0) || (tlb->faultOnExecute == 1))
							retVal = FaultOnExecute;
						break;

					case Modify:
						if (((tlb->swe == 0) || (tlb->sre == 0)) ||
							((tlb->faultOnWrite == 1) ||
							 (tlb->faultOnRead == 1)))
							retVal = AccessControlViolation;
						break;
				}
				break;

			case AXP_CM_USER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						if ((tlb->ure == 0) || (tlb->faultOnRead == 1))
							retVal = FaultOnRead;
						break;

					case Write:
						if ((tlb->uwe == 0) || (tlb->faultOnWrite == 1))
							retVal = FaultOnWrite;
						break;

					case Execute:
						if ((tlb->ure == 0) || (tlb->faultOnExecute == 1))
							retVal = FaultOnExecute;
						break;

					case Modify:
						if (((tlb->uwe == 0) || (tlb->ure == 0)) ||
							((tlb->faultOnWrite == 1) ||
							 (tlb->faultOnRead == 1)))
							retVal = AccessControlViolation;
						break;
				}
				break;
		}	/* switch(cpu->ierCm.cm) */
	}		/* if (tlb->valid == true) */
	else
		retVal = AccessControlViolation;

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
 *	memChk:
 *		A pointer to a location to receive an indicate of the exception that
 *		needs to be processed by the PALcode.
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
		u32 *fault,
		AXP_EXCEPTIONS *memChk)
{
	AXP_21264_TLB	*tlb;
	AXP_VA_SPE		vaSpe = {.va = va};
	u64				pa = 0x0ll;
	u8				spe = (dtb ? cpu->mCtl.spe : cpu->iCtl.spe);

	/*
	 * Initialize the output parameters.
	 */
	*_asm = false;
	*fault = 0;
	*memChk = NoException;

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
		*memChk = AXP_21264_checkMemoryAccess(cpu, tlb, acc);
		if (*memChk != NoException)
		{
			if (dtb == true)
			{
				*fault = AXP_DFAULT;
			}
			else
			{
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
 * AXP_Dcache_Status
 *	This function is called to determine the status of the Dcache block
 *	associated with a particular Virtual Address (VA) and Physical Address (PA)
 *	pair.  If a hit is detected, then information will be returned to the
 *	caller to be used in a subsequent Dcache Read/Write so that we do not have
 *	to search of the entry again.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the AXP 21264 CPU structure containing the current
 *		execution mode and the DTB (for Data) and ITB (for Instructions)
 *		arrays.
 * 	va:
 * 		The virtual address of the data in virtual memory.
 * 	pa:
 * 		The physical address, as stored in the DTB (Data TLB).
 *	len:
 *		A value indicating the length of the data to be stored/read from the
 *		Dcache.  This parameter must be one of the following values
 *		(assumed unsigned):
 *			 1	= byte
 *			 2	= word
 *			 4	= longword
 *			 8	= quadword
 *			64	= 64 bytes of data being copied from memory to the Dcache
 *	disableUnaligned:
 *		A boolean to disable reporting unaligned data faults.  This is to
 *		support the STQ_U and HW_ST instructions.
 *	evict:
 *		A boolean to indicate if the bit should be set to allow this Dcache
 *		entry to be evicted the next time we need this slot (the slot to use is
 *		not updated).
 *
 * Output Parameters:
 *	indexSetOffset:
 *		A location receive the index, set, and offset for the cache block to be
 *		used by the AXP_DcacheWrite and AXP_DcacheRead functions.
 *	status:
 *		An unsigned 32-bit location to receive a masked value with the
 *		following bits set as appropriate:
 *			AXP_21264_CACHE_MISS
 *			AXP_21264_CACHE_HIT
 *			AXP_21264_CACHE_DIRTY
 *			AXP_21264_CACHE_SHARED
 *
 * Return Value:
 *	NoException	= Normal Successful Completion
 *	Unaligned	= Unaligned memory reference
 *
 * NOTE:	This function should be called without the Mbox IPR mutex locked.
 *			It'll temporarily lock the Mbox IPR mutex to retrieve the number
 *			of sets in use and then unlock it.  Then it'll lock the DTAG
 *			mutex, as the Dcache Mutex is not required, to find the entry in
 *			the Dcache/DTAG to be tested, if found, and then unlock the DTAG
 *			mutex.
 */
AXP_EXCEPTIONS AXP_Dcache_Status(
						AXP_21264_CPU *cpu,
						u64 va,
						u64 pa,
						u32 len,
						bool disableUnaligned,
						u32 *status,
						AXP_DCACHE_LOC *indexSetOffset,
						bool evict)
{
	AXP_VA			virtAddr = {.va = va};
	AXP_VA			physAddr = {.va = pa};
	AXP_EXCEPTIONS	retVal = NoException;
	u64				tag = physAddr.vaIdxInfo.tag;
	u32				index = virtAddr.vaIdx.index & 0x07f;
	u32				set = 0;
	u32				activeSets;
	u32				offset = virtAddr.vaIdx.offset;
	u32				ii, jj;

	/*
	 * Before we do anything, let's determine if the virtual address for the
	 * data size is not properly aligned.  We don't do this for to of the store
	 * instructions where they are intended to work on unaligned data.
	 */
	if (disableUnaligned == false)
	{
		switch (len)
		{
			case 1:
				break;

			case 2:
				if ((va & 0xfffffffffffffffe) != va)
					retVal = DataAlignmentTrap;
				break;

			case 4:
				if ((va & 0xfffffffffffffffc) != va)
					retVal = DataAlignmentTrap;
				break;

			case 8:
				if ((va & 0xfffffffffffffff8) != va)
					retVal = DataAlignmentTrap;
				break;

			case 64:
				if ((va & 0xffffffffffffffc0) != va)
					retVal = DataAlignmentTrap;
				break;
		}
	}

	/*
	 * If not exception was detected, then let's go see if we can find the
	 * entry in the Dcache for the specific VA/PA.
	 */
	if (retVal == NoException)
	{

		/*
		 * Before we go too far, initialize the return values to "nothing".
		 */
		if (indexSetOffset != NULL)
		{
			indexSetOffset->set = 0;
			indexSetOffset->offset = 0;
			indexSetOffset->index = 0;
		}
		*status = AXP_21264_CACHE_MISS;

		/*
		 * Get the number of Dcache sets currently in play.  It is either 1 or
		 * 2.  Don't forget to lock/unlock the Mbox IPR mutex around this, so
		 * that it is not changed while we are looking at it.
		 */
		pthread_mutex_lock(&cpu->mBoxIPRMutex);
		activeSets = (cpu->dcCtl.set_en == 1) ? 1 : 2;
		pthread_mutex_unlock(&cpu->mBoxIPRMutex);

		/*
		 * Make sure we lock the DTAG mutex before we do anything else.  We
		 * don't want to be interrupted while we are doing this work.
		 */
		pthread_mutex_lock(&cpu->dtagMutex);

		/*
		 * Because the number of Dcache entries is large enough that the index
		 * shares 2 bits from the VPN, it is possible that a particular
		 * physical address may be in anyone of 4 locations.  Therefore, we
		 * need to find the first location that actually contains the item for
		 * which we are looking.  Since it is the high 2 bits, of a 9 bit value,
		 * we just scroll through the setting of these 2 bits (note: the 2 bits
		 * in question were cleared above, so that the code below is simpler).
		 */
		for (ii = index; ((ii < index + 0x180) & (*status == 0)); ii += 0x080)
		{

			/*
			 * Search through each of the possible sets.
			 */
			for (jj = 0; ((jj < activeSets) & (*status == 0)); jj++)
			{
				if ((cpu->dtag[ii][jj].valid == true) &&
					(cpu->dtag[ii][jj].physTag == tag))
				{
					*status = AXP_21264_CACHE_HIT;
					if (cpu->dtag[ii][jj].dirty == true)
						*status |= AXP_21264_CACHE_DIRTY;
					if (cpu->dtag[ii][jj].shared == true)
						*status |= AXP_21264_CACHE_SHARED;
					if (indexSetOffset != NULL)
					{
						indexSetOffset->set = jj;
						indexSetOffset->offset = offset;
						indexSetOffset->index = ii;
					}
				}
			}
		}

		/*
		 * HRM 4.5.5: If we did not get a Hit, then we need to see if we need
		 * to evict someone.  We perform the following steps:
		 *
		 *		1.	First, see if the slot we are supposed to use it is use and
		 *			the modified bit set.
		 *		2.	If #1 is true, then evict it as necessary.
		 *		3.	Either way, indicate that this location is waiting to be
		 *			filled.
		 *
		 * NOTE:	If the block is evicted, then it is sent to the Cbox for
		 *			processing.
		 *			Also, we can evict the block here, even if it gets filled
		 *			directly from the Bcache.  The Bcache is much larger, and
		 *			this is possible, but we still need to evict the current
		 *			block.
		 */
		if ((*status == 0) && (indexSetOffset != NULL))
		{
			u32		ctagIndex = cpu->dtag[index][set].ctagIndex;
			u32		ctagSet = cpu->dtag[index][set].ctagSet;

			/*
			 * First thing we need to do is determine which set we are to be
			 * using.
			 */
			set = (cpu->dtag[index][0].set_0_1 == false) ? 0 : 1;

			/*
			 * If the entry we are to be using is valid and has been modified,
			 * then we need to write it to the Bcache.
			 */
			if ((cpu->dtag[index][set].valid == true) &&
				(cpu->dtag[index][set].modified == true))
			{
				pthread_mutex_lock(&cpu->dCacheMutex);
				AXP_21264_Add_VDB(
								cpu,
								toBcache,
								pa,
								cpu->dCache[index][set].data,
								false,
								false);
				pthread_mutex_unlock(&cpu->dCacheMutex);
			}

			/*
			 * No reset the DTAG to indicate that the block is valid but in a
			 * pending state, until filled.  Then, unhook the associated CTAG
			 * block, resetting it as well, then link the new CTAG with this
			 * DTAG.
			 */
			cpu->dtag[index][set].valid = false;
			cpu->dtag[index][set].dirty = false;
			cpu->dtag[index][set].modified = false;
			cpu->dtag[index][set].shared = false;
			cpu->dtag[index][set].physTag = physAddr.vaIdxInfo.tag;
			cpu->dtag[index][set].state = Pending;

			/*
			 * Before we reset the associated CTAG, lock the mutex for it.
			 * We'll keep it lock while we link the new CTAG.
			 */
			pthread_mutex_lock(&cpu->cBoxIPRMutex);
			cpu->ctag[ctagIndex][ctagSet].valid = false;
			cpu->ctag[ctagIndex][ctagSet].dirty = false;
			cpu->ctag[ctagIndex][ctagSet].shared = false;
			cpu->ctag[ctagIndex][ctagSet].physTag = 0;
			cpu->ctag[ctagIndex][ctagSet].dtagIndex = 0;

			/*
			 * The CTAG is physically indexed and tagged, unlike the Dcache,
			 * which is virtually indexed, but physically cached.
			 */
			ctagIndex = physAddr.vaIdxInfo.index;
			ctagSet = set;
			cpu->ctag[ctagIndex][ctagSet].valid = true;
			cpu->ctag[ctagIndex][ctagSet].dirty = false;
			cpu->ctag[ctagIndex][ctagSet].shared = false;
			cpu->ctag[ctagIndex][ctagSet].physTag = physAddr.vaIdxInfo.tag;
			cpu->ctag[ctagIndex][ctagSet].dtagIndex = index;
			pthread_mutex_unlock(&cpu->cBoxIPRMutex);

			/*
			 * If this "new" block should be evicted next time we need the
			 * Dcache location, then don't change the flag on whether set 0 or
			 * set 1 should be used.
			 */
			if ((evict == false) && (activeSets == 2))
				cpu->dtag[index][0].set_0_1 = ~cpu->dtag[index][0].set_0_1;

			/*
			 * Even when we don't have a hit, we still return the location
			 * where the Dcache data should be placed.  If the data is in the
			 * Bcache, it is copied directly into this location.  If it is not,
			 * then it is copied from memory into this location by the Cbox.
			 */
			if (indexSetOffset != NULL)
			{
				indexSetOffset->set = set;
				indexSetOffset->offset = offset;
				indexSetOffset->index = index;
			}
		}

		/*
		 * Last thing we need to do is unlock the DTAG mutex so other
		 * accessors (Cbox) can to it's thing.
		 */
		pthread_mutex_unlock(&cpu->dtagMutex);
	}

	/*
	 * Return what we found from checking out the Dcache.
	 */
	return(retVal);
}

/*
 * AXP_DcacheWrite
 * 	This function is called to add/update a cache entry into the Data Cache.
 *
 *	This function is called as a result of a request to the Cbox to perform a
 *	Dcache fill.  In this case, the length is always 64 bytes and the address,
 *	0xffffffffffffffc0.
 *
 *	Refer to Section 2.8.3 in the HRM for more information about processing
 *	Memory Address Space Store Instructions from the Store Queue (SQ).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the Digital Alpha AXP 21264 CPU structure containing the
 * 		Data Cache (dCache) array.
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
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	true:	Normal Successful Completion.
 * 	false:	An error occurred.
 */
bool AXP_DcacheWrite(
			AXP_21264_CPU *cpu,
			AXP_DCACHE_LOC *indexSetOffset,
			u32 len,
			void *data)
{
	bool	retVal = true;
	u32		set = indexSetOffset->set;
	u32		offset = indexSetOffset->offset;
	u32		index = indexSetOffset->index;
	void	*dest = (void *) &cpu->dCache[index][set].data[offset];

	/*
	 * First, lock both the DTAG mutex and the Dcache mutex.
	 */
	pthread_mutex_lock(&cpu->dtagMutex);
	pthread_mutex_lock(&cpu->dCacheMutex);

	switch (len)
	{
		case 1:
			*((u8 *) dest) = *((u8 *) data);
			break;

		case 2:
			*((u16 *) dest) = *((u16 *) data);
			break;

		case 4:
			*((u32 *) dest) = *((u32 *) data);
			break;

		case 8:
			*((u64 *) dest) = *((u8 *) data);
			break;

		case 64:
			memcpy(cpu->dCache[index][set].data, data, len);
			cpu->dtag[index][set].dirty = true;		/* TODO: Do we need more here? */
			cpu->dtag[index][set].modified = true;
			cpu->dtag[index][set].state = Ready;
			break;
	}

	/*
	 * Return the result back to the caller.
	 */
	pthread_mutex_unlock(&cpu->dCacheMutex);
	pthread_mutex_unlock(&cpu->dtagMutex);
	return(retVal);
}

/*
 * AXP_CopyBcacheToDcache
 * 	This function is called because we found a Bcache block that can be used
 * 	to fill a Dcache reference.  We need to not only fetch the block of data
 * 	but also the various bits.
 *
 * Input Parameters:
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 * 	None.
 *
 * NOTE:	This function is called with the Bcache locked, but not the Dcache.
 */
void AXP_CopyBcacheToDcache(
				AXP_21264_CPU *cpu,
				AXP_DCACHE_LOC *indexSetOffset,
				u64 pa)
{
	u32	index = indexSetOffset->index;
	u32	set = indexSetOffset->set;

	/*
	 * We are going to play with the Dcache and DTAG arrays.  Make sure we lock
	 * them down.
	 */
	pthread_mutex_lock(&cpu->dtagMutex);
	pthread_mutex_lock(&cpu->dCacheMutex);
	AXP_21264_Bcache_Read(
					cpu,
					pa,
					cpu->dCache[index][set].data,
					&cpu->dtag[index][set].dirty,
					&cpu->dtag[index][set].shared);
	cpu->dtag[index][set].modified = false;
	cpu->dtag[index][set].state = Ready;

	/*
	 * We're done here, unlock the Dcache and DTAG mutexes.
	 */
	pthread_mutex_unlock(&cpu->dCacheMutex);
	pthread_mutex_unlock(&cpu->dtagMutex);
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
	int			ii, jj;
	int			ctagIdx;
	u64			pa;

	/*
	 * First we need to lock access to the Dcache and Dtag.
	 */
	pthread_mutex_lock(&cpu->dCacheMutex);
	pthread_mutex_lock(&cpu->dtagMutex);
	pthread_mutex_lock(&cpu->bCacheMutex);

	/*
	 * Go through each cache item and invalidate and reset it.
	 */
	for (ii = 0; ii < AXP_CACHE_ENTRIES; ii++)
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			ctagIdx = cpu->dtag[ii][jj].ctagIndex;
			if ((cpu->dtag[ii][jj].valid == true) &&
				(cpu->dtag[ii][jj].modified == true))
			{

				/*
				 * TODO:	We need to do this differently, as we don't have
				 *			the real physical address.
				 */
				pa = cpu->ctag[ctagIdx][jj].physTag;
				/* Update the Bcache with this modified block. */
				AXP_21264_Bcache_Write(
					cpu,
					pa,
					cpu->dCache[ii][jj].data);
				cpu->dtag[ii][jj].modified = false;
			}
			cpu->dtag[ii][jj].set_0_1 = false;
			cpu->dtag[ii][jj].physTag = 0;
			cpu->dtag[ii][jj].dirty = false;
			cpu->dtag[ii][jj].modified = false;
			cpu->dtag[ii][jj].shared = false;
			cpu->dtag[ii][jj].valid = false;
			cpu->ctag[ctagIdx][jj].physTag = 0;
			cpu->ctag[ctagIdx][jj].dirty = false;
			cpu->ctag[ctagIdx][jj].shared = false;
			cpu->ctag[ctagIdx][jj].valid = false;
		}

	/*
	 * Finally we need to unlock access to the Dcache and Dtag.  Always unlock
	 * in the reverse order of how they were locked.  Also,  always make sure
	 * that these locks are locked in the same order where ever both are
	 * locked.
	 */
	pthread_mutex_unlock(&cpu->bCacheMutex);
	pthread_mutex_unlock(&cpu->dtagMutex);
	pthread_mutex_unlock(&cpu->dCacheMutex);

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
	i32					lenOver = ((va % AXP_DCACHE_DATA_LEN) + len - 1) -
						  	  	   (AXP_DCACHE_DATA_LEN - 1);
	u32					offset = virtAddr.vaIdx.offset;
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
	 * We do this here to avoid a deadlock if the above recursive call is made.
	 * Because the Dcache can be access by the Mbox and Cbox, we need to
	 * prevent multiple accessors to these structures.
	 */
	pthread_mutex_lock(&cpu->dCacheMutex);

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
		if ((cpu->dtag[virtAddr.vaIdx.index][ii].valid == true) &&
			(cpu->dtag[virtAddr.vaIdx.index][ii].physTag ==
			 physAddr.vaIdxInfo.tag))
		{

			/*
			 * Combine the index and set associated with this data and return
			 * that into the caller supplied parameter.
			 */
			indexAndSet.idxOrSet.index = virtAddr.vaIdx.index;
			indexAndSet.idxOrSet.set = ii;
			if (idxSet != NULL)
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
				*((u8 *) data) = *((u8 *) &cpu->dCache[index][set].data[offset]);
				break;

			case 2:
				*((u16 *) data) = *((u16 *) &cpu->dCache[index][set].data[offset]);
				break;

			case 4:
				*((u32 *) data) = *((u32 *) &cpu->dCache[index][set].data[offset]);
				break;

			case 8:
				*((u64 *) data) = *((u64 *) &cpu->dCache[index][set].data[offset]);
				break;

			default:
				memcpy(data, &cpu->dCache[index][set].data[offset], len);
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
	pthread_mutex_unlock(&cpu->dCacheMutex);
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
	 * First things first, we need to lock the Icache from being updated by
	 * anyone but us.
	 */
	pthread_mutex_lock(&cpu->iCacheMutex);

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
	pthread_mutex_unlock(&cpu->iCacheMutex);
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

	/*
	 * First things first, we need to lock the Icache from being updated by
	 * anyone but us.
	 */
	pthread_mutex_lock(&cpu->iCacheMutex);

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
	pthread_mutex_unlock(&cpu->iCacheMutex);
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
	 * First things first, we need to lock the Icache from being updated by
	 * anyone but us.
	 */
	pthread_mutex_lock(&cpu->iCacheMutex);

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
	pthread_mutex_unlock(&cpu->iCacheMutex);
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
	 * First things first, we need to lock the Icache from being updated by
	 * anyone but us.
	 */
	pthread_mutex_lock(&cpu->iCacheMutex);

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
	pthread_mutex_unlock(&cpu->iCacheMutex);
	return(retVal);
}
