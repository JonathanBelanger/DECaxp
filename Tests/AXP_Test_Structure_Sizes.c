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
 *  This source file contains tests to verify that the structure definitions
 *  have a correct sizeof() result, for key structures.
 *
 * Revision History:
 *
 *  V01.000        10-Jun-2017    Jonathan D. Belanger
 *  Initially written.
 *
 */

/*
 * Include comutl.
 */
#include "CPU/Cbox/AXP_21264_Cbox.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Blocks.h"

/*
 * Include cpu
 */
#include "CPU/AXP_21264_Instructions.h"
#include "CPU/AXP_Base_CPU.h"
#include "CPU/AXP_21264_IPRs.h"
#include "CPU/Ibox/AXP_21264_Predictions.h"
#include "CPU/Caches/AXP_21264_Cache.h"
#include "CPU/Ibox/AXP_21264_Ibox.h"
#include "CPU/Fbox/AXP_21264_Fbox.h"
#include "CPU/AXP_21264_CPU.h"
#include "Devices/VirtualDisks/AXP_VHDX.h"

#define TST_PASS 0
#define TST_FAIL -1
#define TST_EXPECT 1
#define PRINT_SIZE(type, size, test)                                        \
{                                                                           \
    int curTst = TST_PASS;                                                  \
    if (strcmp(#type, "AXP_VHDX_META_PAR_HDR") == 0)                        \
    {                                                                       \
        if (sizeof(type) != 24)                                             \
        {                                                                   \
            curTst = TST_EXPECT;                                            \
        }                                                                   \
        else                                                                \
        {                                                                   \
            curTst = TST_FAIL;                                              \
        }                                                                   \
    }                                                                       \
    else if (sizeof(type) != size)                                          \
    {                                                                       \
        curTst = TST_FAIL;                                                  \
    }                                                                       \
    printf("    %-25s= %4lu (%4d): %s\n",                                   \
           #type,                                                           \
           sizeof(type),                                                    \
           size,                                                            \
           (curTst == TST_PASS ?                                            \
               pass :                                                       \
               (curTst == TST_FAIL ?                                        \
                   fail :                                                   \
                   expected)));                                             \
    test = (curTst == TST_FAIL ? false : true);                             \
}

/*
 * main
 *   This function is called to verify that certain critical data structures are
 *   defined properly by size.
 *
 * Input Parameters:
 *   None.
 *
 * Output Parameters:
 *   None.
 *
 * Return Value:
 *   Success(0).
 */
int main()
{
    char *pass = "passed";
    char *fail = "failed";
    char *expected = "expected";
    bool passed = true;

    printf("Alpha AXP 21264 Data Structure Size Test.\n\n");
    printf("AXP_Utility.h\n");
    PRINT_SIZE(u8, 1, passed);
    PRINT_SIZE(u16, 2, passed);
    PRINT_SIZE(u32, 4, passed);
    PRINT_SIZE(u64, 8, passed);
    PRINT_SIZE(u128, 16, passed);
    PRINT_SIZE(i8, 1, passed);
    PRINT_SIZE(i16, 2, passed);
    PRINT_SIZE(i32, 4, passed);
    PRINT_SIZE(i64, 8, passed);
    PRINT_SIZE(i128, 16, passed);

    printf("\nAXP_21264_Instructions.h\n");
    PRINT_SIZE(AXP_MEM_INS, 4, passed);
    PRINT_SIZE(AXP_BR_INS, 4, passed);
    PRINT_SIZE(AXP_OP1_INS, 4, passed);
    PRINT_SIZE(AXP_OP2_INS, 4, passed);
    PRINT_SIZE(AXP_FP_INS, 4, passed);
    PRINT_SIZE(AXP_PAL_INS, 4, passed);
    PRINT_SIZE(AXP_HW_LD, 4, passed);
    PRINT_SIZE(AXP_HW_ST, 4, passed);
    PRINT_SIZE(AXP_HW_RET, 4, passed);
    PRINT_SIZE(AXP_HW_MXPR, 4, passed);
    PRINT_SIZE(AXP_INS_FMT, 4, passed);

    printf("\nAXP_Base_CPU.h\n");
    PRINT_SIZE(AXP_INT_REGISTER, 8, passed);
    PRINT_SIZE(AXP_B_MEMORY, 1, passed);
    PRINT_SIZE(AXP_W_MEMORY, 2, passed);
    PRINT_SIZE(AXP_L_MEMORY, 4, passed);
    PRINT_SIZE(AXP_Q_MEMORY, 8, passed);
    PRINT_SIZE(AXP_F_REGISTER_CVT, 8, passed);
    PRINT_SIZE(AXP_FPR_REGISTER, 8, passed);
    PRINT_SIZE(AXP_G_REGISTER_CVT, 8, passed);
    PRINT_SIZE(AXP_D_REGISTER_CVT, 8, passed);
    PRINT_SIZE(AXP_FDR_REGISTER, 8, passed);
    PRINT_SIZE(AXP_S_REGISTER_CVT, 8, passed);
    PRINT_SIZE(AXP_X_REGISTER, 16, passed);
    PRINT_SIZE(AXP_L_REGISTER,8, passed);
    PRINT_SIZE(AXP_Q_REGISTER,8, passed);
    PRINT_SIZE(AXP_FP_REGISTER, 8, passed);
    PRINT_SIZE(AXP_F_MEMORY, 4, passed);
    PRINT_SIZE(AXP_G_MEMORY, 8, passed);
    PRINT_SIZE(AXP_D_MEMORY, 8, passed);
    PRINT_SIZE(AXP_S_MEMORY, 4, passed);
    PRINT_SIZE(AXP_PC, 8, passed);
    PRINT_SIZE(AXP_PTE, 8, passed);
    PRINT_SIZE(AXP_BASE_ASN, 8, passed);
    PRINT_SIZE(AXP_BASE_ASTEN, 8, passed);
    PRINT_SIZE(AXP_BASE_ASTEN_R16, 8, passed);
    PRINT_SIZE(AXP_BASE_ASTSR, 8, passed);
    PRINT_SIZE(AXP_BASE_ASTSR_R16, 8, passed);
    PRINT_SIZE(AXP_BASE_DATFX, 8, passed);
    PRINT_SIZE(AXP_BASE_ESP, 8, passed);
    PRINT_SIZE(AXP_BASE_FEN, 8, passed);
    PRINT_SIZE(AXP_BASE_IPL, 8, passed);
    PRINT_SIZE(AXP_BASE_KSP, 8, passed);
    PRINT_SIZE(AXP_BASE_MCES, 8, passed);
    PRINT_SIZE(AXP_BASE_PCBB, 8, passed);
    PRINT_SIZE(AXP_BASE_PRBR, 8, passed);
    PRINT_SIZE(AXP_BASE_PTBR, 8, passed);
    PRINT_SIZE(AXP_BASE_SCBB, 8, passed);
    PRINT_SIZE(AXP_BASE_SIRR, 8, passed);
    PRINT_SIZE(AXP_BASE_SISR, 8, passed);
    PRINT_SIZE(AXP_BASE_SSP, 8, passed);
    PRINT_SIZE(AXP_BASE_SYSPTBR, 8, passed);
    PRINT_SIZE(AXP_BASE_TBCHK, 8, passed);
    PRINT_SIZE(AXP_BASE_TBCHK_R16, 8, passed);
    PRINT_SIZE(AXP_BASE_USP, 8, passed);
    PRINT_SIZE(AXP_BASE_VIRBND, 8, passed);
    PRINT_SIZE(AXP_BASE_VPTB, 8, passed);
    PRINT_SIZE(AXP_BASE_WHAMI, 8, passed);

    printf("\nAXP_21264_IPRs.h\n");
    PRINT_SIZE(AXP_EBOX_CC, 8, passed);
    PRINT_SIZE(AXP_EBOX_CC_CTL, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA_CTL, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA_FORM_00, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA_FORM_01, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA_FORM_10, 8, passed);
    PRINT_SIZE(AXP_EBOX_VA_FORM, 8, passed);
    PRINT_SIZE(AXP_FBOX_FPCR, 8, passed);
    PRINT_SIZE(AXP_IBOX_ITB_TAG, 8, passed);
    PRINT_SIZE(AXP_IBOX_ITB_PTE, 8, passed);
    PRINT_SIZE(AXP_IBOX_ITB_IS, 8, passed);
    PRINT_SIZE(AXP_IBOX_EXC_ADDR, 8, passed);
    PRINT_SIZE(AXP_IBOX_IVA_FORM_00, 8, passed);
    PRINT_SIZE(AXP_IBOX_IVA_FORM_10, 8, passed);
    PRINT_SIZE(AXP_IBOX_IVA_FORM_01, 8, passed);
    PRINT_SIZE(AXP_IBOX_IVA_FORM, 8, passed);
    PRINT_SIZE(AXP_IBOX_IER_CM, 8, passed);
    PRINT_SIZE(AXP_IBOX_SIRR, 8, passed);
    PRINT_SIZE(AXP_IBOX_ISUM, 8, passed);
    PRINT_SIZE(AXP_IBOX_HW_INT_CLR, 8, passed);
    PRINT_SIZE(AXP_IBOX_EXC_SUM, 8, passed);
    PRINT_SIZE(AXP_IBOX_PAL_BASE, 8, passed);
    PRINT_SIZE(AXP_IBOX_I_CTL, 8, passed);
    PRINT_SIZE(AXP_IBOX_I_STAT, 8, passed);
    PRINT_SIZE(AXP_IBOX_PCTX, 8, passed);
    PRINT_SIZE(AXP_IBOX_PCTR_CTL, 8, passed);
    PRINT_SIZE(AXP_MBOX_DTB_TAG, 8, passed);
    PRINT_SIZE(AXP_MBOX_DTB_PTE, 8, passed);
    PRINT_SIZE(AXP_MBOX_DTB_ALTMODE, 8, passed);
    PRINT_SIZE(AXP_MBOX_DTB_IS, 8, passed);
    PRINT_SIZE(AXP_MBOX_DTB_ASN, 8, passed);
    PRINT_SIZE(AXP_MBOX_MM_STAT, 8, passed);
    PRINT_SIZE(AXP_MBOX_M_CTL, 8, passed);
    PRINT_SIZE(AXP_MBOX_DC_CTL, 8, passed);
    PRINT_SIZE(AXP_MBOX_DC_STAT, 8, passed);
    PRINT_SIZE(AXP_CBOX_C_DATA, 8, passed);
    PRINT_SIZE(AXP_CBOX_C_SHFT, 8, passed);
    PRINT_SIZE(AXP_CBOX_READ_IPR, 16, passed);

    printf("\nAXP_21264_Cbox.h.h\n");
    PRINT_SIZE(AXP_21264_CBOX_CSRS, 40, passed);
    PRINT_SIZE(AXP_21264_CBOX_IOWB, 88, passed);

    printf("\nAXP_21264_Predictions.h\n");
    PRINT_SIZE(LCLindex, 8, passed);
    PRINT_SIZE(LPTIndex, 8, passed);

    printf("\nAXP_21264_CacheDefs.h\n");
    PRINT_SIZE(AXP_VA_SPE2, 8, passed);
    PRINT_SIZE(AXP_VA_SPE1, 8, passed);
    PRINT_SIZE(AXP_VA_SPE0, 8, passed);
    PRINT_SIZE(AXP_VA_SPE, 8, passed);
    PRINT_SIZE(AXP_DCACHE_BLK, 64, passed);
    PRINT_SIZE(AXP_ICACHE_BLK, 72, passed);
    PRINT_SIZE(AXP_CACHE_IDX, 8, passed);
    PRINT_SIZE(AXP_VA_FIELDS, 8, passed);
    PRINT_SIZE(AXP_VA, 8, passed);
    PRINT_SIZE(AXP_VPC_FIELDS, 8, passed);
    PRINT_SIZE(AXP_VPC, 8, passed);

    printf("\nAXP_21264_Fbox.h\n");
    PRINT_SIZE(AXP_FP_FUNC, 4, passed);

    printf("\nAXP_VHDX.h\n");
    PRINT_SIZE(AXP_VHDX_GUID, 16, passed);
    PRINT_SIZE(AXP_VHDX_ID, 520, passed);
    PRINT_SIZE(AXP_VHDX_HDR, 4096, passed);
    PRINT_SIZE(AXP_VHDX_REG_HDR, 16, passed);
    PRINT_SIZE(AXP_VHDX_REG_ENT, 32, passed);
    PRINT_SIZE(AXP_VHDX_LOG_HDR, 64, passed);
    PRINT_SIZE(AXP_VHDX_ZERO_DSC, 32, passed);
    PRINT_SIZE(AXP_VHDX_DATA_DSC, 32, passed);
    PRINT_SIZE(AXP_VHDX_LOG_DATA, 4096, passed);
    PRINT_SIZE(AXP_VHDX_BAT_ENT, 8, passed);
    PRINT_SIZE(AXP_VHDX_META_HDR, 32, passed);
    PRINT_SIZE(AXP_VHDX_META_ENT, 32, passed);
    PRINT_SIZE(AXP_VHDX_META_FILE, 8, passed);
    PRINT_SIZE(AXP_VHDX_META_DISK, 8, passed);
    PRINT_SIZE(AXP_VHDX_META_PAGE83, 16, passed);
    PRINT_SIZE(AXP_VHDX_META_SEC, 4, passed);
    PRINT_SIZE(AXP_VHDX_META_PAR_HDR, 24, passed);
    PRINT_SIZE(AXP_VHDX_META_PAR_ENT, 12, passed);

    printf("\nOverall Result: %s\n", (passed ? pass : fail));

    return(0);
}
