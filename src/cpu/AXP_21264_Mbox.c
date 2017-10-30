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
 *	functionality of the Mbox.
 *
 *	Revision History:
 *
 *	V01.000		19-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_21264_Mbox.h"

/*
 * AXP_21264_Mbox_GetLQSlot
 *	This function is called to get the next available Load slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The LQ slot index is incremented, if there is room for another load
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Load instruction.  If there are no
 *	slots available a value of the size of the LoadQueue will be returned.
 */
u32 AXP_21264_Mbox_GetLQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As loads are completed, the index will be reduced.
	 */
	if (cpu->lqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->lqNext++;
		cpu->lq[retVal].state = Assigned;
	}

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}
/*
 * AXP_21264_Mbox_ReadMem
 *	This function is called to queue up a read from Dcache based on a virtual
 *	address, size of the data to be read and the instruction that is queued up
 *	to be completed in order.  This function works with the AXP_Mbox_WriteMem
 *	function to ensure correct Alpha memory reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the read is complete, the
 *		value from memory is store in the instr->destv location and the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Load Queue (LQ) where this read entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be read is located.
 *	length:
 *		A value indicating the length, in bytes, of the value to read from the
 *		Dcache.
 *
 * Output Parameters:
 *	instr:
 *		A pointer to the decoded instruction.  It is updated with the value
 *		read from the Dcache.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_ReadMem(AXP_21264_CPU *cpu,
							AXP_INSTRUCTION *instr,
							u32 slot,
							u64 virtAddr,
							u32 length)
{
	
	/*
	 * Store the information in the
	 */
	cpu->lq[slot].length = length;
	cpu->lq[slot].virtAddress = virtAddr;
	cpu->lq[slot].instr = instr;
	cpu->lq[slot].state = ReadPending;

	/*
	 * Return back to the caller.
	 */
	return;
 }
 
/*
 * AXP_21264_Mbox_GetSQSlot
 *	This function is called to get the next available Store slot.  They are
 *	assigned in instruction order.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *
 * Output Parameters:
 *	cpu:
 *		The SQ slot index is incremented, if there is room for another store
 *		request.
 *
 * Return Value:
 *	The value of the slot to be used for the Store instruction.  If there are no
 *	slots available a value of the size of the StoreQueue will be returned.
 */
u32 AXP_21264_Mbox_GetSQSlot(AXP_21264_CPU *cpu)
{
	u32 retVal = AXP_MBOX_QUEUE_LEN;

	/*
	 * If there is another slot available, get is to return to the caller and
	 * increment the index.  As stores are completed, the index will be reduced.
	 */
	if (cpu->sqNext < AXP_MBOX_QUEUE_LEN)
	{
		retVal = cpu->sqNext++;
		cpu->sq[retVal].state = Assigned;
	}

	/*
	 * Returned the assigned slot back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Mbox_WriteMem
 *	This function is called to queue up a write to the Dcache based on a
 *	virtual address, size of the data to be written, the value of the data and
 *	the instruction that is queued up to be completed in order.  This function
 *	works with the AXP_Mbox_ReadMem function to ensure correct Alpha memory
 *	reference behavior.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate 
 *		a single CPU.
 *	instr:
 *		A pointer to the decoded instruction.  When the write is completed, the
 *		instruction marked as WaitingForCompletion.
 *	slot:
 *		A value indicating the assigned Store Queue (SQ) where this write entry
 *		is to be stored.
 *	virtAddr:
 *		A value containing the virtual address, full 64-bits, where the value
 *		to be written is located.
 *	value:
 *		The value to be be written to the Dcache (and ultimately memory).
 *	length:
 *		A value indicating the length, in bytes, of the value to be written to
 *		the Dcache.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Mbox_WriteMem(AXP_21264_CPU *cpu,
							 AXP_INSTRUCTION *instr,
							 u32 slot,
							 u64 virtAddr,
							 u64 value,
							 u32 length)
{
	
	/*
	 * Store the information in the
	 */
	cpu->sq[slot].value = value;
	cpu->sq[slot].length = length;
	cpu->sq[slot].virtAddress = virtAddr;
	cpu->sq[slot].instr = instr;
	cpu->sq[slot].state = WritePending;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Mbox_Init
 *	This function is called by the Cbox to initialize the Mbox items.  These
 *	items are:
 *
 *		- Dcache
 *		- Dcache Tag
 *		- Duplicate Tag
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	Failed to initialize the Mbox
 *	false:	Successful normal completion.
 */
bool AXP_21264_Mbox_Init(AXP_21264_CPU *cpu)
{
	bool retVal = false;
	int ii, jj;

	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			memset(cpu->dCache[ii][jj].data, 0, AXP_DCACHE_DATA_LEN);
			cpu->dCache[ii][jj].physTag = 0;
			cpu->dCache[ii][jj].valid = false;
			cpu->dCache[ii][jj].dirty = false;
			cpu->dCache[ii][jj].shared = false;
			cpu->dCache[ii][jj].modified = false;
			cpu->dCache[ii][jj].set_0_1 = false;;
			cpu->dCache[ii][jj].locked = false;;
			cpu->dCache[ii][jj].state = Invalid;
		}
	}
	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			cpu->dtag[ii][jj].physTag = 0;
			cpu->dtag[ii][jj].ctagIndex = AXP_CACHE_ENTRIES;
			cpu->dtag[ii][jj].ctagSet = AXP_2_WAY_CACHE;
			cpu->dtag[ii][jj].valid = false;
		}
	}
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->lq[ii].value = 0;
		cpu->lq[ii].length = 0;
		cpu->lq[ii].virtAddress = 0;
		cpu->lq[ii].instr = NULL;
		cpu->lq[ii].state = QNotInUse;
	}
	cpu->lqNext = 0;
	for (ii = 0; ii < AXP_MBOX_QUEUE_LEN; ii++)
	{
		cpu->sq[ii].value = 0;
		cpu->sq[ii].length = 0;
		cpu->sq[ii].virtAddress = 0;
		cpu->sq[ii].instr = NULL;
		cpu->sq[ii].state = QNotInUse;
	}
	cpu->sqNext = 0;
	for (ii = 0; ii < AXP_TB_LEN; ii++)
	{
		cpu->dtb[ii].virtAddr = 0;
		cpu->dtb[ii].physAddr = 0;
		cpu->dtb[ii].matchMask = 0;
		cpu->dtb[ii].keepMask = 0;
		cpu->dtb[ii].kre = 0;
		cpu->dtb[ii].ere = 0;
		cpu->dtb[ii].sre = 0;
		cpu->dtb[ii].ure = 0;
		cpu->dtb[ii].kwe = 0;
		cpu->dtb[ii].ewe = 0;
		cpu->dtb[ii].swe = 0;
		cpu->dtb[ii].uwe = 0;
		cpu->dtb[ii].faultOnRead = 0;
		cpu->dtb[ii].faultOnWrite = 0;
		cpu->dtb[ii].faultOnExecute = 0;
		cpu->dtb[ii].res_1 = 0;
		cpu->dtb[ii].asn = 0;
		cpu->dtb[ii]._asm = false;
		cpu->dtb[ii].valid = false;
	}
	cpu->nextDTB = 0;
	for (ii = 0; ii < AXP_21264_MAF_LEN; ii++)
	{
		cpu->maf[ii].type = MNotInUse;
		cpu->maf[ii].rq = NOPcmd;
		cpu->maf[ii].rsp = NOPsysdc;
		cpu->maf[ii].pa = 0;
		cpu->maf[ii].valid = false;
		cpu->maf[ii].complete = false;
	}
	cpu->tbMissOutstanding = false;;
	cpu->dtbTag0.res_1 = 0;
	cpu->dtbTag0.va = 0;
	cpu->dtbTag0.res_2 = 0;
	cpu->dtbTag1.res_1 = 0;
	cpu->dtbTag1.va = 0;
	cpu->dtbTag1.res_2 = 0;
	cpu->dtbPte0.res_1 = 0;
	cpu->dtbPte0.pa = 0;
	cpu->dtbPte0.res_2 = 0;
	cpu->dtbPte0.uwe = 0;
	cpu->dtbPte0.swe = 0;
	cpu->dtbPte0.ewe = 0;
	cpu->dtbPte0.kwe = 0;
	cpu->dtbPte0.ure = 0;
	cpu->dtbPte0.sre = 0;
	cpu->dtbPte0.ere = 0;
	cpu->dtbPte0.kre = 0;
	cpu->dtbPte0.res_3 = 0;
	cpu->dtbPte0.gh = 0;
	cpu->dtbPte0._asm = 0;
	cpu->dtbPte0.res_4 = 0;
	cpu->dtbPte0.fow = 0;
	cpu->dtbPte0._for = 0;
	cpu->dtbPte0.res_5 = 0;
	cpu->dtbPte1.res_1 = 0;
	cpu->dtbPte1.pa = 0;
	cpu->dtbPte1.res_2 = 0;
	cpu->dtbPte1.uwe = 0;
	cpu->dtbPte1.swe = 0;
	cpu->dtbPte1.ewe = 0;
	cpu->dtbPte1.kwe = 0;
	cpu->dtbPte1.ure = 0;
	cpu->dtbPte1.sre = 0;
	cpu->dtbPte1.ere = 0;
	cpu->dtbPte1.kre = 0;
	cpu->dtbPte1.res_3 = 0;
	cpu->dtbPte1.gh = 0;
	cpu->dtbPte1._asm = 0;
	cpu->dtbPte1.res_4 = 0;
	cpu->dtbPte1.fow = 0;
	cpu->dtbPte1._for = 0;
	cpu->dtbPte1.res_5 = 0;
	cpu->dtbAltMode = AXP_MBOX_ALTMODE_KERNEL;
	cpu->dtbIs0.res_1 = 0;
	cpu->dtbIs0.inval_itb = 0;
	cpu->dtbIs0.res_2 = 0;
	cpu->dtbIs1.res_1 = 0;
	cpu->dtbIs1.inval_itb = 0;
	cpu->dtbIs1.res_2 = 0;
	cpu->dtbAsn0.res_1 = 0;
	cpu->dtbAsn0.asn = 0;
	cpu->dtbAsn0.res_2 = 0;
	cpu->dtbAsn1.res_1 = 0;
	cpu->dtbAsn1.asn = 0;
	cpu->dtbAsn1.res_2 = 0;
	cpu->mmStat.res = 0;
	cpu->mmStat.dc_tag_perr = 0;
	cpu->mmStat.opcodes = 0;
	cpu->mmStat.fow = 0;
	cpu->mmStat._for = 0;
	cpu->mmStat.acv = 0;
	cpu->mmStat.wr = 0;
	cpu->mCtl.res_1 = 0;
	cpu->mCtl.spe = 0;
	cpu->mCtl.res_2 = 0;
	cpu->dcCtl.dcdat_err_en = 0;
	cpu->dcCtl.dctag_par_en = 0;
	cpu->dcCtl.f_bad_decc = 0;
	cpu->dcCtl.f_bad_tpar = 0;
	cpu->dcCtl.f_hit = 0;
	cpu->dcCtl.set_en = 3;				/* use both Dcache sets */
	cpu->dcStat.res = 0;
	cpu->dcStat.seo = 0;
	cpu->dcStat.ecc_err_ld = 0;
	cpu->dcStat.ecc_err_st = 0;
	cpu->dcStat.tperr_p1 = 0;
	cpu->dcStat.tperr_p1 = 0;

	/*
	 * All done, return to the caller.
	 */
	return(retVal);
}
