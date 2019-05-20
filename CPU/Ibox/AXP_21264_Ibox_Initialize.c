/*
 * Copyright (C) Jonathan D. Belanger 2018-2019.
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
 *  initialization functionality of the Ibox.
 *
 * Revision History:
 *
 *  V01.000 10-May-2017 Jonathan D. Belanger
 *  Initially written from AXP_21264_Ibox.c.
 *
 *  V01.001 19-May-2019 Jonathan D. Belanger
 *  GCC 7.4.0, and possibly earlier, turns on strict-aliasing rules by default.
 *  There are a number of issues in this module where the address of one
 *  variable is cast to extract a value in a different format.  In this module
 *  these all appear to be when trying to get the 64-bit value equivalent of
 *  the 64-bit long PC structure.  We will use shifts (in a macro) instead of
 *  the casts.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"
#include "CommonUtilities/AXP_Trace.h"

/*
 * AXP_21264_Ibox_ResetRegMap
 *  This function is called to [re]set the virtual to physical register
 *  mapping.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the CPU structure for the emulated Alpha AXP 21264
 *      processor.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void AXP_21264_Ibox_ResetRegMap(AXP_21264_CPU *cpu)
{
    int ii;

    if (AXP_IBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("AXP_21264_Ibox_ResetRegMap called");
        AXP_TRACE_END();
    }

    /*
     * First set up the mapping:
     *  R0 --> PR0, R1 --> PR1, ..., R38 --> PR38, R39 --> PR39
     *  F0 --> PF0, F1 --> PF1, ..., F30 --> PF30, F31 --> PF31
     *
     * NOTE:    R32 to R39 are PALshadow registers.
     */
    cpu->prFlStart = cpu->prFlEnd = 0;
    cpu->pfFlStart = cpu->pfFlEnd = 0;
    for (ii = 0; ii < AXP_INT_PHYS_REG; ii++)
    {
        cpu->pr[ii].value = 0;
        cpu->pr[ii].refCount = 0;
        if (ii < AXP_MAX_INT_REGISTERS)
        {
            cpu->pr[ii].state = Valid;
            cpu->prMap[ii] = ii;
        }
        else
        {
            cpu->pr[ii].state = Free;
            cpu->prFreeList[cpu->prFlEnd] = ii;
            cpu->prFlEnd = (cpu->prFlEnd + 1) % AXP_I_FREELIST_SIZE;
        }
        if (ii < AXP_FP_PHYS_REG)
        {
            cpu->pf[ii].value = 0;
            cpu->pf[ii].refCount = 0;
            if (ii < AXP_MAX_FP_REGISTERS)
            {
                cpu->pf[ii].state = Valid;
                cpu->pfMap[ii] = ii;
            }
            else
            {
                cpu->pf[ii].state = Free;
                cpu->pfFreeList[cpu->pfFlEnd] = ii;
                cpu->pfFlEnd = (cpu->pfFlEnd + 1) % AXP_F_FREELIST_SIZE;
            }
        }
    }

    if (AXP_IBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("");
        AXP_TraceWrite("\tInteger Physical Registers");
        for (ii = 0; ii < AXP_MAX_INT_REGISTERS; ii++)
        {
            AXP_TraceWrite("\tR%02d --> PR%02u", ii, cpu->prMap[ii]);
        }
        AXP_TraceWrite("");
        AXP_TraceWrite("\tStart = %u : End = %u",
                       cpu->prFlStart,
                       cpu->prFlEnd);
        for (ii = 0; ii < AXP_I_FREELIST_SIZE; ii++)
        {
            AXP_TraceWrite("\tR-FreeList[%d] --> PR%02u",
                           ii,
                           cpu->prFreeList[ii]);
        }
        AXP_TraceWrite("");
        AXP_TraceWrite("\tFloating-Point Physical Registers");
        for (ii = 0; ii < AXP_MAX_FP_REGISTERS; ii++)
        {
            AXP_TraceWrite("\tF%02d --> PF%02u", ii, cpu->pfMap[ii]);
        }
        AXP_TraceWrite("");
        AXP_TraceWrite("\tStart = %u : End = %u",
                       cpu->pfFlStart,
                       cpu->pfFlEnd);
        for (ii = 0; ii < AXP_F_FREELIST_SIZE; ii++)
        {
          AXP_TraceWrite("\tF-FreeList[%d] --> PF%02u",
                         ii,
                         cpu->pfFreeList[ii]);
        }
        AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_21264_Ibox_Init
 *  This function is called to initialize the Ibox.  It will set the IPRs
 *  associated with the Ibox to their initial/reset values.
 *
 * Input Parameters:
 *  cpu:
 *      A pointer to the CPU structure for the emulated Alpha AXP 21264
 *      processor.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  true:   Failed to perform all initialization processing.
 *  false:  Normal Successful Completion.
 */
bool AXP_21264_Ibox_Init(AXP_21264_CPU *cpu)
{
    bool retVal = false;
    int ii, jj, kk;

    if (AXP_IBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Ibox is initializing");
        AXP_TRACE_END();
    }

    /*
     * We start out with no exceptions pending.
     */
    cpu->excPend = false;

    /*
     * Initialize the branch prediction information.
     */
    for (ii = 0; ii < ONE_K; ii++)
    {
        cpu->localHistoryTable.lcl_history[ii] = 0;
        cpu->localPredictor.lcl_pred[ii] = 0;
        cpu->choicePredictor.choice_pred[ii] = 0;
    }
    for (ii = 0; ii < FOUR_K; ii++)
    {
        cpu->globalPredictor.gbl_pred[ii] = 0;
    }
    cpu->globalPathHistory = 0;
    for (ii = 0; ii < AXP_INFLIGHT_MAX; ii++)
    {
        AXP_PUT_PC(cpu->predictionStack[ii], 0);
    }
    cpu->predStackIdx = AXP_INFLIGHT_MAX;

    /*
     * Initialize the Ibox IPRs.
     */
    cpu->itbTag.res_1 = 0; /* ITB_TAG */
    cpu->itbTag.tag = 0;
    cpu->itbTag.res_2 = 0;
    cpu->itbPte.res_1 = 0; /* ITB_PTE */
    cpu->itbPte._asm = 0;
    cpu->itbPte.gh = 0;
    cpu->itbPte.res_2 = 0;
    cpu->itbPte.kre = 0;
    cpu->itbPte.ere = 0;
    cpu->itbPte.sre = 0;
    cpu->itbPte.ure = 0;
    cpu->itbPte.res_3 = 0;
    cpu->itbPte.pfn = 0;
    cpu->itbPte.res_4 = 0;
    cpu->itbIs.res_1 = 0; /* ITB_IS */
    cpu->itbIs.inval_itb = 0;
    cpu->itbIs.res_2 = 0;
    cpu->excAddr.exc_addr = 0; /* EXC_ADDR */
    cpu->ivaForm.form10.res = 0; /* IVA_FORM */
    cpu->ivaForm.form10.va_sext_vptb = 0;
    cpu->ierCm.res_1 = 0; /* IER_CM */
    cpu->ierCm.cm = 0;
    cpu->ierCm.res_2 = 0;
    cpu->ierCm.asten = 0;
    cpu->ierCm.sien = 0;
    cpu->ierCm.pcen = 0;
    cpu->ierCm.cren = 0;
    cpu->ierCm.slen = 0;
    cpu->ierCm.eien = 0;
    cpu->ierCm.res_3 = 0;
    cpu->sirr.res_1 = 0; /* SIRR */
    cpu->sirr.sir = 0;
    cpu->sirr.res_2 = 0;
    cpu->iSum.res_1 = 0; /* ISUM */
    cpu->iSum.astk = 0;
    cpu->iSum.aste = 0;
    cpu->iSum.res_2 = 0;
    cpu->iSum.asts = 0;
    cpu->iSum.astu = 0;
    cpu->iSum.res_3 = 0;
    cpu->iSum.si = 0;
    cpu->iSum.pc = 0;
    cpu->iSum.cr = 0;
    cpu->iSum.sl = 0;
    cpu->iSum.ei = 0;
    cpu->iSum.res_4 = 0;
    cpu->hwIntClr.res_1 = 0; /* HW_INT_CLR */
    cpu->hwIntClr.fbtp = 0;
    cpu->hwIntClr.mchk_d = 0;
    cpu->hwIntClr.res_2 = 0;
    cpu->hwIntClr.pc = 0;
    cpu->hwIntClr.cr = 0;
    cpu->hwIntClr.sl = 0;
    cpu->hwIntClr.res_3 = 0;
    cpu->excSum.swc = 0; /* EXC_SUM */
    cpu->excSum.inv = 0;
    cpu->excSum.dze = 0;
    cpu->excSum.ovf = 0;
    cpu->excSum.unf = 0;
    cpu->excSum.ine = 0;
    cpu->excSum.iov = 0;
    cpu->excSum._int = 0;
    cpu->excSum.reg = 0;
    cpu->excSum.bad_iva = 0;
    cpu->excSum.res = 0;
    cpu->excSum.pc_ovfl = 0;
    cpu->excSum.set_inv = 0;
    cpu->excSum.set_dze = 0;
    cpu->excSum.set_ovf = 0;
    cpu->excSum.set_unf = 0;
    cpu->excSum.set_ine = 0;
    cpu->excSum.set_iov = 0;
    cpu->excSum.sext_set_iov = 0;
    cpu->palBase.pal_base_pc = 0; /* PAL_BASE */
    cpu->iCtl.spce = 0; /* I_CTL */
    cpu->iCtl.ic_en = 3;
    cpu->iCtl.spe = 0;
    cpu->iCtl.sde = 0;
    cpu->iCtl.sbe = 0;
    cpu->iCtl.bp_mode = 0;
    cpu->iCtl.hwe = 0;
    cpu->iCtl.sl_xmit = 0;
    cpu->iCtl.sl_rcv = 0;
    cpu->iCtl.va_48 = 0;
    cpu->iCtl.va_form_32 = 0;
    cpu->iCtl.single_issue_h = 0;
    cpu->iCtl.pct0_en = 0;
    cpu->iCtl.pct1_en = 0;
    cpu->iCtl.call_pal_r23 = 0;
    cpu->iCtl.mchk_en = 0;
    cpu->iCtl.tb_mb_en = 0;
    cpu->iCtl.bist_fail = 1;
    cpu->iCtl.chip_id = cpu->minorType;
    cpu->iCtl.vptb = 0;
    cpu->iCtl.sext_vptb = 0;
    cpu->iStat.res_1 = 0; /* I_STAT */
    cpu->iStat.tpe = 0;
    cpu->iStat.dpe = 0;
    cpu->iStat.res_2 = 0;
    cpu->pCtx.res_1 = 0; /* PCTX */
    cpu->pCtx.ppce = 0;
    cpu->pCtx.fpe = 1;
    cpu->pCtx.res_2 = 0;
    cpu->pCtx.aster = 0;
    cpu->pCtx.astrr = 0;
    cpu->pCtx.res_3 = 0;
    cpu->pCtx.asn = 0;
    cpu->pCtx.res_4 = 0;
    cpu->pCtrCtl.sl1 = 0; /* PCTR_CTL */
    cpu->pCtrCtl.sl0 = 0;
    cpu->pCtrCtl.res_1 = 0;
    cpu->pCtrCtl.pctr1 = 0;
    cpu->pCtrCtl.res_2 = 0;
    cpu->pCtrCtl.pctr0 = 0;
    cpu->pCtrCtl.sext_pctr0 = 0;

    /*
     * Initialize the Unique instruction ID and the VPC array.
     */
    cpu->instrCounter = 0;
    cpu->vpcStart = 0;
    cpu->vpcEnd = 0;
    for (ii = 0; ii < AXP_INFLIGHT_MAX; ii++)
    {
        cpu->vpc[ii].pal = 0;
        cpu->vpc[ii].res = 0;
        cpu->vpc[ii].pc = 0;
    }

    /*
     * Initialize the instruction cache.
     */
    for (ii = 0; ii < AXP_CACHE_ENTRIES; ii++)
    {
        for (jj = 0; jj < AXP_2_WAY_CACHE; jj++)
        {
            cpu->iCache[ii][jj].kre = 0;
            cpu->iCache[ii][jj].ere = 0;
            cpu->iCache[ii][jj].sre = 0;
            cpu->iCache[ii][jj].ure = 0;
            cpu->iCache[ii][jj]._asm = 0;
            cpu->iCache[ii][jj].asn = 0;
            cpu->iCache[ii][jj].pal = 0;
            cpu->iCache[ii][jj].vb = 0;
            cpu->iCache[ii][jj].tag = 0;
            cpu->iCache[ii][jj].set_0_1 = 0;
            cpu->iCache[ii][jj].res_1 = 0;
            for (kk = 0; kk < AXP_ICACHE_LINE_INS; kk++)
            {
                cpu->iCache[ii][jj].instructions[kk].instr = 0;
            }
        }
    }

    /*
     * Initialize the Instruction Translation Look-aside Buffer.
     */
    cpu->nextITB = 0;
    for (ii = 0; ii < AXP_TB_LEN; ii++)
    {
        cpu->itb[ii].virtAddr = 0;
        cpu->itb[ii].physAddr = 0;
        cpu->itb[ii].matchMask = 0;
        cpu->itb[ii].keepMask = 0;
        cpu->itb[ii].kre = 0;
        cpu->itb[ii].ere = 0;
        cpu->itb[ii].sre = 0;
        cpu->itb[ii].ure = 0;
        cpu->itb[ii].kwe = 0;
        cpu->itb[ii].ewe = 0;
        cpu->itb[ii].swe = 0;
        cpu->itb[ii].uwe = 0;
        cpu->itb[ii].faultOnRead = 0;
        cpu->itb[ii].faultOnWrite = 0;
        cpu->itb[ii].faultOnExecute = 0;
        cpu->itb[ii].res_1 = 0;
        cpu->itb[ii].asn = 0;
        cpu->itb[ii]._asm = false;
        cpu->itb[ii].valid = false;
    }

    /*
     * Initialize the ReOrder Buffer (ROB).
     */
    cpu->robStart = 0;
    cpu->robEnd = 0;
    for (ii = 0; ii < AXP_INFLIGHT_MAX; ii++)
    {
        cpu->rob[ii].state = Retired;
    }

    if (AXP_IBOX_OPT1)
    {
        AXP_TRACE_BEGIN();
        AXP_TraceWrite("Ibox has initialized");
        AXP_TRACE_END();
    }

    /*
     * Return the result of this initialization back to the caller.
     */
    return (retVal);
}

