/*
 * Copyright (C) Jonathan D. Belanger 2017-2019.
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
 *  This source file contains the functions needed to implement the
 *  functionality of the Cbox.
 *
 * Revision History:
 *
 *  V01.000 26-May-2017 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 28-Oct-2017 Jonathan D. Belanger
 *  Time to fill out the functionality of the Cbox.  Some of the functions
 *  previously defined, may be going away.  We are going to need functions for
 *  all the requests needed by the CPU of the system (from Mbox, Ibox, and
 *  Cbox).  Additionally, IRQ_H interrupt signals from the System to the Ibox
 *  for processing.
 *
 *  V01.002 16-Nov-2017 Jonathan D. Belanger
 *  Added functions to Flush, Read, Write, and Check for a Valid record for
 *  the Bcache.
 *
 *  V01.003 22-Dec-2017 Jonathan D. Belanger
 *  Worked through the IOWB processing.  This code is pretty much complete.
 *
 *  V01.004 26-Dec-2017 Jonathan D. Belanger
 *  Started on the VDB and MAF processing.  Need to take another look at PQ
 *  processing.
 *
 *  V01.005 29-Dec-2017 Jonathan D. Belanger
 *  This module is getting too large (over 4000 lines).  I'm breaking it up
 *  into six files.  One for each of Bcache, PQ, MAF, IOWB, VDB, and Cbox.
 *
 *  V01.006 12-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CPU/Cbox/AXP_21264_Cbox.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Blocks.h"
#include "CommonUtilities/AXP_NameValuePair_Read.h"
#include "CPU/Caches/AXP_21264_CacheDefs.h"
#include "CPU/Mbox/AXP_21264_Mbox.h"
#include "CPU/Ebox/AXP_21264_Ebox.h"
#include "CPU/Fbox/AXP_21264_Fbox.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"
#include "CPU/Ibox/AXP_21264_Ibox_Initialize.h"
#include "CPU/Ibox/AXP_21264_Ibox_PCHandling.h"
#include "CommonUtilities/AXP_Trace.h"
#include "CommonUtilities/AXP_Dumps.h"

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

const char *componentStr[] =
{
    "Mbox Initialization",
    "Ebox Initialization",
    "Fbox Initialization",
    "Ibox Initialization",
    "Cbox Initialization",
    "Cbox Configuration",
    "Load from SROM"
};

/*
 * AXP_21264_Cbox_IQArbiter
 *  This function is responsible for the arbitration of instructions pending in
 *  the Integer Issue Queue (IQ).  Architecturally, this is defined in the Cbox
 *  as the "Arbiter".  According to the architecture, there are two(2) arbiters
 *  for the IQ, one for the upper sub-clusters and one for the lower.  Each
 *  will select two instructions from the 20 possible queued integer
 *  instructions.  Priority is given to older requests over newer ones.  If one
 *  instruction requests both lower sub-clusters and there is no other
 *  requesting a lower sub-cluster, then L0 is selected.  The same, but for the
 *  upper-clusters will select U1.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *
 * Return Value:
 */
void
AXP_21264_Cbox_IQArbiter(AXP_21264_CPU *cpu)
{
    /* TODO: This code needs to be completed */
    return;
}

/*
 * AXP_21264_Cbox_FQArbiter
 *  This function is responsible for the arbitration of instructions pending in
 *  the Floating-point Issue Queue (FQ).  Architecturally, this is defined in
 *  the Cbox as the "Arbiter".  According to the architecture, there are
 *  three(3) arbiters for the FQ, one for each add, multiply, or store
 *  pipelines.  The add and multiply arbiters will pick one request, while the
 *  store arbiter will pick two (one for each store pipeline).  Priority is
 *  given to older requests over newer ones.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *
 * Return Value:
 */
void
AXP_21264_Cbox_FQArbiter(AXP_21264_CPU *cpu)
{
    /* TODO: This code needs to be completed */
    return;
}

/*
 * AXP_21264_Cbox_Config
 *   This function is called to configure the Cbox from an initialization file
 *   (aka: SROM), which contains the settings for the Cbox CSRs.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *   Cbox CSRs will be set with SROM initialization.
 *
 * Return Value:
 *   true:    Failed to perform all initialization functions.
 *   false:    Succeeded.
 */
bool
AXP_21264_Cbox_Config(AXP_21264_CPU *cpu)
{
    bool retVal = false;
    bool readResult = true;
    FILE *fp;
    char name[80];
    u32 value;
    int ii, csrCnt = 0;
    AXP_21264_CBOX_CSR_VAL csr;

    retVal = !AXP_ConfigGet_CboxCSRFile(name);
    if (AXP_CBOX_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Cbox is loading CSR values from %s", name);
        AXP_TRACE_END();
    }

    /*
     * Open the file to configure the CSRs for the Cbox.
     */
    fp = AXP_Open_NVP_File(name);
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
                if (csrCnt < AXP_21264_CBOX_CSR_CNT)
                {
                    if (AXP_CBOX_CALL)
                    {
                        AXP_TRACE_BEGIN();
                        AXP_TraceWrite("Not all CSRs loaded");
                        AXP_TRACE_END();
                    }
                    retVal = true;
                }
                continue;
            }

            /*
             * OK, if we get this far we have a name, in string format and a
             * 32-bit value.  We need to convert the string name to an
             * equivalent name value, so that we can use select to determine
             * which of the CSRs is being initialized with the value.
             */
            csr = LastCSR;
            for (ii = 0;
                 ((csrNames[ii].name != NULL) && (csr == LastCSR));
                 ii++)
            {
                if (strcmp(csrNames[ii].name, name) == 0)
                {
                    csr = csrNames[ii].values;
                    csrCnt++;
                }
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
                {
                    int bCacheArraySize;

                    cpu->csr.BcSize = value;

                    /*
                     * OK, now allocate a Bcache large enough for the size.
                     * NOTE: Each Bcache block contains 64 bytes, so the
                     * array size is the Bcache size divided by 64.
                     */
                    bCacheArraySize = ((cpu->csr.BcSize + 1) *
                                      ONE_M) / AXP_BCACHE_BLOCK_SIZE;

                    /*
                     * Go and actually allocate the 2 arrays needed for the
                     * Bcache (the cache array and the tag array).
                     */
                    cpu->bCache =
                        AXP_Allocate_Block(-(bCacheArraySize *
                                             sizeof(AXP_21264_BCACHE_BLK)),
                                           cpu->bCache);
                    cpu->bTag = AXP_Allocate_Block(-(bCacheArraySize *
                                                     sizeof(
                                                         AXP_21264_BCACHE_TAG)),
                                                   cpu->bTag);

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
                    if (AXP_CBOX_OPT1)
                    {
                        AXP_TRACE_BEGIN();
                        AXP_TraceWrite(
                            "Unexpected name/value pair: 'name' returned "
                            "as \'%s\' at %s, line %d.\n",
                            name,
                            __FILE__,
                            __LINE__);
                        AXP_TRACE_END();
                    }
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
    else
    {
        fprintf(stderr,
                "%%AXP-F-FILEERR, %s not found or cannot be opened for read\n",
                name);
        fflush(stderr);
        retVal = true;
    }

    if (AXP_CBOX_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite((retVal == false ? "Cbox CSRs have been loaded" :
                                          "Cbox CSRs failed to load"));
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return (retVal);
}

/*
 * AXP_21264_Process_IRQ
 *  This function is called process and any all Interrupt Request Queue (IRQ)
 *  flags.  After processing, these flags are cleared.
 *
 *  NOTE:   The system calls this function.  It does so when one or more
 *          devices is requesting interrupt processing.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *   None.
 *
 * Return Value:
 *   None.
 */
void
AXP_21264_Process_IRQ(AXP_21264_CPU *cpu)
{
    AXP_PC pc;

    pc.pal = 0;
    pc.res = 0;
    pc.pc = 0;

    /*
     * Queue this up to the Ibox.  We don't supply a PC or virtual address,
     * or opcode.  We do indicate that this is an interrupt, use the unmapped
     * register (31), this is not a write operation and we are not the Ibox.
     */
    AXP_21264_Ibox_Event(cpu,
                         AXP_INTERRUPT,
                         pc,
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
 *  This function is called by the system to set Interrupt Request Queue (IRQ)
 *  flags.
 *
 *  NOTE:   This function will lock the Cbox Interface Mutex, set the IRQ_H
 *          flags, signal the Cbox Interface Condition variable, and finally
 *          unlock the mutex.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *  flags:
 *      An unsigned byte value representing the flags to be set.
 *
 * Output Parameters:
 *   None.
 *
 * Return Value:
 *   None.
 */
void
AXP_21264_Set_IRQ(AXP_21264_CPU *cpu, u8 flags)
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
    pthread_cond_signal(&cpu->cBoxInterfaceCond);
    pthread_mutex_lock(&cpu->cBoxInterfaceMutex);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21264_Cbox_Init
 *   This function is called to initialize the Cbox.  It will read in the
 *   initialization file, which contains the settings for the Cbox CSRs.  It
 *   will then load the SROM data into the Icache and finally set the BiSTState
 *   if all went well.  Otherwise, it will it will return a false (BiST
 *   failure).
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *   Cbox CSRs will be set and Icache loaded with SROM initialization code.
 *
 * Return Value:
 *   true:    Failed to perform all initialization functions.
 *   false:    Succeeded.
 */
bool
AXP_21264_Cbox_Init(AXP_21264_CPU *cpu)
{
    u32 ii, jj;
    bool retVal = false;

    if (AXP_CBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Cbox is initializing");
        AXP_TRACE_END();
    }

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
            cpu->ctag[ii][jj].dirty = false;
            cpu->ctag[ii][jj].shared = false;
        }
    }
    for (ii = 0; ii < AXP_21264_MAF_LEN; ii++)
    {
        cpu->maf[ii].type = MAFNotInUse;
        cpu->maf[ii].pa = 0;
        cpu->maf[ii].mask = 0;
        cpu->maf[ii].dataLen = 0;
        cpu->maf[ii].bufLen = 0;
        cpu->maf[ii].valid = false;
        cpu->maf[ii].complete = false;
        cpu->maf[ii].shared = false;
        cpu->maf[ii].ioReq = false;
        for (jj = 0; jj < AXP_21264_MBOX_MAX; jj++)
            cpu->maf[ii].lqSqEntry[jj] = 0;
    }
    for (ii = 0; ii < AXP_21264_VDB_LEN; ii++)
    {
        cpu->vdb[ii].type = toBcache;
        cpu->vdb[ii].pa = 0;
        cpu->vdb[ii].validVictim = false;
        cpu->vdb[ii].validProbe = false;
        cpu->vdb[ii].processed = false;
        cpu->vdb[ii].valid = false;
        cpu->vdb[ii].marked = false;
    }
    for (ii = 0; ii < AXP_21264_PQ_LEN; ii++)
    {
        cpu->pq[ii].pa = 0;
        cpu->pq[ii].sysDc = SysDC_Nop;
        cpu->pq[ii].probeStatus = HitClean;
        cpu->pq[ii].probe = 0;
        cpu->pq[ii].rvb = false;
        cpu->pq[ii].rpb = false;
        cpu->pq[ii].a = false;
        cpu->pq[ii].c = false;
        cpu->pq[ii].processed = false;
        cpu->pq[ii].valid = false;
        cpu->pq[ii].pendingRsp = false;
        cpu->pq[ii].dm = false;
        cpu->pq[ii].vs = false;
        cpu->pq[ii].ms = false;
        cpu->pq[ii].ID = 0;
        cpu->pq[ii].vdb = 0;
        cpu->pq[ii].maf = 0;
    }
    for (ii = 0; ii < AXP_21264_IOWB_LEN; ii++)
    {
        cpu->iowb[ii].processed = false;
        cpu->iowb[ii].valid = false;
        cpu->iowb[ii].pa = 0;
        cpu->iowb[ii].mask = 0;
        cpu->iowb[ii].storeLen = 0;
        cpu->iowb[ii].bufLen = 0;
        for (jj = 0; jj < AXP_21264_MBOX_MAX; jj++)
            cpu->iowb[ii].lqSqEntry[jj] = 0;
    }

    if (AXP_CBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Cbox has initialized");
        AXP_TRACE_END();
    }

    return (retVal);
}

/*
 * AXP_21264_Cbox_Main
 *   This is the main function for the Cbox.  It looks at each of the queues to
 *   determine if there is anything that needs to be processed for the various
 *   queues from other CPU boxes and the System.
 *
 * Input Parameters:
 *   cpu:
 *       A pointer to the CPU structure for the emulated Alpha AXP 21264
 *       processor.
 *
 * Output Parameters:
 *   None.
 *
 * Return Value:
 *   None.
 */
void *
AXP_21264_CboxMain(void *voidPtr)
{
    AXP_21264_CPU *cpu = (AXP_21264_CPU *) voidPtr;
    AXP_SROM_HANDLE sromHdl;
    char name[80];
    u64 ii;
    int component = 0, jj, entry;
    bool initFailure = false, processed;

    if (AXP_CBOX_CALL)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Cbox is starting");
        AXP_TRACE_END();
    }

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
                if (AXP_CBOX_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Cbox is performing a Cold Start");
                    AXP_TRACE_END();
                }
                pthread_mutex_lock(&cpu->cpuMutex);
                cpu->cpuState = WaitBiST;
                cpu->BiSTState = SystemReset;
                pthread_mutex_unlock(&cpu->cpuMutex);
                break;

            case WaitBiST:
            case WaitBiSI:
                if (AXP_CBOX_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Cbox is performing a BiST/BiSI running");
                    AXP_TRACE_END();
                }
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
                while ((initFailure == false) && (component < 7))
                {
                    switch (component)
                    {
                        case 0: /* Mbox */
                            initFailure = AXP_21264_Mbox_Init(cpu);
                            break;

                        case 1: /* Ebox */
                            initFailure = AXP_21264_Ebox_Init(cpu);
                            break;

                        case 2: /* Fbox */
                            initFailure = AXP_21264_Fbox_Init(cpu);
                            break;

                        case 3: /* Ibox */
                            initFailure = AXP_21264_Ibox_Init(cpu);
                            break;

                        case 4: /* Cbox */
                            initFailure = AXP_21264_Cbox_Init(cpu);
                            break;

                        case 5: /* Cbox Config */

                            /*
                             * HRM: 11.5.2
                             *
                             * All right, BiST passed, now we have to load the
                             * SROM Cbox configuration.
                             */
                            if (AXP_CBOX_OPT2)
                            {
                                AXP_TRACE_BEGIN();
                                AXP_TraceWrite("Cbox BiST/BiSI passed.  "
                                               "Configuring Cbox.");
                                AXP_TRACE_END();
                            }
                            cpu->BiSTState = BiSTSucceeded;
                            initFailure = AXP_21264_Cbox_Config(cpu);
                            break;

                        case 6:
                            if (AXP_CBOX_OPT2)
                            {
                                AXP_TRACE_BEGIN();
                                AXP_TraceWrite("Cbox is configured.  "
                                               "Loading SROM.");
                                AXP_TRACE_END();
                            }

                            /*
                             * HRM: 11.5.2.1
                             *
                             * Finally, load the Instruction Cache for the
                             * initialization code.  This is where the console
                             * is loaded.
                             */
                            initFailure = !AXP_ConfigGet_ROMFile(name);
                            if (initFailure == false)
                            {
                                initFailure = AXP_OpenRead_SROM(name, &sromHdl);
                            }
                            if (initFailure == false)
                            {
                                AXP_IBOX_ITB_PTE pte;
                                AXP_PC startingPC;
                                u32 instructions[AXP_ICACHE_LINE_INS];
                                int retVal = 1;

                                /*
                                 * First set the ITB_PTE IPR so that we can
                                 * load and execute the initialization
                                 * instructions.
                                 */
                                pte = cpu->itbPte;
                                cpu->itbPte._asm = 1;
                                cpu->itbPte.kre = 1;
                                cpu->itbPte.sre = 1;
                                cpu->itbPte.ere = 1;
                                cpu->itbPte.ure = 1;

                                /*
                                 * Add the ITB, into which we will be loading
                                 * the instructions.
                                 */
                                AXP_addTLBEntry(cpu,
                                                sromHdl.destAddr,
                                                sromHdl.destAddr,
                                                false);

                                /*
                                 * Finally, load the ROM code into the SROM.
                                 * Once we set the CPU state to Run, this will
                                 * cause the Ibox to start processing
                                 * instructions at the just set PC (where we
                                 * loaded the ROM code).
                                 */
                                for (ii = sromHdl.destAddr;
                                     retVal > 0;
                                     ii += (AXP_ICACHE_LINE_INS * sizeof(u32)))
                                {
                                    retVal = AXP_Read_SROM(&sromHdl,
                                                           instructions,
                                                           AXP_ICACHE_LINE_INS);
                                    AXP_21264_Ibox_UpdateIcache(cpu,
                                                                ii,
                                                                (u8 *)
                                                                instructions,
                                                                true);
                                    if ((AXP_CBOX_INST) && (retVal > 0))
                                    {
                                        char traceBuf[256];

                                        AXP_PUT_PC(startingPC, ii);
                                        AXP_TRACE_BEGIN();
                                        for (jj = 0;
                                             jj < AXP_ICACHE_LINE_INS;
                                             jj++)
                                        {
                                            AXP_Decode_Instruction(&startingPC,
                                                                   (AXP_INS_FMT) instructions[jj],
                                                                   true,
                                                                   traceBuf);
                                            AXP_TraceWrite(traceBuf);
                                            startingPC.pc++;
                                        }
                                        AXP_TRACE_END();
                                    }
                                }
                                cpu->itbPte = pte;

                                /*
                                 * Get the PC for the base address of the code
                                 * just loaded.  We're putting this in PALmode.
                                 */
                                startingPC = AXP_21264_MakeVPC(cpu,
                                                               sromHdl.destAddr,
                                                               AXP_PAL_MODE);
                                initFailure = AXP_Close_SROM(&sromHdl);
                                if (((retVal == AXP_E_READERR) ||
                                     (retVal == AXP_E_BADSROMFILE)) &&
                                    (initFailure == false))
                                {
                                    initFailure = true;
                                }

                                /*
                                 * Set the PC to the code to be called now that
                                 * the SROM has been initialized.
                                 */
                                AXP_21264_AddVPC(cpu, startingPC);
                            }
                            else
                            {
                                fprintf(stderr,
                                        "%%AXP-F-FILEERR, %s not found or "
                                        "cannot be opened for read\n",
                                        name);
                                fflush(stderr);
                            }
                            break;
                    }

                    /*
                     * If the current component successfully initialized, then
                     * move on to the next.
                     */
                    if (initFailure == false)
                    {
                        component++;
                    }
                }

                /*
                 * If any of the initializations failed, then the BiST failed.
                 * We will have to perform a bunch of shutdown logic outside
                 * the top while loop.
                 */
                if (initFailure == true)
                {
                    if (AXP_CBOX_OPT2)
                    {
                        AXP_TRACE_BEGIN();
                        AXP_TraceWrite("CPU Startup has failed at component %s",
                                       componentStr[component]);
                        AXP_TRACE_END();
                    }
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
                    if (AXP_CBOX_OPT2)
                    {
                        AXP_TRACE_BEGIN();
                        AXP_TraceWrite("-------------------------------------"
                                       "-----------------------------");
                        AXP_TraceWrite(
                            "The Digital Alpha AXP 21264 CPU Emulator is "
                            "now in a Running state");
                        AXP_TraceWrite("-------------------------------------"
                                       "-----------------------------");
                        AXP_TRACE_END();
                    }
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
                if ((entry = AXP_21264_IOWB_Empty(cpu)) != -1)
                {
                    AXP_21264_Process_IOWB(cpu, entry);
                    processed = true;
                }
                if ((entry = AXP_21264_PQ_Empty(cpu)) != -1)
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
                {
                    pthread_cond_wait(&cpu->cBoxInterfaceCond,
                                      &cpu->cBoxInterfaceMutex);
                }

                /*
                 * Unlock the mutex so that something else could get queued up
                 * between now and when we get back into this section of code
                 * (which should be the next time through the outter loop).
                 */
                pthread_mutex_unlock(&cpu->cBoxInterfaceMutex);
                break;

            case FaultReset:
                if (AXP_CBOX_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Cbox got a Reset Fault");
                    AXP_TRACE_END();
                }
                pthread_mutex_lock(&cpu->cpuMutex);
                cpu->cpuState = WaitBiSI;
                cpu->BiSTState = SystemReset;
                pthread_mutex_unlock(&cpu->cpuMutex);
                break;

            case Sleep:
                if (AXP_CBOX_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Cbox is in Sleep State");
                    AXP_TRACE_END();
                }

                /*
                 * Need to quiesce everything and put the world to sleep waiting
                 * just for the wake-up signal.
                 */
                break;

            case ShuttingDown:
                if (AXP_CBOX_OPT2)
                {
                    AXP_TRACE_BEGIN();
                    AXP_TraceWrite("Cbox is Shutting Down.");
                    AXP_TRACE_END();
                }

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
    return (NULL);
}
