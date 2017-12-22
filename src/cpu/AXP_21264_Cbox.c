
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
 *	functionality of the Cbox.
 *
 *	Revision History:
 *
 *	V01.000		26-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		28-Oct-2017	Jonathan D. Belanger
 *	Time to fill out the functionality of the Cbox.  Some of the functions
 *	previously defined, may be going away.  We are going to need functions for
 *	all the requests needed by the CPU of the system (from Mbox, Ibox, and
 *	Cbox).  Additionally, IRQ_H interrupt signals from the System to the Ibox
 *	for processing.
 *
 *	V01.002		16-Nov-2017	Jonathan D. Belanger
 *	Added functions to Flush, Read, Write, and Check for a Valid record for
 *	the Bcache.
 *
 *	V01.003		22-Dec-2017	Jonathan D. Belanger
 *	Worked through the IOWB processing.  This code is pretty much complete.
 */
#include "AXP_21264_Cbox.h"
#include "AXP_Configure.h"
#include "AXP_NameValuePair_Read.h"
#include "AXP_21264_CacheDefs.h"
#include "AXP_21264_Mbox.h"
#include "AXP_21264_Ebox.h"
#include "AXP_21264_Fbox.h"
#include "AXP_21264_Ibox.h"

/*
 * Local Variables
 */
AXP_21264_CBOX_CSR_NAMES csrNames[] =
	{
		{"BcBankEnable", BcBankEnable},
		{"BcBurstModeEnable", BcBurstModeEnable},
		{"BcCleanVictim", BcCleanVictim},
		{"BcClkfwdEnable", BcClkfwdEnable},
		{"BcClockOut", BcClockOut},
		{"BcDdmFallEn", BcDdmFallEn},
		{"BcDdmfEnable", BcDdmfEnable},
		{"BcDdmrEnable", BcDdmrEnable},
		{"BcDdmRiseEn", BcDdmRiseEn},
		{"BcEnable", BcEnable},
		{"BcFrmClk", BcFrmClk},
		{"BcLateWriteUpper", BcLateWriteUpper},
		{"BcPentiumMode", BcPentiumMode},
		{"BcRdRdBubble", BcRdRdBubble},
		{"BcRdvictim", BcRdvictim},
		{"BcSjBankEnable", BcSjBankEnable},
		{"BcTagDdmFallEn", BcTagDdmFallEn},
		{"BcTagDdmRiseEn", BcTagDdmRiseEn},
		{"BcWrWrBubble", BcWrWrBubble},
		{"ThirtyTwoByteIo", ThirtyTwoByteIo},
		{"DupTagEnable", DupTagEnable},
		{"EnableEvict", EnableEvict},
		{"EnableProbeCheck", EnableProbeCheck},
		{"EnableStcCommand", EnableStcCommand},
		{"FastModeDisable", FastModeDisable},
		{"InitMode", InitMode},
		{"JitterCmd", JitterCmd},
		{"MboxBcPrbStall", MboxBcPrbStall},
		{"PrbTagOnly", PrbTagOnly},
		{"RdvicAckInhibit", RdvicAckInhibit},
		{"SkewedFillMode", SkewedFillMode},
		{"SpecReadEnable", SpecReadEnable},
		{"StcEnable", StcEnable},
		{"SysbusFormat", SysbusFormat},
		{"SysbusMbEnable", SysbusMbEnable},
		{"SysClkfwdEnable", SysClkfwdEnable},
		{"SysDdmFallEn", SysDdmFallEn},
		{"SysDdmfEnable", SysDdmfEnable},
		{"SysDdmrEnable", SysDdmrEnable},
		{"SysDdmRdFallEn", SysDdmRdFallEn},
		{"SysDdmRdRiseEn", SysDdmRdRiseEn},
		{"SysDdmRiseEn", SysDdmRiseEn},
		{"BcClkDelay", BcClkDelay},
		{"BcCpuClkDelay", BcCpuClkDelay},
		{"BcCpuLateWriteNum", BcCpuLateWriteNum},
		{"BcRcvMuxCntPreset", BcRcvMuxCntPreset},
		{"CfrFrmclkDelay", CfrFrmclkDelay},
		{"DataValidDly", DataValidDly},
		{"InvalToDirty", InvalToDirty1},
		{"InvalToDirtyEnable", InvalToDirtyEnable},
		{"SysBusSize", SysBusSize},
		{"SysClkDelay", SysClkDelay},
		{"SysCpuClkDelay", SysCpuClkDelay},
		{"SysRcvMuxCntPreset", SysRcvMuxCntPreset},
		{"SysRcvMuxPreset", SysRcvMuxPreset},
		{"BcLateWriteNum", BcLateWriteNum},
		{"CfrEv6clkDelay", CfrEv6clkDelay},
		{"SetDirtyEnable", SetDirtyEnable},
		{"SysbusVicLimit", SysbusVicLimit},
		{"BcBphaseLdVector", BcBphaseLdVector},
		{"BcSize", BcSize},
		{"BcWrRdBubbles", BcWrRdBubbles},
		{"BcWrtSts", BcWrtSts},
		{"CfrGclkDelay", CfrGclkDelay},
		{"MbCnt", MbCnt},
		{"SysBphaseLdVector", SysBphaseLdVector},
		{"SysdcDelay", SysdcDelay},
		{"SysbusAckLimit", SysbusAckLimit},
		{"SysClkRatio", SysClkRatio},
		{"SysFrameLdVector", SysFrameLdVector},
		{"BcRdWrBubbles", BcRdWrBubbles},
		{"BcLatTagPattern", BcLatTagPattern},
		{"BcFdbkEn", BcFdbkEn},
		{"DcvicThreshold", DcvicThreshold},
		{"SysFdbkEn", SysFdbkEn},
		{"BcClkLdVector", BcClkLdVector},
		{"SysClkLdVector", SysClkLdVector},
		{"BcLatDataPattern", BcLatDataPattern},
		{NULL, LastCSR}
	};

/*
 * AXP_DuplicateDcacheTagArray
 *	This function is responsible for managing the duplicate Dcache Tag (DTAG)
 *	array and is used by the Cbox when processing Dcache fills, Icache fills
 *	and system port probes.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_DuplicateDcacheTagArray()
{
	return;
}

/*
 * AXP_IQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Integer Issue Queue (IQ).  Architecturally, this is defined in the Cbox
 *	as the "Arbiter".  According to the architecture, there are two(2) arbiters
 *	for the IQ, one for the upper subclusters and one for the lower.  Each will
 *	select two instructions from the 20 possible queued integer instructions.
 *	Priority is given to older requests over newer ones.  If one instruction
 *	requests both lower subclusters and there is no other requesting a lower
 *	subcluster, then L0 is selected.  The same, but for the upper-clusters will
 *	select U1.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IQArbiter()
{
	return;
}

/*
 * AXP_FQArbiter
 *	This function is responsible for the arbitration of instructions pending in
 *	the Floating-point Issue Queue (FQ).  Architecturally, this is defined in
 *	the Cbox as the "Arbiter".  According to the architecture, there are
 *	three(3) arbiters for the FQ, one for each add, multiply, or store
 *	pipelines.  The add and multiply arbiters will pick one request, while the
 *	store arbiter will pick two (one for each store pipeline).  Priority is
 *	given to older requests over newer ones.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_FQArbiter()
{
	return;
}

/*
 * AXP_21264_Cbox_Config
 * 	This function is called to configure the Cbox from an initialization file
 * 	(aka: SROM), which contains the settings for the Cbox CSRs.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	Cbox CSRs will be set with SROM initialization.
 *
 * Return Value:
 * 	true:	Failed to perform all initialization functions.
 * 	false:	Succeeded.
 */
bool AXP_21264_Cbox_Config(AXP_21264_CPU *cpu)
{
	bool					retVal = false;
	bool					readResult = true;
	FILE					*fp;
	char					*configFile = "../dat/AXP_21264_Cbox_CSR.nvp";
	char					name[32];
	u32						value;
	int						ii;
	AXP_21264_CBOX_CSR_VAL	csr;

	/*
	 * Open the file to configure the CSRs for the Cbox.
	 */
	fp = AXP_Open_NVP_File(configFile);
	if (fp != NULL)
	{

		/*
		 * While the read from the NVP file returns a name/value pair,
		 * continue to process the file.
		 */
		while (readResult == true)
		{
			readResult = AXP_Read_NVP_File(fp, name, &value);

			/*
			 * If the above call failed, then set the retVal and get out of
			 * the loop.
			 */
			if (readResult == false)
			{
				retVal = true;
				continue;
			}

			/*
			 * OK, if we get this far we have a name, in string format and a
			 * 32-bit value.  We need to convert the string name to an
			 * equivalent name value, so that we can use select to determine
			 * which of the CSRs is being initialized with the value.
			 */
			csr = LastCSR;
			for (ii = 0; ((csrNames[ii].name != NULL) && (csr != LastCSR)); ii++)
			{
				if (strcmp(csrNames[ii].name, name) == 0)
					csr = csrNames[ii].values;
			}

			/*
			 * Initialize the correct CSR.  If the above loop did not find
			 * anything that matched, then csr will equal LastCSR, which will
			 * be processed by the same code that would trip to the default
			 * (both of which are errors).
			 */
			switch (csr)
			{
				case BcBankEnable:
					cpu->csr.BcBankEnable = value;
					break;

				case BcBurstModeEnable:
					cpu->csr.BcBurstModeEnable = value;
					break;

				case BcCleanVictim:
					cpu->csr.BcCleanVictim = value;
					break;

				case BcClkfwdEnable:
					cpu->csr.BcClkfwdEnable = value;
					break;

				case BcClockOut:
					cpu->csr.BcClockOut = value;
					break;

				case BcDdmFallEn:
					cpu->csr.BcDdmFallEn = value;
					break;

				case BcDdmfEnable:
					cpu->csr.BcDdmfEnable = value;
					break;

				case BcDdmrEnable:
					cpu->csr.BcDdmrEnable = value;
					break;

				case BcDdmRiseEn:
					cpu->csr.BcDdmRiseEn = value;
					break;

				case BcEnable:
					cpu->csr.BcEnable = value;
					break;

				case BcFrmClk:
					cpu->csr.BcFrmClk = value;
					break;

				case BcLateWriteUpper:
					cpu->csr.BcLateWriteUpper = value;
					break;

				case BcPentiumMode:
					cpu->csr.BcPentiumMode = value;
					break;

				case BcRdRdBubble:
					cpu->csr.BcRdRdBubble = value;
					break;

				case BcRdvictim:
					cpu->csr.BcRdvictim = value;
					break;

				case BcSjBankEnable:
					cpu->csr.BcSjBankEnable = value;
					break;

				case BcTagDdmFallEn:
					cpu->csr.BcTagDdmFallEn = value;
					break;

				case BcTagDdmRiseEn:
					cpu->csr.BcTagDdmRiseEn = value;
					break;

				case BcWrWrBubble:
					cpu->csr.BcWrWrBubble = value;
					break;

				case ThirtyTwoByteIo:
					cpu->csr.ThirtyTwoByteIo = value;
					break;

				case DupTagEnable:
					cpu->csr.DupTagEnable = value;
					break;

				case EnableEvict:
					cpu->csr.EnableEvict = value;
					break;

				case EnableProbeCheck:
					cpu->csr.EnableProbeCheck = value;
					break;

				case EnableStcCommand:
					cpu->csr.EnableStcCommand = value;
					break;

				case FastModeDisable:
					cpu->csr.FastModeDisable = value;
					break;

				case InitMode:
					cpu->csr.InitMode = value;
					break;

				case JitterCmd:
					cpu->csr.JitterCmd = value;
					break;

				case MboxBcPrbStall:
					cpu->csr.MboxBcPrbStall = value;
					break;

				case PrbTagOnly:
					cpu->csr.PrbTagOnly = value;
					break;

				case RdvicAckInhibit:
					cpu->csr.RdvicAckInhibit = value;
					break;

				case SkewedFillMode:
					cpu->csr.SkewedFillMode = value;
					break;

				case SpecReadEnable:
					cpu->csr.SpecReadEnable = value;
					break;

				case StcEnable:
					cpu->csr.StcEnable = value;
					break;

				case SysbusFormat:
					cpu->csr.SysbusFormat = value;
					break;

				case SysbusMbEnable:
					cpu->csr.SysbusMbEnable = value;
					break;

				case SysClkfwdEnable:
					cpu->csr.SysClkfwdEnable = value;
					break;

				case SysDdmFallEn:
					cpu->csr.SysDdmFallEn = value;
					break;

				case SysDdmfEnable:
					cpu->csr.SysDdmfEnable = value;
					break;

				case SysDdmrEnable:
					cpu->csr.SysDdmrEnable = value;
					break;

				case SysDdmRdFallEn:
					cpu->csr.SysDdmRdFallEn = value;
					break;

				case SysDdmRdRiseEn:
					cpu->csr.SysDdmRdRiseEn = value;
					break;

				case SysDdmRiseEn:
					cpu->csr.SysDdmRiseEn = value;
					break;

				case BcClkDelay:
					cpu->csr.BcClkDelay = value;
					break;

				case BcCpuClkDelay:
					cpu->csr.BcCpuClkDelay = value;
					break;

				case BcCpuLateWriteNum:
					cpu->csr.BcCpuLateWriteNum = value;
					break;

				case BcRcvMuxCntPreset:
					cpu->csr.BcRcvMuxCntPreset = value;
					break;

				case CfrFrmclkDelay:
					cpu->csr.CfrFrmclkDelay = value;
					break;

				case DataValidDly:
					cpu->csr.DataValidDly = value;
					break;

				case InvalToDirty1:
					cpu->csr.InvalToDirty = value;
					break;

				case InvalToDirtyEnable:
					cpu->csr.InvalToDirtyEnable = value;
					break;

				case SysBusSize:
					cpu->csr.SysBusSize = value;
					break;

				case SysClkDelay:
					cpu->csr.SysClkDelay = value;
					break;

				case SysCpuClkDelay:
					cpu->csr.SysCpuClkDelay = value;
					break;

				case SysRcvMuxCntPreset:
					cpu->csr.SysRcvMuxCntPreset = value;
					break;

				case SysRcvMuxPreset:
					cpu->csr.SysRcvMuxPreset = value;
					break;

				case BcLateWriteNum:
					cpu->csr.BcLateWriteNum = value;
					break;

				case CfrEv6clkDelay:
					cpu->csr.CfrEv6clkDelay = value;
					break;

				case SetDirtyEnable:
					cpu->csr.SetDirtyEnable = value;
					break;

				case SysbusVicLimit:
					cpu->csr.SysbusVicLimit = value;
					break;

				case BcBphaseLdVector:
					cpu->csr.BcBphaseLdVector = value;
					break;

				case BcSize:
					int bCacheArraySize;

					cpu->csr.BcSize = value;

					/*
					 * Now that we know the Bcache size, go an allocate a
					 * buffer large enough for it.  First, deallocate anything
					 * that was previously allocated.
					 */
					if (cpu->bCache != NULL)
						free(cpu->bCache);
					if (cpu->bTag != NULL)
						free(cpu->bTag);

					/*
					 * OK, now allocate a Bcache large enough for the size.
					 * NOTE: Each Bcache block contains 64 bytes, so the array
					 * size is the Bcache size divided by 64.
					 */
					bCacheArraySize = ((cpu->csr.BcSize + 1) * ONE_M) /
							AXP_BCACHE_BLOCK_SIZE;

					/*
					 * Go and actually allocate the 2 arrays needed for the
					 * Bcache (the cache array and the tag array).
					 */
					cpu->bCache = calloc(
									bCacheArraySize,
									sizeof(AXP_21264_BCACHE_BLK));
					cpu->bTag = calloc(
									bCacheArraySize,
									sizeof(AXP_21264_BCACHE_TAG));

					/*
					 * If we failed to allocate either, then we are done
					 * here.  Returning true will cause the caller to
					 * exit.
					 */
					if ((cpu->bCache == NULL) || (cpu->bTag == NULL))
					{
						retVal = true;
						readResult = false;
					}
					break;

				case BcWrRdBubbles:
					cpu->csr.BcWrRdBubbles = value;
					break;

				case BcWrtSts:
					cpu->csr.BcWrtSts = value;
					break;

				case CfrGclkDelay:
					cpu->csr.CfrGclkDelay = value;
					break;

				case MbCnt:
					cpu->csr.MbCnt = value;
					break;

				case SysBphaseLdVector:
					cpu->csr.SysBphaseLdVector = value;
					break;

				case SysdcDelay:
					cpu->csr.SysdcDelay = value;
					break;

				case SysbusAckLimit:
					cpu->csr.SysbusAckLimit = value;
					cpu->cmdAck = 0;
					break;

				case SysClkRatio:
					cpu->csr.SysClkRatio = value;
					break;

				case SysFrameLdVector:
					cpu->csr.SysFrameLdVector = value;
					break;

				case BcRdWrBubbles:
					cpu->csr.BcRdWrBubbles = value;
					break;

				case BcLatTagPattern:
					cpu->csr.BcLatTagPattern = value;
					break;

				case BcFdbkEn:
					cpu->csr.BcFdbkEn = value;
					break;

				case DcvicThreshold:
					cpu->csr.DcvicThreshold = value;
					break;

				case SysFdbkEn:
					cpu->csr.SysFdbkEn = value;
					break;

				case BcClkLdVector:
					cpu->csr.BcClkLdVector = value;
					break;

				case SysClkLdVector:
					cpu->csr.SysClkLdVector = value;
					break;

				case BcLatDataPattern:
					cpu->csr.BcLatDataPattern = value;
					break;

				default:
					printf("Unexpected name/value pair: 'name' returned as \'%s\' at %s, line %d.\n",
 						name, __FILE__, __LINE__); 
					retVal = true;
					readResult = false;
					break;
			}
		}

		/*
		 * We successfully opened the file, now let's make sure we close it.
		 */
		AXP_Close_NVP_File(fp);
	}

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Bcache_Flush
 *	This function is called to Flush everything from the Bcache.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_21264_Bcache_Flush(AXP_21264_CPU *cpu)
{
	int		ii;
	int		bCacheArraySize;

	/*
	 * First we need to determine the size of the Bcache.
	 */
	switch (cpu->csr.BcSize)
	{
		case AXP_BCACHE_1MB:
			bCacheArraySize = AXP_21264_1MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_2MB:
			bCacheArraySize = AXP_21264_2MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_4MB:
			bCacheArraySize = AXP_21264_4MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_8MB:
			bCacheArraySize = AXP_21264_8MB / AXP_BCACHE_BLOCK_SIZE;
			break;

		case AXP_BCACHE_16MB:
			bCacheArraySize = AXP_21264_16MB / AXP_BCACHE_BLOCK_SIZE;
			break;
	}

	/*
	 * NOTE: We only have to mark the array entry invalid in the Bcache Tag
	 * array.  What is in the Bcache Block array is only relevant when the Tag
	 * array indicates that it is valid.  When the valid flag is set, the data
	 * was just written to the block array.
	 */
	for (ii = 0; ii < bCacheArraySize; ii++)
		cpu->bTag[ii].valid = false;

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Bcache_Valid
 *	This function is called to determine if a physical address has a valid
 *	location within the Bcache.  We don't actually look in the Bcache, but do
 *	look in the Bcache Tag array.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	false:	The physical address is not in the Bcache.
 *	true:	The physical address is in the Bcache.
 */
bool AXP_21264_Bcache_Valid(AXP_21264_CPU *cpu, u64 pa)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);

	/*
	 * If the entry at the index, based on the physical address, is valid and
	 * the tag associated with that entry matches the tag out of the physical
	 * address, then we have a valid entry.  Otherwise, we do not.
	 */
	return((cpu->bTag[index].valid == true) &&
		   (cpu->bTag[index].tag == AXP_BCACHE_TAG(cpu, pa)));
}

/*
 * AXP_21264_Bcache_Status
 * 	This function is called to return the status of a Bcache entry (if valid).
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Values:
 *	An unsigned 32-bit location to receive a masked value with the following
 *	bits set as appropriate:
 *			AXP_21264_CACHE_MISS
 *			AXP_21264_CACHE_HIT
 *			AXP_21264_CACHE_DIRTY
 *			AXP_21264_CACHE_SHARED
 *
 * NOTE:	This is called from the Mbox and Cbox, which already locked bCache
 * 			mutex.
 */
u32 AXP_21264_Bcache_Status(AXP_21264_CPU *cpu, u64 pa)
{
	u8		retVal = AXP_21264_CACHE_MISS;
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If there is no valid record, then this is a MISS.  Nothing else we need
	 * to do.
	 */
	if (valid == true)
	{
		u32 index = AXP_BCACHE_INDEX(cpu, pa);

		/*
		 * Set the return value based on the fact that we hit in the Bcache and
		 * the status bits associated with that entry.
		 */
		retVal = AXP_21264_CACHE_HIT;
		if (cpu->bTag[index].dirty == true)
			retVal |= AXP_21264_CACHE_DIRTY;
		if (cpu->bTag[index].shared == true)
			retVal |= AXP_21264_CACHE_SHARED;
	}
	return(retVal);
}

/*
 * AXP_21264_Bcache_Read
 *	This function is called to read the contents of a Bcache location and
 *	return them to the caller.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *
 * Output Parameters:
 *	data:
 *		A pointer to a properly size buffer (64 bytes) to receive the contents
 *		of the Bcache, if found (valid).
 *
 * Return Values:
 *	false:	The physical address is not in the Bcache.
 *	true:	The physical address is in the Bcache.
 */
bool AXP_21264_Bcache_Read(
				AXP_21264_CPU *cpu,
				u64 pa,
				u8 *data,
				bool *dirty,
				bool *shared)
{
	bool	retVal = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * If the physical address is in the Bcache, then we can copy the data to
	 * the caller's buffer.
	 */
	if (retVal == true)
	{
		u32 index = AXP_BCACHE_INDEX(cpu, pa);

		/*
		 * Copy the data.
		 */
		memcpy(
			data,
			cpu->bCache[index],
			AXP_BCACHE_BLOCK_SIZE);

		/*
		 * If requested, return the dirty and shared bits.
		 */
		if (dirty != NULL)
			*dirty = cpu->bTag[index].dirty;
		if (shared != NULL)
			*shared = cpu->bTag[index].shared;
	}

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_21264_Bcache_Write
 *	This function is called to write the contents of a buffer into a Bcache
 *	location.  This function always succeeds.  If a location is already in use
 *	and we are updating it, then we will also write the values to memory.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the CPU structure where the Bcache is stored.
 *	pa:
 *		A value containing the physical address to be checked for validity.
 *	data:
 *		A pointer to a properly size buffer (64 bytes) containing the data to
 *		be written to the Bcache.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Bcache_Write(AXP_21264_CPU *cpu, u64 pa, u8 *data)
{
	int		index = AXP_BCACHE_INDEX(cpu, pa);
	bool	valid = AXP_21264_Bcache_Valid(cpu, pa);

	/*
	 * First copy the buffer into the Bcache, then update the associated tag
	 * with the tag value and setting the valid bit.
	 *
	 * TODO: What do we do with the shared bit.
	 */
	memcpy(cpu->bCache[index], data, AXP_BCACHE_BLOCK_SIZE);
	cpu->bTag[index].tag = AXP_BCACHE_TAG(cpu, pa);
	cpu->bTag[index].valid = true;

	/*
	 * If the buffer was already valid, then we have to update memory as well.
	 */
	if (valid == true)
	{
		/* TODO: Write the updated buffer out to memory. */
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

void AXP_21264_OldestPQFlags(
				AXP_21264_CPU *cpu,
				bool *m1,
				bool *m2,
				bool *ch)
{
	int						oldestPQ = -1;
	int						ii;
	int 					start1 = -1, end1 = -1, start2 = -1, end2 = -1;

	*m1 = false;
	*m2 = false;
	*ch = false;

	/*
	 * HRM Table 4-12, 21264-to-System Command Fields Definitions
	 *
	 * SysAddOut	Definition
	 * ---------	-----------------------------------------------------------
	 * M1			When set, reports a miss to the system for the oldest
	 * 				probe.  When clear, has no meaning.
	 * M2			When set, reports that the oldest probe has missed in
	 * 				cache. Also, this bit is set for system-to-21264 probe
	 * 				commands that hit but have no data movement (see the CH
	 * 				bit, below).  When clear, has no meaning.
	 * 				M1 and M2 are not asserted simultaneously. Reporting probe
	 * 				results as soon as possible is critical to high-speed
	 * 				operation, so when a result is known the 21264 uses the
	 * 				earliest opportunity to send an M signal to the system.
	 * 				M bit assertion can occur either in a valid command or a
	 * 				NZNOP.
	 * CH			The cache hit bit is asserted, along with M2, when probes
	 * 				with no data movement hit in the Dcache or Bcache. This
	 * 				response can be generated by a probe that explicitly
	 * 				indicates no data movement or a ReadIfDirty command that
	 * 				hits on a valid but clean or shared block.
	 */
	if (cpu->pqTop > cpu->pqBottom)
	{
		start1 = cpu->pqTop;
		end1 = AXP_21264_PQ_LEN;
		start2 = 0;
		end2 = cpu->pqBottom;
	}
	else
	{
		start1 = cpu->pqTop;
		end1 = cpu->pqBottom;
	}

	/*
	 * Search through the list to find the oldest entry that can be processed.
	 */
	for (ii = start1; ((ii < end1) && (oldestPQ == -1)); ii++)
		if ((cpu->pq[ii].valid == true) && (cpu->pq[ii].processed == false))
			oldestPQ = ii;
	if ((oldestPQ == -1) && (start2 != -1))
		for (ii = start2; ((ii < end2) && (oldestPQ == -1)); ii++)
			if ((cpu->pq[ii].valid == true) && (cpu->pq[ii].processed == false))
				oldestPQ = ii;

	/*
	 * If we found a pending probe, the first one found is by definition the
	 * oldest one.
	 */
	if (oldestPQ >= 0)
	{
		AXP_21264_CBOX_PQ	*pq = &cpu->pq[oldestPQ];
		AXP_VA				physAddr = {.va = pq->pa};
		AXP_EXCEPTIONS 		except;
		u32					status = AXP_21264_CACHE_MISS;
		u32					ctagIndex = physAddr.vaIdxInfo.index;
		u32					setToUse = 0;

		/*
		 * We use the CTAG to determine Dcache bits because the CTAG is a
		 * duplicate of the DTAG, except that it is physically indexed and
		 * tagged.  Lock the Cbox IPR mutex first.
		 */
		pthread_mutex_lock(&cpu->cBoxIPRMutex);

		/*
		 * If set 0 at the CTAG index is not valid or the tags don't match,
		 * then try set 1.
		 */
		if ((cpu->ctag[ctagIndex][setToUse].valid == false) ||
			(cpu->ctag[ctagIndex][setToUse].physTag != physAddr.vaIdxInfo.tag))
			setToUse++;

		/*
		 * If set 1 at the CTAG index is not valid or the tags don't match,
		 * then set the set to use to 2 (one past the highest valid value).
		 */
		if ((cpu->ctag[ctagIndex][setToUse].valid ==  false) ||
			(cpu->ctag[ctagIndex][setToUse].physTag != physAddr.vaIdxInfo.tag))
			setToUse++;

		/*
		 * If we found something, check the bits out for the Dcache line and
		 * set the status bits as appropriate.
		 */
		if (setToUse < AXP_2_WAY_CACHE)
		{
			status = AXP_21264_CACHE_HIT;
			if (cpu->ctag[ctagIndex][setToUse].dirty == true)
				status |= AXP_21264_CACHE_DIRTY;
			if (cpu->ctag[ctagIndex][setToUse].shared == true)
				status |= AXP_21264_CACHE_SHARED;
		}

		/*
		 * Unlock the Cbox IPR mutex.  We are all done with the CTAG.
		 */
		pthread_mutex_unlock(&cpu->cBoxIPRMutex);

		/*
		 * If the Dcache got us a MISS, then check the Bcache (which is a
		 * superset of the combination of Dcache and Icache).  We need to check
		 * both, even though the Bcache is supposed to be a superset, because
		 * there may have been an update to the Dcache that has not made it to
		 * the Bcache.
		 */
		if (status == AXP_21264_CACHE_MISS)
			status = AXP_21264_Bcache_Status(cpu, pq->pa);

		/*
		 * If we still have a MISS, then set the M1 bit.
		 */
		if (status == AXP_21264_CACHE_MISS)
		{
			*m1 = true;
		}

		/*
		 * If we have a HIT, but the probe was not looking to move data, then
		 * set set the CH bit, and conditionally the M2 bit.
		 */
		else if (pq->notJustSysDc == false)
		{
			*ch = true;

			/*
			 * If the probe was for a ReadIfDirty, and the cache entry is either
			 * clean (not dirty) or shared, then set the M2 bit.
			 */
			if ((AXP_21264_GET_PROBE_DM(pq->probe) == AXP_21264_DM_RDDIRTY) &&
				 (((status & AXP_21264_CACHE_SHARED) == AXP_21264_CACHE_SHARED) ||
				   ((status & AXP_21264_CACHE_DIRTY) != AXP_21264_CACHE_DIRTY)))
				*m2 = true;
		}
		if ((m1 == true) || (m2 == true) || (ch == true))
			pq->processed = true;
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_MAF_Empty
 *	This function is called to determine of there is a record in the Missed
 *	Address File (MAF) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_MAF_Empty(AXP_21264_CPU *cpu)
{
	int		retVal = -1;
	int		ii;
	int		start1 = -1, start2 = -1, end1 = -1, end2 = -1;

	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 */
	for (ii = start1; ((ii < end1) && (retVal == -1)); ii++)
		if ((cpu->maf[ii].type != MAFNotInUse) && (cpu->maf[ii].complete == false))
			retVal = ii;
	if ((retVal == -1) && (start2 != -1))
		for (ii = start2; ((ii < end2) && (retVal == -1)); ii++)
			if ((cpu->maf[ii].type != MAFNotInUse) && (cpu->maf[ii].complete == false))
				retVal = ii;

	/*
	 * Return what we found, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Process_MAF
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the MAF records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the MAF to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_MAF(AXP_21264_CPU *cpu, int entry)
{
	AXP_21264_CBOX_MAF		*maf = &cpu->maf[entry];
	AXP_21264_TO_SYS_CMD	cmd;
	u16						mask = AXP_21264_IO_INV;
	bool					m1 = false;
	bool					m2 = false;
	bool					rv = true;
	bool					ch = false;

	/*
	 * Process the next MAF entry that needs it.
	 *
	 * TODO: Need to look at Speculative Transactions.
	 */
	switch(maf->type)
	{
		case LDx:
			cmd = ReadBlk;
			break;

		case STx:
		case STx_C:
			cmd = ReadBlkMod;
			break;

		case STxChangeToDirty:
			cmd = STxChangeToDirty;
			break;

		case STxCChangeToDirty:
			cmd = STxCChangeToDirty;
			break;

		case WH64:
			cmd = InvalToDirty;
			break;

		case ECB:
			cmd = Evict;
			break;

		case Istream:
			cmd = ReadBlkI;
			break;
	}

	/*
	 * Go check the Oldest pending PQ and set the flags for it here and now.
	 */
	AXP_21264_OldestPQFlags(cpu, &m1, &m2, &ch);

	/*
	 * OK, send what we have to the System.
	 */
	AXP_System_CommandSend(cmd, m2, entry, rv, mask, ch, maf->pa, NULL, 0);
	
	/*
	 * Indicate that the entry is now processed and return back to the caller.
	 */
	maf->complete = true;
	return;
}

/*
 * AXP_21264_Add_MAF
 *	This function is called to add an Miss Address File (MAF) entry on to the
 *	queue for processing.
 *
 *	NOTE:	The Ibox and Mbox call this function. They do this to add a fill
 *			request to the for their associated caches.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	type:
 *		An enumerated value for the type of MAF entry to add.
 *	pa:
 *		A value representing the physical address associated with this record.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Add_MAF(
		AXP_21264_CPU *cpu,
		AXP_CBOX_MAF_TYPE type,
		u64 pa,
		i8 lqSqEntry,
		int dataLen,
		bool shared)
{
	int		ii;
	int		start1 = -1, start2 = -1, end1 = -1, end2 = -1;
	int		matchingMAF = -1;

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * HRM 2.9
	 * Because all memory transactions are to 64-byte blocks, efficiency is
	 * improved by merging several small data transactions into a single larger
	 * data transaction.
	 * Table 2–9 lists the rules the 21264 uses when merging memory
	 * transactions into 64-byte naturally aligned data block transactions.
	 * Rows represent the merged instruction in the MAF and columns represent
	 * the new issued transaction.
	 *
	 * Table 2–9 MAF Merging Rules
	 * MAF/New	LDx		STx		STx_C	WH64	ECB		Istream
	 * -------	-----	-----	-----	-----	-----	--------
	 * LDx 		Merge	—		—		—		—		—
	 * STx		Merge	Merge	—		—		—		—
	 * STx_C	—		—		Merge	—		—		—
	 * WH64		—		—		—		Merge	—		—
	 * ECB		—		—		—		—		Merge	—
	 * Istream	—		—		—		—		—		Merge
	 *
	 * In summary, Table 2–9 shows that only like instruction types, with the
	 * exception of load instructions merging with store instructions, are
	 * merged.
	 */

	if (cpu->mafTop > cpu->mafBottom)
	{
		start1 = cpu->mafTop;
		end1 = AXP_21264_MAF_LEN;
		start2 = 0;
		end2 = cpu->mafBottom;
	}
	else
	{
		start1 = cpu->mafTop;
		end1 = cpu->mafBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 * We do this test in 3 stages:
	 *
	 *		1)	If the MAF is in-use and not completed
	 *		2)	If the MAF type matches the new type, or we have a store and
	 *			are doing a load
	 *		3)	If the 64-byte block of the physical address includes the all
	 *			the bytes for the data we are reading/writing.
	 */
	for (ii = start1; ((ii < end1) && (matchingMAF == -1)); ii++)
		if ((cpu->maf[ii].type != MAFNotInUse) &&
			(cpu->maf[ii].complete == false))
		{
			if ((cpu->maf[ii].type = type) ||
				((cpu->maf[ii].type == STx) && (type == LDx)))
			{
				if (cpu->maf[ii].pa ==
					((pa + dataLen - 1) & AXP_21264_ALIGN_MEM_BLK))
					matchingMAF = ii;
			}
		}
	if ((matchingMAF == -1) && (start2 != -1))
		for (ii = start2; ((ii < end2) && (matchingMAF == -1)); ii++)
			if ((cpu->maf[ii].type != MAFNotInUse) &&
				(cpu->maf[ii].complete == false))
			{
				if ((cpu->maf[ii].type = type) ||
					((cpu->maf[ii].type == STx) && (type == LDx)))
				{
					if (cpu->maf[ii].pa ==
						((pa + dataLen - 1) & AXP_21264_ALIGN_MEM_BLK))
						matchingMAF = ii;
				}
			}
	if (matchingMAF >= 0)
	{
		bool done = false;

		for (ii = 0; ((ii < AXP_21264_MBOX_MAX) && (done == false)); ii++)
		{
			if (cpu->maf[cpu->mafBottom].lqSqEntry[ii] == 0)
			{
				cpu->maf[cpu->mafBottom].lqSqEntry[ii] = lqSqEntry;
				done = true;
			}
		}
	}
	else
	{

		/*
		 * Add a record to the next available MAF.
		 */
		cpu->mafBottom = (cpu->mafBottom + 1) & 0x07;
		cpu->maf[cpu->mafBottom].type = type;
		cpu->maf[cpu->mafBottom].pa = pa & AXP_21264_ALIGN_MEM_BLK;
		cpu->maf[cpu->mafBottom].complete = false;
		cpu->maf[cpu->mafBottom].lqSqEntry[0] = lqSqEntry;
		for (ii = 1; ii < AXP_21264_MBOX_MAX; ii++)
			cpu->maf[cpu->mafBottom].lqSqEntry[ii] = 0;
	}

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	pthread_cond_signal(&cpu->cBoxInterfaceCond, &cpu->cBoxInterfaceMutex);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
	return;
}

/*
 * AXP_21264_VDB_Empty
 *	This function is called to determine of there is a record in the Victim
 *	Data Buffer (VDB) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_VDB_Empty(AXP_21264_CPU *cpu)
{
	int		retVal = -1;
	int		ii;
	int		start1 = -1, end1 = -1, start2 = -1, end2 = -1;

	if (cpu->vdbTop > cpu->vdbBottom)
	{
		start1 = cpu->vdbTop;
		end1 = AXP_21264_VDB_LEN;
		start2 = 0;
		end2 = cpu->vdbBottom;
	}
	else
	{
		start1 = cpu->vdbTop;
		end1 = cpu->vdbBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 */
	for (ii = start1; ((ii < end1) && (retVal == -1)); ii++)
		if ((cpu->vdb[ii].valid == true) && (cpu->vdb[ii].processed == false))
			retVal = ii;
	if ((retVal == -1) && (start2 != -1))
	for (ii = start2; ((ii < end2) && (retVal == -1)); ii++)
		if ((cpu->vdb[ii].valid == true) && (cpu->vdb[ii].processed == false))
			retVal = ii;

	/*
	 * Return what we found, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Process_VDB
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the VDB records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the VDB to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_VDB(AXP_21264_CPU *cpu, int entry)
{

	/*
	 * Process the next VDB entry that needs it.
	 */
	switch(cpu->vdb[entry].type)
	{

		/*
		 * Istream cache blocks from memory or Dcache blocks to be written to
		 * the Bcache.
		 */
		case toBcache:

			/*
			 * This does not go off chip, but was stored directly into the
			 * Bcache.
			 */
			break;

		/*
		 * We need to write a Bcache block out to memory.
		 */
		case toMemory:

			/*
			 * Send the Bcache block out to the system to store in memory.
			 */
#if 0
			AXP_SendToSystem(&cpu->vdb[entry].rq);
#endif
			break;

		/*
		 * Dcache or Bcache blocks send to the system in response to a
		 * probe command.
		 */
		case probeResponse:

			/*
			 * Send the ProbeResponse to the system having sent a Probe
			 * request.
			 */
#if 0
			AXP_SendToSystem(&cpu->vdb[entry].rsp);
#endif
			break;
	}

	/*
	 * Indicate that the entry is now processed and return back to the caller.
	 */
	cpu->vdb[entry].processed = true;
	return;
}

/*
 * AXP_21264_Add_VDB
 *	This function is called to add an Victim Data Buffer (VDB) entry on to the
 *	queue for processing.
 *
 *	NOTE:	The Mbox and Cbox call this function.  The Mbox to have a Dcache
 *			block to be written to the Bcache.  The Cbox to have Istream blocks
 *			recently written to the Icache to the Bcache as well
 *			(TODO: Determine if the Cbox can do both at the same time), Bcache
 *			blocks that need to be written to memory, or cache blocks sent to
 *			the system in response to probe commands.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	entry used.
 */
u8 AXP_21264_Add_VDB(
				AXP_21264_CPU *cpu,
				AXP_21264_VDB_TYPE type,
				u64 pa,
				u8 *buf,
				u8 bufLen,
				bool probe,
				bool alreadyLocked)
{
	bool	locked = false;

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	if (alreadyLocked == false)
	{
		pthread_mutex_lock(&cpu->cBoxInterfaceMutex);
		locked = true;
	}

	/*
	 * Add a record to the next available VDB.
	 */
	cpu->vdbBottom = (cpu->vdbBottom + 1) & 0x07;
	cpu->vdb[cpu->vdbBottom].type = type;
	cpu->vdb[cpu->vdbBottom].pa = pa;
	cpu->vdb[cpu->vdbBottom].validProbe = probe;
	memset(cpu->vdb[cpu->vdbBottom].data, '\0', QUAD_LEN);
	memcpy(cpu->vdb[cpu->vdbBottom].data, buf, bufLen);
	cpu->vdb[cpu->vdbBottom].dataLen = bufLen;
	cpu->vdb[cpu->vdbBottom].valid = true;
	cpu->vdb[cpu->vdbBottom].processed = false;

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	if (locked == true)
	{
		pthread_cond_signal(&cpu->cBoxInterfaceCond, &cpu->cBoxInterfaceMutex);
		pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
	}
	return(cpu->vdbBottom);
}

/*
 * AXP_21264_IOWB_Empty
 *	This function is called to determine of there is a record in the I/O
 *	Write Buffer (IOWB) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_IOWB_Empty(AXP_21264_CPU *cpu)
{
	int		retVal = -1;
	int		ii;
	int 	start1 = -1, end1 = -1, start2 = -1, end2 = -1;

	if (cpu->iowbTop > cpu->iowbBottom)
	{
		start1 = cpu->iowbTop;
		end1 = AXP_21264_IOWB_LEN;
		start2 = 0;
		end2 = cpu->iowbBottom;
	}
	else
	{
		start1 = cpu->iowbTop;
		end1 = cpu->iowbBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 */
	for (ii = start1; ((ii < end1) && (retVal == -1)); ii++)
		if ((cpu->iowb[ii].valid == true) && (cpu->iowb[ii].processed == false))
			retVal = ii;
	if ((retVal == -1) && (start2 != -1))
		for (ii = start2; ((ii < end2) && (retVal == -1)); ii++)
			if ((cpu->iowb[ii].valid == true) && (cpu->iowb[ii].processed == false))
				retVal = ii;

	/*
	 * Return what we found, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Process_IOWB
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the IOWB records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the IOWB to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_IOWB(AXP_21264_CPU *cpu, int entry)
{
	AXP_21264_CBOX_IOWB		*iowb = &cpu->iowb[entry];
	AXP_21264_TO_SYS_CMD	cmd;
	u16						mask = AXP_21264_IO_INV;
	bool					m1 = false;
	bool					m2 = false;
	bool					rv = true;
	bool					ch = false;

	/*
	 * Process the next IOWB entry that needs it.
	 */
	switch(iowb->storeLen)
	{
		case BYTE_LEN:
			mask = AXP_21264_IO_BYTE;
			cmd = WrBytes;
			break;

		case WORD_LEN:
			mask = AXP_21264_IO_WORD;
			cmd = WrBytes;
			break;

		case LONG_LEN:
			mask = AXP_21264_IO_LONG;
			cmd = WrLWs;
			break;

		case QUAD_LEN:
			mask = AXP_21264_IO_QUAD;
			cmd = WrQWs;
			break;
	}

	/*
	 * Go check the Oldest pending PQ and set the flags for it here and now.
	 */
	AXP_21264_OldestPQFlags(cpu, &m1, &m2, &ch);

	/*
	 * OK, send what we have to the System.
	 */
	AXP_System_CommandSend(
					cmd,
					m2,
					entry,
					rv,
					mask,
					ch,
					iowb->pa,
					iowb->data,
					iowb->dataLen);

	/*
	 * Indicate that the entry is now processed and return back to the caller.
	 */
	iowb->processed = true;
	return;
}

/*
 * AXP_21264_Merge_IOWB
 *	This function is called to attempt to merge an request for a new I/O Write
 *	Block (IOWB) with an existing entry on to the queue for processing.
 *
 *	NOTE:	The Mbox calls this function. It does so when it has a SQ entry
 *			needs to be written to an I/O device.
 *
 * Input Parameters:
 * 	iowb:
 * 		A pointer to the next queued IOWB to see if we can merge the next data
 * 		with it.
 *	pa:
 *		A value indicating the physical address of the device the I/O Buffer is
 *		to be written.
 *	data:
 *		A pointer to the buffer to be written to the device.
 *	dataLen:
 *		A value indicating the length of the 'data' parameter.
 *	maxLen:
 *		A value indicating the longest merged buffer that is allowed (32 or
 *		64).
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	true:	A new IOWB still needs to be allocated.
 *	false:	The data was merged into an existing IOWB.
 */
bool AXP_21264_Merge_IOWB(
				AXP_21264_CBOX_IOWB *iowb,
				u64 pa,
				i8 lqSqEntry,
				u8 *data,
				int dataLen,
				int maxLen)
{
	bool	retVal = true;

	/*
	 * If the IOEB is valid, is being used for a length the same as
	 * the current one, is for an ascending, consecutive address, has
	 * not yet been processed, and the block is for aligned values.
	 */
	if ((iowb->valid == true) && (iowb->storeLen == dataLen) &&
		((iowb->pa + iowb->dataLen) == pa) &&
		(iowb->processed == false) && (iowb->aligned == true))
	{

		/*
		 * If the merge register is not full, then copy this next block
		 * into it and update the length.  Also, indicate that an IOWB
		 * does not need to be allocated.
		 */
		if (iowb->dataLen < maxLen)
		{
			if (dataLen == LONG_LEN)
				*((u32 *) &iowb->data[iowb->dataLen]) = *((u32 *) data);
			else
				*((u64 *) &iowb->data[iowb->dataLen]) = *((u64 *) data);
			iowb->lqSqEntry[iowb->dataLen/dataLen] = lqSqEntry;
			iowb->dataLen += dataLen;
			retVal = false;
		}
	}

	/*
	 * Let the caller know what just happened, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Add_IOWB
 *	This function is called to add an I/O Write Block (IOWB) entry on to the
 *	queue for processing.
 *
 *	NOTE:	The Mbox calls this function. It does so when it has a SQ entry
 *			needs to be written to an I/O device.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	pa:
 *		A value indicating the physical address of the device the I/O Buffer is
 *		to be written.
 *	data:
 *		A pointer to the buffer to be written to the device.
 *	dataLen:
 *		A value indicating the length of the 'data' parameter.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Add_IOWB(
				AXP_21264_CPU *cpu,
				u64 pa,
				i8 lqSqEntry,
				u8 *data,
				int dataLen)
{
	u32		ii;
	bool	allocateIOWB = true;

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * HRM Table 2–8 Rules for I/O Address Space Store Instruction Data Merging
	 *
	 * Merge Register/		Store
	 * Replayed Instruction	Byte/Word	Store Longword			Store Quadword
	 * --------------------	---------	--------------------	--------------------
	 * Byte/Word 			No merge	No merge				No merge
	 * Longword				No merge	Merge up to 32 bytes	No merge
	 * Quadword				No merge	No merge				Merge up to 64 bytes
	 * --------------------	---------	--------------------	--------------------
	 * Table 2–8 shows some of the following rules:
	 * 	- Byte/word store instructions and different size store instructions
	 * 	  are not allowed to merge.
	 *	- A stream of ascending non-overlapping, but not necessarily
	 *	  consecutive, longword store instructions are allowed to merge into
	 *	  naturally aligned 32-byte blocks.
	 *	- A stream of ascending non-overlapping, but not necessarily
	 *	  consecutive, quadword store instructions are allowed to merge into
	 *	  naturally aligned 64-byte blocks.
	 *	- Merging of quadwords can be limited to naturally-aligned 32-byte
	 *	  blocks based on the Cbox WRITE_ONCE chain 32_BYTE_IO field.
	 *	- Issued MB, WMB, and I/O load instructions close the I/O register
	 *	  merge window.  To minimize latency, the merge window is also closed
	 *	  when a timer detects no I/O store instruction activity for 1024
	 *	  cycles.
	 */
	if ((dataLen != BYTE_LEN) || (dataLen != WORD_LEN))	/* Don't merge Bytes/Words */
	{
		int		maxLen;
		int 	start1 = -1, end1 = -1, start2 = -1, end2 = -1;

		if (cpu->iowbTop > cpu->iowbBottom)
		{
			start1 = cpu->iowbTop;
			end1 = AXP_21264_IOWB_LEN;
			start2 = 0;
			end2 = cpu->iowbBottom;
		}
		else
		{
			start1 = cpu->iowbTop;
			end1 = cpu->iowbBottom;
		}

		/*
		 * We either have a longword or a quadword.  Longwords can be merged
		 * upto 32-bytes long.  Quadwords can either be merged upto 64-bytes or
		 * 32-bytes, depending upon the setting of the 32_BYTE_IO field in the
		 * Cbox CSR.
		 */
		if  (((dataLen == QUAD_LEN) && (cpu->csr.ThirtyTwoByteIo == 1)) ||
			 (dataLen == LONG_LEN))
			maxLen = AXP_21264_SIZE_LONG;
		else
			maxLen = AXP_21264_SIZE_QUAD;

		/*
		 * Search through each of the allocated IOWBs and see if they are
		 * candidates for merging.
		 */
		for (ii = start1; ((ii < end1) && (allocateIOWB == true)); ii++)
			allocateIOWB = AXP_21264_Merge_IOWB(
									&cpu->iowb[ii],
									pa,
									lqSqEntry,
									data,
									dataLen,
									maxLen);
		if ((allocateIOWB == false) && (start2 != -1))
			for (ii = start2; ((ii < end2) && (allocateIOWB == false)); ii++)
				allocateIOWB = AXP_21264_Merge_IOWB(
										&cpu->iowb[ii],
										pa,
										lqSqEntry,
										data,
										dataLen,
										maxLen);
	}

	/*
	 * If we didn't perform a merge, then we need to add a record to the next
	 * available IOWB.
	 */
	if (allocateIOWB == true)
	{
		cpu->iowbBottom = (cpu->iowbBottom + 1) & 0x03;
		cpu->iowb[cpu->iowbBottom].pa = pa;
		cpu->iowb[cpu->iowbBottom].lqSqEntry[0] = lqSqEntry;
		for (ii = 1; ii < AXP_21264_MBOX_MAX; ii++)
			cpu->iowb[cpu->iowbBottom].lqSqEntry[ii] = 0;
		cpu->iowb[cpu->iowbBottom].storeLen = dataLen;
		if (data != NULL)	/* this is a store */
			memcpy(cpu->iowb[cpu->iowbBottom].data, data, dataLen);
		else
			memset(
				cpu->iowb[cpu->iowbBottom].data,
				0,
				sizeof(cpu->iowb[cpu->iowbBottom].data));
		cpu->iowb[cpu->iowbBottom].dataLen = dataLen;
		cpu->iowb[cpu->iowbBottom].valid = true;
		cpu->iowb[cpu->iowbBottom].processed = false;
		cpu->iowb[cpu->iowbBottom].aligned = ((pa & ~(dataLen-1)) == pa);
	}

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	pthread_cond_signal(&cpu->cBoxInterfaceCond, &cpu->cBoxInterfaceMutex);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
	return;
}

/*
 * AXP_21264_PQ_Empty
 *	This function is called to determine of there is a record in the Probe
 *	Queue (PQ) that needs to be processed.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	-1:		No entries requiring processing.
 *	>=0:	Index of next entry that can be processed.
 */
int AXP_21264_PQ_Empty(AXP_21264_CPU *cpu)
{
	int		retVal = -1;
	int		ii;
	int 	start1 = -1, end1 = -1, start2 = -1, end2 = -1;

	if (cpu->pqTop > cpu->pqBottom)
	{
		start1 = cpu->pqTop;
		end1 = AXP_21264_PQ_LEN;
		start2 = 0;
		end2 = cpu->pqBottom;
	}
	else
	{
		start1 = cpu->pqTop;
		end1 = cpu->pqBottom;
	}

	/*
	 * Search through the list to find the first entry that can be processed.
	 */
	for (ii = start1; ((ii < end1) && (retVal == -1)); ii++)
		if ((cpu->pq[ii].valid == true) && (cpu->pq[ii].processed == false))
			retVal = ii;
	if ((retVal == -1) && (start2 != -1))
		for (ii = start2; ((ii < end2) && (retVal == -1)); ii++)
			if ((cpu->pq[ii].valid == true) && (cpu->pq[ii].processed == false))
				retVal = ii;

	/*
	 * Return what we found, if anything.
	 */
	return(retVal);
}

/*
 * AXP_21264_Process_PQ
 *	This function is called to check the first unprocessed entry on the queue
 *	containing the PQ records.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	entry:
 *		An integer value that is the entry in the PQ to be processed.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Process_PQ(AXP_21264_CPU *cpu, int entry)
{
	AXP_21264_PROBE_STAT probeStatus;
	AXP_VA			physAddr;
	u32				ctagIndex;
	u32				dCacheIndex;
	u32				bCacheIndex;
	u32				dCacheIndex0;
	u32				dCacheIndex1;
	u32				setToUse;
	int				ii;
	bool			locked;
	bool			bCacheValid = false;
	bool			dCacheValid = false;
	bool			dm = false;
	bool			vs = false;
	bool			ms = false;

	/*
	 * Between the time when a probe request was queued up and this queue
	 * entry was considered for processing, it may have already been processed.
	 * If so, there is no need to process it again.
	 */
	if ((cpu->pq[entry].valid == true) && (cpu->pq[entry].processed == false))
	{
		bCacheValid = AXP_21264_Bcache_Valid(cpu, cpu->pq[entry].pa);
		physAddr.va = cpu->pq[entry].pa;
		ctagIndex = physAddr.vaIdxInfo.index;

		/*
		 * We are going to be checking the dtag for a matching physical address
		 * tag.  Once checked, we will no longer need this lock.  Also note, we
		 * don't need to worry about whether one or two sets are enabled.  If
		 * only one, then the second set's valid bit will not be set.
		 */
		pthread_mutex_lock(&cpu->dtagMutex);
		dCacheIndex0 = cpu->ctag[ctagIndex][0].dtagIndex;
		dCacheIndex1 = cpu->ctag[ctagIndex][1].dtagIndex;
		if ((cpu->ctag[ctagIndex][0].valid = true) &&
			(cpu->dtag[dCacheIndex][0].physTag == physAddr.vaIdxInfo.tag))
		{
			setToUse = 0;
			dCacheIndex = dCacheIndex0;
			dCacheValid = true;
		}
		else if ((cpu->ctag[ctagIndex][1].valid = true) &&
				 (cpu->dtag[dCacheIndex][1].physTag == physAddr.vaIdxInfo.tag))
		{
			setToUse = 1;
			dCacheIndex = dCacheIndex1;
			dCacheValid = true;
		}
		pthread_mutex_unlock(&cpu->dtagMutex);

		/*
		 * We are going to play with a couple of shared structures in the Cbox,
		 * so we need to lock them down.
		 *
		 * NOTE:	If we need to lock down the Mbox shared structure, then we
		 *			need to unlock this first, in order to avoid a deadlock.
		 */
		pthread_mutex_lock(&cpu->cBoxIPRMutex);
		locked = true;
		/* TODO: I'm not sure I like the above code.  Locking should be better */

		/*
		 * If we have an A[cknowledge] from the system, decrement the
		 * outstanding
		 */
		if ((cpu->csr.SysbusAckLimit > 0) &&
			(cpu->pq[entry].a == true) &&
			(cpu->cmdAck > 0))
			cpu->cmdAck--;

		/*
		 * If the RVB bit is set, clear the VDB or IOWB specified in the ID
		 * field.
		 */
		if (cpu->pq[entry].rvb == true)
		{
			if ((cpu->pq[entry].ID &0x08) == 0x08)
			{
				cpu->iowb[cpu->pq[entry].ID & 0x07].valid = false;
				AXP_21264_Complete_SQ(cpu, cpu->iowb[cpu->pq[entry].ID & 0x07].sqEntry);
			}
			else
				cpu->vdb[cpu->pq[entry].ID].valid = false;
		}

		/*
		 * If the RPB bit is set, clear the Probe specified in the ID
		 * field.
		 */
		if (cpu->pq[entry].rpb == true)
		{
			u32	oldPqTop = cpu->pqTop;

			cpu->pq[cpu->pq[entry].ID & 0x07].valid = false;

			/*
			 * Reset the top of the queue to the first valid PQ entry.
			 */
			for (ii = oldPqTop; ii < oldPqTop+8; ii++)
			{
				if (cpu->pq[ii&0x07].valid == true)
				{
					cpu->pqTop = (cpu->pqTop + 1) & 0x07;
					break;
				}
			}
				
		}

		/*
		 * We either have both a Probe and SysDc or just a SysDc.  The SysDc
		 * controls data movement in and out of the CPU.
		 */
		if (cpu->pq[entry].notJustSysDc == true)
		{
			switch(AXP_21264_GET_PROBE_DM(cpu->pq[entry].probe))
			{
				case AXP_21264_DM_NOP:
					/* Nothing to do here. */
					break;

				case AXP_21264_DM_RDHIT:
					if (cpu->dCache[dCacheIndex][setToUse].valid == true)
					{
						/* TODO: if the cache block is in any state send to system */
						dm = true;
					}
					break;

				case AXP_21264_DM_RDDIRTY:
					if ((cpu->dCache[dCacheIndex][setToUse].valid == true) &&
						(cpu->dCache[dCacheIndex][setToUse].dirty == true))
					{
						/* TODO: if the cache block is in any state send to system */
						dm = true;
					}
					break;

				case AXP_21264_DM_RDANY:
					/* TODO: if the cache block is in any state send to system */
					dm = true;
					break;
			}
		}

		/*
		 * Process the SysDc information.
		 *
		 * NOTE: A Block state is defined as follows:
		 * 		State			Valid		Dirty		Shared
		 * 		------------	-----		-----		------
		 * 		Clean			true		false		false
		 * 		Clean/Shared	true		false		true
		 * 		Dirty			true		true		false
		 * 		Dirty/Shared	true		true		true
		 * 		Invalid			false		n/a			n/a
		 */
		pthread_mutex_lock(&cpu->dCacheMutex);
		switch(AXP_21264_GET_PROBE_NS(cpu->pq[entry].probe))
		{
			case AXP_21264_NS_NOP:
			case AXP_21264_NS_RES:
				/* Nothing to do here. */
				break;

			case AXP_21264_NS_CLEAN:
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
				{
					cpu->dCache[dCacheIndex][setToUse].dirty = false;
					cpu->dCache[dCacheIndex][setToUse].shared = false;
				}
				break;

			case AXP_21264_NS_CLEAN_SHARED:
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
				{
					cpu->dCache[dCacheIndex][setToUse].dirty = false;
					cpu->dCache[dCacheIndex][setToUse].shared = true;
				}
				break;

			case AXP_21264_NS_TRANS3:
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
				{

					/*
					 * Clean
					 */
					if ((cpu->dCache[dCacheIndex][setToUse].dirty == false) &&
						(cpu->dCache[dCacheIndex][setToUse].shared == false))
						cpu->dCache[dCacheIndex][setToUse].shared = true;

					/*
					 * Dirty/Shared
					 */
					else if ((cpu->dCache[dCacheIndex][setToUse].dirty == true) &&
							 (cpu->dCache[dCacheIndex][setToUse].shared == true))
						cpu->dCache[dCacheIndex][setToUse].dirty = false;

					/*
					 * Dirty
					 */
					else if ((cpu->dCache[dCacheIndex][setToUse].dirty == true) &&
							 (cpu->dCache[dCacheIndex][setToUse].shared == false))
						cpu->dCache[dCacheIndex][setToUse].valid = false;
				}
				break;

			case AXP_21264_NS_DIRTY_SHARED:
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
				{
					cpu->dCache[dCacheIndex][setToUse].dirty = true;
					cpu->dCache[dCacheIndex][setToUse].shared = true;
				}
				break;

			case AXP_21264_NS_INVALID:
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
					cpu->dCache[dCacheIndex][setToUse].valid = false;
				break;

			case AXP_21264_NS_TRANS1:

				/*
				 * If the block is valid then we are turning either a Clean or
				 * Dirty block into a Clean/Shared or Dirty/Shared block.  The
				 * only thing that is really changing is the shared bit (and
				 * setting an already set bit is  really a no-op).
				 */
				if (cpu->dCache[dCacheIndex][setToUse].valid == true)
					cpu->dCache[dCacheIndex][setToUse].shared = true;
				break;
		}
		pthread_mutex_unlock(&cpu->dCacheMutex);

		/*
		 * If the we hit in the Bcache (which should be a superset of both the
		 * Dcache and Icache), then we need to send a ProbeResponse.
		 * TODO: Otherwise, we send what?
		 */
		if (bCacheValid == true)
		{
			u8	dataBuf[AXP_BCACHE_BLOCK_SIZE];
			u8	vdbEntry;

			AXP_21264_Bcache_Read(cpu, cpu->pq[entry].pa, dataBuf);
			vdbEntry = AXP_21264_Add_VDB(
								cpu,
								probeResponse,
								cpu->pq[entry].pa,
								dataBuf,
								AXP_BCACHE_BLOCK_SIZE,
								true);
#if 0
			AXP_System_ProbeResponse(
							dm,			/* DM */
							false,		/* VS */
							vdbEntry,
							maf,
							probeStatus);
#endif
		}
		else
		{
		}

		/*
		 * Indicate that the entry is now processed, and unlock the Cbox IPR
		 * lock, if it is still locked.
		 */
		cpu->pq[entry].processed = true;
		if (locked == true)
			pthread_mutex_unlock(&cpu->cBoxIPRMutex);
	}

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Add_PQ
 *	This function is called to add an entry into the Probe Queue (PQ).
 *
 *	NOTE:	The system calls this function.  It does so when probing the Cbox
 *			about information within the CPU's caches.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	probe:
 *		A value indicating the probe command to queue up to the probe queue.
 *	pa:
 *		The physical address (PA) associated with the probe command.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_21264_Add_PQ(
				AXP_21264_CPU *cpu,
				int probe,
				AXP_21264_SYSDC_RSP sysDc,
				u64 pa,
				u8 id,
				bool notJustSysDc,
				bool rvb,
				bool rpb,
				bool a,
				bool c)
{

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * Queue up the next PQ entry.
	 */
	cpu->pqBottom = (cpu->pqBottom + 1) & 0x07;
	cpu->pq[cpu->pqBottom].probe = probe;
	cpu->pq[cpu->pqBottom].sysDc = sysDc;
	cpu->pq[cpu->pqBottom].pa = pa;
	cpu->pq[cpu->pqBottom].notJustSysDc = notJustSysDc;
	cpu->pq[cpu->pqBottom].rvb = rvb;
	cpu->pq[cpu->pqBottom].rpb = rpb;
	cpu->pq[cpu->pqBottom].a = a;
	cpu->pq[cpu->pqBottom].c = c;
	cpu->pq[cpu->pqBottom].marked = false;
	cpu->pq[cpu->pqBottom].valid = true;
	cpu->pq[cpu->pqBottom].processed = false;

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	pthread_cond_signal(&cpu->cBoxInterfaceCond, &cpu->cBoxInterfaceMutex);
	pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
	return;
}

/*
 * AXP_21264_Process_IRQ
 *	This function is called process and any all Interrupt Request Queue (IRQ)
 *	flags.  After processing, these flags are cleared.
 *
 *	NOTE:	The system calls this function.  It does so when one or more
 *			devices is requesting interrupt processing.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_Process_IRQ(AXP_21264_CPU *cpu)
{

	/*
	 * Queue this up to the Ibox.  We don't supply a PC or virtual address,
	 * or opcode.  We do indicate that this is an interrupt, use the unmapped
	 * register (31), this is not a write operation and we are not the Ibox.
	 */
	AXP_21264_Ibox_Event(
			cpu,
			AXP_INTERRUPT,
			0,
			0,
			0,
			AXP_UNMAPPED_REG,
			false,
			false);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Set_IRQ
 *	This function is called by the system to set Interrupt Request Queue (IRQ)
 *	flags.
 *
 *	NOTE:	This function will lock the Cbox Interface Mutex, set the IRQ_H
 *			flags, signal the Cbox Interface Condition variable, and finally
 *			unlock the mutex.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *	flags:
 *		An unsigned byte value representing the flags to be set.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_Set_IRQ(AXP_21264_CPU *cpu, u8 flags)
{

	/*
	 * Before we do anything, lock the interface mutex to prevent multiple
	 * accessors.
	 */
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * The Cbox may not have processed all the previous interrupts the system
	 * sent to the Cbox, so OR the bits here with the ones that may have been
	 * set previously.
	 */
	cpu->irqH |= flags;

	/*
	 * Let the Cbox know there is something for it to process, then unlock the
	 * mutex so it can.
	 */
	pthread_cond_signal(&cpu->cBoxInterfaceCond, &cpu->cBoxInterfaceMutex);
	pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_21264_Cbox_Init
 * 	This function is called to initialize the Cbox.  It will read in the
 * 	initialization file, which contains the settings for the Cbox CSRs.  It
 * 	will then load the SROM data into the Icache and finally set the BiSTState
 * 	if all went well.  Otherwise, it will it will return a false (BiST
 * 	failure).
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	Cbox CSRs will be set and Icache loaded with SROM initialization code.
 *
 * Return Value:
 * 	true:	Failed to perform all initialization functions.
 * 	false:	Succeeded.
 */
bool AXP_21264_Cbox_Init(AXP_21264_CPU *cpu)
{
	u32		ii, jj;
	bool	retVal = false;

	/*
	 * Initialize the Cbox IPRs.  Also, this is as good a place as any to
	 * initialize the AMASK and IMPLVER IPRs (which don't really have one of
	 * the other boxes (Cbox, Mbox, FBox, EBox, or Ibox) controlling them.
	 */
	cpu->cData.cdata = 0;
	cpu->cData.res = 0;
	cpu->cShft.c_shift = 0;
	cpu->cShft.res = 0;

	cpu->amask.bwx = 1;
	cpu->amask.fix = 1;
	cpu->amask.cix = 0;
	cpu->amask.mvi = 1;
	cpu->amask.patr = 1;
	cpu->amask.res_1 = 0;
	cpu->amask.pwmi = 0;
	cpu->amask.res_2 = 0;
	cpu->implVer = AXP_PASS_2_EV68A;

	for (ii = 1; ii < AXP_CACHE_ENTRIES; ii++)
	{
		for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
		{
			cpu->ctag[ii][jj].physTag = 0;
			cpu->ctag[ii][jj].dtagIndex = AXP_CACHE_ENTRIES;
			cpu->ctag[ii][jj].valid = false;
		}
	}

	return(retVal);
}

/*
 * AXP_21264_Cbox_Main
 * 	This is the main function for the Cbox.  It looks at each of the queues to
 * 	determine if there is anything that needs to be processed for the various
 * 	queues from other CPU boxes and the System.
 *
 * Input Parameters:
 * 	cpu:
 * 		A pointer to the CPU structure for the emulated Alpha AXP 21264
 * 		processor.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	None.
 */
void AXP_21264_Cbox_Main(AXP_21264_CPU *cpu)
{
	AXP_SROM_HANDLE	sromHdl;
	int				component = 0, ii, jj, entry;
	bool			initFailure = false, processed;

	/*
	 * The Cbox is very involved in the initialization of the CPU at power-up,
	 * fault-resetting, and waking up from sleep.  The Cbox CSRs are
	 * initialized at this point and the the SROM is loaded into the Icache.
	 * Once all this has been done, then we put the CPU into a Running state,
	 * at which point all hell can break lose.
	 *
	 * When we first come in the CPU should be in a Cold state.  We will be
	 */
	while (cpu->cpuState != ShuttingDown)
	{
		switch (cpu->cpuState)
		{
			case Cold:
				pthread_mutex_lock(&cpu->cpuMutex);
				cpu->cpuState = WaitBiST;
				cpu->BiSTState = SystemReset;
				pthread_mutex_unlock(&cpu->cpuMutex);
				break;

			case WaitBiST:
			case WaitBiSI:
				pthread_mutex_lock(&cpu->cpuMutex);

				/*
				 * HRM: 11.5.1
				 *
				 * We have a SystemReset, set the BiSTState appropriately.
				 */
				cpu->BiSTState = BiSTRunning;

				/*
				 * The other components (Ibox, Ebox, Fbox, and Mbox) should
				 * have their initialization function called to perform the
				 * initialization they need to do for themselves.  After this,
				 * they'll all be waiting for the CPU to go into Running State,
				 * which is set here after the Cbox finished its own
				 * initialization.  If any of the initialization routines
				 * return an error, an error should have been displayed and
				 * we'll change the CPU state to ShuttingDown (which will cause
				 * all the other components to shutdown as well).
				 */
				while (initFailure == false)
				{
					switch (component)
					{
						case 0:		/* Mbox */
							initFailure = AXP_21264_Mbox_Init(cpu);
							break;

						case 1:		/* Ebox */
							initFailure = AXP_21264_Ebox_Init(cpu);
							break;

						case 2:		/* Fbox */
							initFailure = AXP_21264_Fbox_Init(cpu);
							break;

						case 3:		/* Ibox */
							initFailure = AXP_21264_Ibox_Init(cpu);
							break;

						case 4:		/* Cbox */
							initFailure = AXP_21264_Cbox_Init(cpu);

						case 5:		/* Cbox Config */

							/*
							 * HRM: 11.5.2
							 *
							 * All right, BiST passed, now we have to load the
							 * SROM Cbox configuration.
							 */
							cpu->BiSTState = BiSTSucceeded;
							initFailure = AXP_21264_Cbox_Config(cpu);
							break;

						case 6:

							/*
							 * HRM: 11.5.2.1
							 *
							 * Finally, load the Instruction Cache for the
							 * initialization code.  This is where the console
							 * is loaded.
							 *
							 * TODO: Get the ROM filename from the
							 *		 configuration.
							 */
							initFailure = AXP_OpenRead_SROM("", &sromHdl);
							if (initFailure == false)
							{
								AXP_CACHE_IDX	destAddr;
								AXP_PC			palFuncPC;
								int				retVal = 1;

								/*
								 * First set the PAL_BASE IPR.
								 */
								cpu->palBase.pal_base_pc = sromHdl.destAddr;

								/*
								 * Get the PC for the RESET/WAKEUP PALcode.
								 */
						 		palFuncPC = AXP_21264_GetPALFuncvpc(
						 									cpu,
						 									AXP_RESET_WAKEUP);

						 		/*
						 		 * Set the PC to the PALcode to be called once
						 		 * the SROM has been initialized.
						 		 */
								AXP_21264_AddVPC(cpu, palFuncPC);

								/*
								 * Finally, load the ROM code into the SROM.
								 * Once we set the CPU state to Run, this will
								 * cause the Ibox to start processing
								 * instructions at the just set PC (where we
								 * loaded the ROM code).
								 */
								destAddr.offset = 0;
								destAddr.index = sromHdl.destAddr / 64;
								destAddr.res = 0;
								for (ii = destAddr.index; retVal > 0; ii++)
								{
									for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
										retVal = AXP_Read_SROM(
											&sromHdl,
											cpu->iCache[ii][jj].instructions,
											(AXP_ICACHE_LINE_INS *
											 sizeof(AXP_INS_FMT)));
								}
								initFailure = AXP_Close_SROM(&sromHdl);
								if (((retVal == AXP_E_READERR) ||
									 (retVal == AXP_E_BADSROMFILE)) &&
									(initFailure == false))
									initFailure = true;
							}
							break;
					}
					if (initFailure == false)
						component++;
				}

				/*
				 * If any of the initializations failed, then the BiST failed.
				 * We will have to perform a bunch of shutdown logic outside
				 * the top while loop.
				 */
				if (initFailure == true)
				{
					cpu->BiSTState = BiSTFailed;
					cpu->cpuState = ShuttingDown;
				}
				else
				{

					/*
					 * All the initialization work has been performed. Signal
					 * all the other threads to start to do their processing.
					 */
					cpu->cpuState = Run;
				}
				pthread_cond_broadcast(&cpu->cpuCond);
				pthread_mutex_unlock(&cpu->cpuMutex);
				break;

			case Run:

				/*
				 * We are now executing actual Alpha AXP instructions.  Monitor
				 * the interface queues and process the requests from the Mbox,
				 * Ibox and probes from the System, and responses from the
				 * System to requests sent from the Cbox.
				 */
				pthread_mutex_lock(&cpu->cBoxInterfaceMutex);
				processed = false;
				if ((entry = AXP_21264_MAF_Empty(cpu)) != -1)
				{
					AXP_21264_Process_MAF(cpu, entry);
					processed = true;
				}
				if ((entry = AXP_21264_VDB_Empty(cpu)) != -1)
				{
					AXP_21264_Process_VDB(cpu, entry);
					processed = true;
				}
				if ((entry = AXP_21264_IOWB_Empty(cpu)) == -1)
				{
					AXP_21264_Process_IOWB(cpu, entry);
					processed = true;
				}
				if ((entry = AXP_21264_PQ_Empty(cpu)) == -1)
				{
					AXP_21264_Process_PQ(cpu, entry);
					processed = true;
				}
				if (cpu->irqH != 0)
				{
					AXP_21264_Process_IRQ(cpu);
					processed = true;
				}

				/*
				 * If all the queues were empty, then wait for something to get
				 * queued up and the condition variable signaled.
				 */
				 if (processed == false)
					pthread_cond_wait(
							&cpu->cBoxInterfaceCond,
							&cpu->cBoxInterfaceMutex);

				/*
				 * Unlock the mutex so that something else could get queued up
				 * between now and when we get back into this section of code
				 * (which should be the next time through the outter loop).
				 */
				pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
				break;

			case FaultReset:
				pthread_mutex_lock(&cpu->cpuMutex);
				cpu->cpuState = WaitBiSI;
				cpu->BiSTState = SystemReset;
				pthread_mutex_unlock(&cpu->cpuMutex);
				break;

			case Sleep:

				/* 
				 * Need to quiesce everything and put the world to sleep waiting
				 * just for the wake-up signal.
				 */
				break;

			case ShuttingDown:

				/*
				 * We are shutting down.  Since we started everything, we need
				 * to clean ourself up.  The main function will be joining to
				 * all the threads it created and then freeing up the memory
				 * and exiting the image.
				 */
				pthread_exit(NULL);
				break;
		}
	}
	return;
}
