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
 */
#include "AXP_Configure.h"
#include "AXP_21264_CboxDefs.h"
#include "AXP_NameValuePair_Read.h"
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
 * AXP_VictimBuffer
 *	This function is responsible for managing the Victim Buffer (VB).  The VB
 *	is comprised of the Victim Address File (VAF) and Victim Data File (VDF).
 *	There are a total of eight(8) entries in the VB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_VictimBuffer()
{
	return;
}

/*
 * AXP_IOWriteBuffer
 *	This function is responsible for managing the I/O Write Buffer (IOWB).
 *	There are a total of four(4) 64-byte entries in the IOWB.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_IOWriteBuffer()
{
	return;
}

/*
 * AXP_ProbeQueue
 *	This function is responsible for managing the Probe Queue (PQ).
 *	There are a total of eight(8) entries in the PQ.
 *
 * Input Parameters:
 *
 * Output Parameters:
 *
 * Return Value:
 */
void AXP_ProbeQueue()
{
	return;
}

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
	bool						retVal = false;
	bool						readResult = true;
	FILE						*fp;
	char					*configFile = "../dat/AXP_21264_Cbox_CSR.nvp";
	char						name[32];
	u32							value;
	int							ii;
	AXP_21264_CBOX_CSR_VALUES	csr;

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
					switch (cpu->csr.BcSize)
					{
						case AXP_BCACHE_1MB:
							bCacheArraySize =
									AXP_21264_1MB / AXP_BCACHE_BLOCK_SIZE;
							break;

						case AXP_BCACHE_2MB:
							bCacheArraySize =
									AXP_21264_2MB / AXP_BCACHE_BLOCK_SIZE;
							break;

						case AXP_BCACHE_4MB:
							bCacheArraySize =
									AXP_21264_4MB / AXP_BCACHE_BLOCK_SIZE;
							break;

						case AXP_BCACHE_8MB:
							bCacheArraySize =
									AXP_21264_8MB / AXP_BCACHE_BLOCK_SIZE;
							break;

						case AXP_BCACHE_16MB:
							bCacheArraySize =
									AXP_21264_16MB / AXP_BCACHE_BLOCK_SIZE;
							break;
					}

					/*
					 * Go and actually allocate the 2 arrays needed for the
					 * Bcache (the cache array and the tag array).
					 */
					cpu->bCache = calloc(
									bCacheArraySize,
									sizeof(AXP_21264_BCACHE_BLK);
					cpu->bTag = calloc(
									bCacheArraySize,
									sizeof(AXP_21264_BCACHE_TAG);

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
	bool	initFailure = false;
	int		component = 0;

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
				cpu->cpuState = WaitBiST;
				cpu->BiSTState = SystemReset;
				break;

			case WaitBiST:
			case WaitBiSI:

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

						case 5:		/* Cbox Config*/

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
							 */
/* TODO:					initFailure = AXP_21264_LoadFlashROM(cpu);	*/
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
					 * TODO: Send Exception to the Ibox.
					 *
					 * Need to send a RESET/WAKEUP Exception to the Ibox.
					 * Since the above initialization code set the PALBase to
					 * 0x0, the RESET/WAKEUP event will cause the Ibox to jump
					 * to the Offset in the PALcode for the code to finalize
					 * the initialization and run the console.  This offset is
					 * located at PAL_BASE + 0x780 (or 0x780, in PAL_MODE).
					 */
					cpu->cpuState = Run;
				}
				break;

			case Run:
				break;

			case FaultReset:
				cpu->cpuState = WaitBiSI;
				cpu->BiSTState = SystemReset;
				break;

			case Sleep:
				break;

			case ShuttingDown:
			default:
				break;
		}
	}
	return;
}
