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
 * AXP_findTLBEntry
 *	This function is called to locate a TLB entry in either the Data or
 *	Instruction TLB, based off of the virtual address.
 *
 * Input Parameaters:
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
	u8				asn = (dtb ? cpu->dtbAsn0 : cpu->pCtx.asn);
	int				ii;

	/*
	 * Search through all valid TLB entries until we find the one we are being
	 * asked to return.
	 */
	for (ii = 0; ii < AXP_TLB_LEN; ii++)
	{
		if (tlbArray[ii].valid == true)
		{
			if ((tlbArray[ii].virtAddr ==
				(virtAddr & tlbArray[ii].matchMask)) &&
			   (tlbArray[ii].asn == asn) &&
			   (tlbArray[ii]._asm == true))
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
 * AXP_netNextFreeTLB
 *	This function is called to find the first TLB entry that is not being used
 *	(invalid).  Unlike the 21264 ARM indicates, we do not do this as a
 *	round-robin list.  We just find the first available, starting from the
 *	first entry.
 *
 *	TODO:	Determine if we want to have an index to the next available entry,
 *			instead of the first available entry.
 *
 * Input Parameters:
 *	tlbArray:
 *		A pointer to the array to be searched.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	NULL:		An available entry was not found.
 *	Not NULL:	A pointer to an unused TLB entry.
 */
AXP_21264_TLB *AXP_getNextFreeTLB(AXP_21264_TLB *tlbArray)
{
	AXP_21264_TLB 	*retVal = NULL;
	int				ii;

	for (ii = 0; ii < AXP_TLB_LEN; ii++)
		if (tlbArray[ii].valid == false)
		{
			retVal = tlbArray[ii];
			break;
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
 * Input Parameaters:
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
			tlbEntry = AXP_getNextFreeTLB(cpu->dtb);
		else
			tlbEntry = AXP_getNextFreeTLB(cpu->itb);
		// TODO:	What if there are no availebl TLB entries?
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
		 * If we do not have an outstanding TLM Miss, then we need to use the
		 * DTE_PTE0 and DTE_ASN0 IPRs.  Otherwise, we need to use the DTE_PTE1
		 * and the DTE_ASN1 IPRs.
		 *
		 * TODO: Verify that the above it actually true.
		 */
		if (cpu->tbMissOutstanding == false)
		{
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
			tlbEntry->asn = cpu->dtbAsn0.asn;
			tlbEntry->_asm = cpu->dtbPte0._asm;
		}
		else
		{
			tlbEntry->faultOnRead = cpu->dtbPte1._for;
			tlbEntry->faultOnWrite = cpu->dtbPte1.fow;
			tlbEntry->faultOnExecute = 0;
			tlbEntry->kre = cpu->dtbPte1.kre;
			tlbEntry->ere = cpu->dtbPte1.ere;
			tlbEntry->sre = cpu->dtbPte1.sre;
			tlbEntry->ure = cpu->dtbPte1.ure;
			tlbEntry->kwe = cpu->dtbPte1.kwe;
			tlbEntry->ewe = cpu->dtbPte1.ewe;
			tlbEntry->swe = cpu->dtbPte1.swe;
			tlbEntry->uwe = cpu->dtbPte1.uwe;
			tlbEntry->asn = cpu->dtbAsn1.asn;
			tlbEntry->_asm = cpu->dtbPte1._asm;
		}
	}
	else
	{

		/*
		 * The following fault-on-read/write/execute are hardcoded to keep the
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
		tlbEntry->asn = cpu->pCtx.asn;	// TODO: Is this right?
		tlbEntry->_asm = cpu->itbPte._asm;
	}
	tlbEntry->valid = true;				// Mark the TLB entry as valid.

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
 * Input Parameaters:
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
	for (ii = 0; ii < AXP_TLB_LEN; ii++)
		tlbArray[ii].valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_tbia
 * 	This function is called to invalidate all process-specific TLB entries (TLB
 *	entries that do not have the ASM bit set).
 *
 * Input Parameaters:
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
	for (ii = 0; ii < AXP_TLB_LEN; ii++)
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
 * Input Parameaters:
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
	AXP_21264_TLB	*tlb	tlb = AXP_findTLBEntry(cpu, va, dtb);

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
 *	This function is caller to detetermine if the process has the access needed
 *	to the memory location the are trying to use (read/write/modify/execute).
 *
 * Input Parameaters:
 *	cpu:
 *		A pointer to the CPU structure where the current process mode is
 *		located.
 *	tlb:
 *		A pointer to the Tramslation Lookaside Buffer that has the access
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
	if (tlb->v == true)
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
						retVal = ((tlb->kre == 1) && (tlb->_for == 1));
						break;

					case Write:
						retVal = ((tlb->kwe == 1) && (tlb->fow == 1));
						break;

					case Execute:
						retVal = ((tlb->kre == 1) && (tlb->foe == 1));
						break;

					case Modify:
						retVal = ((tlb->kwe == 1) && (tlb->kre) &&
								  (tlb->fow == 1) && (tlb->_for == 1));
						break;
				}
				break;

			case AXP_CM_EXEC;
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ere == 1) && (tlb->_for == 1));
						break;

					case Write:
						retVal = ((tlb->ewe == 1) && (tlb->fow == 1));
						break;

					case Execute:
						retVal = ((tlb->ere == 1) && (tlb->foe == 1));
						break;

					case Modify:
						retVal = ((tlb->ewe == 1) && (tlb->ere) &&
								  (tlb->fow == 1) && (tlb->_for == 1));
						break;
				}
				break;

			case AXP_CM_SUPER;
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->sre == 1) && (tlb->_for == 1));
						break;

					case Write:
						retVal = ((tlb->swe == 1) && (tlb->fow == 1));
						break;

					case Execute:
						retVal = ((tlb->sre == 1) && (tlb->foe == 1));
						break;

					case Modify:
						retVal = ((tlb->swe == 1) && (tlb->sre) &&
								  (tlb->fow == 1) && (tlb->_for == 1));
						break;
				}
				break;

			case AXP_CM_USER:
				switch(acc)
				{
					case None:
						break;

					case Read:
						retVal = ((tlb->ure == 1) && (tlb->_for == 1));
						break;

					case Write:
						retVal = ((tlb->uwe == 1) && (tlb->fow == 1));
						break;

					case Execute:
						retVal = ((tlb->ure == 1) && (tlb->foe == 1));
						break;

					case Modify:
						retVal = ((tlb->uwe == 1) && (tlb->ure) &&
								  (tlb->fow == 1) && (tlb->_for == 1));
						break;
				}
				break;
	}

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
 *		3)	A translation lookaside buffer (TLB) is located for the virtual
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
 *		A pointe to an unsigned 32-bit integer location to receive an indicator
 *		of whether a fault should occur.  This will cause the CPU to call the
 *		approprirate PALcode to handle the fault.
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
	 * If we are using a super page, then we need to go down that translation
	 * path.
	 */
	else if (spe != 0)
	{
		if (cpu->ierCm.cm != AXP_CM_KERNEL)
		{
			/* access violation */
		}
		else if ((spe & AXP_SPE2_BIT) && (vaSpe.spe2 == AXP_SPE2_VA_VAL))
		{
			pa = va & AXP_SPE2_VA_MASK;
			_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE1_BIT) && (vaSpe.spe1 == AXP_SPE1_VA_VAL))
		{
			pa = ((va & AXP_SPE1_VA_MASK) |
				  (va & AXP_SPE1_VA_40 ? AXP_SPE1_PA_43_41 : 0));
			_asm = false;
			return(pa);
		}
		else if ((spe & AXP_SPE0_BIT) && (vaSpe.spe0 == AXP_SPE0_VA_VAL))
		{
			pa = va & AXP_SPE0_VA_MASK;
			_asm = false;
			return(pa);
		}
	}

	/*
	 * We need to see if we can find a TLB entry for this virtual address.  We
	 * get here, either when we are not in PALmode, not using a Superpage, or
	 * the virtual address did not contain the expected superpage values.
	 */
	tlb = AXP_findTLBEntry(cpu, va, dtb);

	/*
	 * We were unable to find a TLB entry for this virtual address.  We need to
	 * call the PALcode to fill in for the TLB Miss.
	 */
	if (tlb == NULL)
	{

		/*
		 * TODO: The caller nees to set the following.
		 *	cpu->excAddr = pc;
		 *	if (cpu->tbMissOutstanding == false)
		 *		cpu->mmStat.? = 1;
		 *	cpu->va = va;
		 *	cpu->excSum.? = 1;
		 */
		if (cpu->tbMissOutstanding == true)
		{
			if (cpu->iCtrl.va_48 == 0)
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
			cpu->excAddr = pc;
			if (dtb == true)
			{

				/*
				 * TODO: The caller nees to set the following.
				 *	cpu->excSum.? = 1;
				 *	cpu->mmStat.? = 1;
				 *	cpu->va = va;
				 */
				*fault = AXP_DFAULT;
			}
			else
			{

				/*
				 * TODO: The caller nees to set the following.
				 *	cpu->excSum = 0;
				 */
				*fault = AXP_IACV;
			}
		}
		else
		{
			pa = tlb->physAddr | (va & tlb->keepMask)
			_asm = tlb->_asm;
		}
	}

	/*
	 * Return the physical address (zero if we could not determine it) back to
	 * the caller.  If 0x0ll is returned, then the fault output parameter
	 * should have the fault reason for not returning a valid physical address.
	 */
	return(pa);
}
