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
 *	This source file contains tests to verify that the structure definitions
 *	have a correct sizeof() result, for key structures.
 *
 * Revision History:
 *
 *	V01.000		10-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 *
 */

/*
 * Include comutl.
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Blocks.h"

/*
 * Include cpu
 */
#include "AXP_21264_Instructions.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_IPRs.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_ICache.h"
#include "AXP_21264_Ibox.h"
#include "AXP_21264_Cbox.h"
#include "AXP_21264_CPU.h"

#define PRINT_SIZE(type, size) \
	printf("%-25s= %3lu (%3d): %s\n", #type, sizeof(type), size, (sizeof(type) == size ? pass : fail))

/*
 * main
 * 	This function is called to verify that certain critical data strcutures are
 * 	defined properly by size.
 *
 * Input Parameters:
 * 	None.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	Success(0).
 */
int main()
{
	char *pass = "passed";
	char *fail = "failed";

	printf("Alpha AXP 21264 Data Structure Size Test.\n\n");
	printf("AXP_Utility.h\n");
	PRINT_SIZE(u8, 1);
	PRINT_SIZE(u16, 2);
	PRINT_SIZE(u32, 4);
	PRINT_SIZE(u64, 8);
	PRINT_SIZE(i8, 1);
	PRINT_SIZE(i16, 2);
	PRINT_SIZE(i32, 4);
	PRINT_SIZE(i64, 8);

	printf("\nAXP_21264_Instructions.h\n");
	PRINT_SIZE(AXP_MEM_INS, 4);
	PRINT_SIZE(AXP_BR_INS, 4);
	PRINT_SIZE(AXP_OP1_INS, 4);
	PRINT_SIZE(AXP_OP2_INS, 4);
	PRINT_SIZE(AXP_FP_INS, 4);
	PRINT_SIZE(AXP_PAL_INS, 4);
	PRINT_SIZE(AXP_HW_LD, 4);
	PRINT_SIZE(AXP_HW_ST, 4);
	PRINT_SIZE(AXP_HW_RET, 4);
	PRINT_SIZE(AXP_HW_MXPR, 4);
	PRINT_SIZE(AXP_INS_FMT, 4);

	printf("\nAXP_Base_CPU.h\n");
	PRINT_SIZE(AXP_PC, 8);
	PRINT_SIZE(AXP_PTE, 8);
	PRINT_SIZE(AXP_BASE_ASN, 8);
	PRINT_SIZE(AXP_BASE_ASTEN, 8);
	PRINT_SIZE(AXP_BASE_ASTEN_R16, 8);
	PRINT_SIZE(AXP_BASE_ASTSR, 8);
	PRINT_SIZE(AXP_BASE_ASTSR_R16, 8);
	PRINT_SIZE(AXP_BASE_DATFX, 8);
	PRINT_SIZE(AXP_BASE_ESP, 8);
	PRINT_SIZE(AXP_BASE_FEN, 8);
	PRINT_SIZE(AXP_BASE_IPL, 8);
	PRINT_SIZE(AXP_BASE_KSP, 8);
	PRINT_SIZE(AXP_BASE_MCES, 8);
	PRINT_SIZE(AXP_BASE_PCBB, 8);
	PRINT_SIZE(AXP_BASE_PRBR, 8);
	PRINT_SIZE(AXP_BASE_PTBR, 8);
	PRINT_SIZE(AXP_BASE_SCBB, 8);
	PRINT_SIZE(AXP_BASE_SIRR, 8);
	PRINT_SIZE(AXP_BASE_SISR, 8);
	PRINT_SIZE(AXP_BASE_SSP, 8);
	PRINT_SIZE(AXP_BASE_SYSPTBR, 8);
	PRINT_SIZE(AXP_BASE_TBCHK, 8);
	PRINT_SIZE(AXP_BASE_TBCHK_R16, 8);
	PRINT_SIZE(AXP_BASE_USP, 8);
	PRINT_SIZE(AXP_BASE_VIRBND, 8);
	PRINT_SIZE(AXP_BASE_VPTB, 8);
	PRINT_SIZE(AXP_BASE_WHAMI, 8);

	printf("\nAXP_21264_IPRs.h\n");
	PRINT_SIZE(AXP_EBOX_CC, 8);
	PRINT_SIZE(AXP_EBOX_CC_CTL, 8);
	PRINT_SIZE(AXP_EBOX_VA, 8);
	PRINT_SIZE(AXP_EBOX_VA_CTL, 8);
	PRINT_SIZE(AXP_EBOX_VA_FORM_00, 8);
	PRINT_SIZE(AXP_EBOX_VA_FORM_01, 8);
	PRINT_SIZE(AXP_EBOX_VA_FORM_10, 8);
	PRINT_SIZE(AXP_EBOX_VA_FORM, 8);
	PRINT_SIZE(AXP_FBOX_FPCR, 8);
	PRINT_SIZE(AXP_IBOX_ITB_TAG, 8);
	PRINT_SIZE(AXP_IBOX_ITB_PTE, 8);
	PRINT_SIZE(AXP_IBOX_ITB_IS, 8);
	PRINT_SIZE(AXP_IBOX_EXC_ADDR, 8);
	PRINT_SIZE(AXP_IBOX_IVA_FORM_00, 8);
	PRINT_SIZE(AXP_IBOX_IVA_FORM_10, 8);
	PRINT_SIZE(AXP_IBOX_IVA_FORM_01, 8);
	PRINT_SIZE(AXP_IBOX_IVA_FORM, 8);
	PRINT_SIZE(AXP_IBOX_IER_CM, 8);
	PRINT_SIZE(AXP_IBOX_SIRR, 8);
	PRINT_SIZE(AXP_IBOX_ISUM, 8);
	PRINT_SIZE(AXP_IBOX_HW_INT_CLR, 8);
	PRINT_SIZE(AXP_IBOX_EXC_SUM, 8);
	PRINT_SIZE(AXP_IBOX_PAL_BASE, 8);
	PRINT_SIZE(AXP_IBOX_I_CTL, 8);
	PRINT_SIZE(AXP_IBOX_I_STAT, 8);
	PRINT_SIZE(AXP_IBOX_PCTX, 8);
	PRINT_SIZE(AXP_IBOX_PCTR_CTL, 8);
	PRINT_SIZE(AXP_MBOX_DTB_TAG, 8);
	PRINT_SIZE(AXP_MBOX_DTB_PTE, 8);
	PRINT_SIZE(AXP_MBOX_DTB_ALTMODE, 8);
	PRINT_SIZE(AXP_MBOX_DTB_IS, 8);
	PRINT_SIZE(AXP_MBOX_DTB_ASN, 8);
	PRINT_SIZE(AXP_MBOX_MM_STAT, 8);
	PRINT_SIZE(AXP_MBOX_M_CTL, 8);
	PRINT_SIZE(AXP_MBOX_DC_CTL, 8);
	PRINT_SIZE(AXP_MBOX_DC_STAT, 8);
	PRINT_SIZE(AXP_CBOX_C_DATA, 8);
	PRINT_SIZE(AXP_CBOX_C_SHFT, 8);

	printf("\nAXP_21264_Predictions.h\n");
	PRINT_SIZE(LCLindex, 8);
	PRINT_SIZE(LPTIndex, 8);

	printf("\nAXP_21264_ICache.h\n");
	PRINT_SIZE(AXP_ICACHE_VPC, 8);
	PRINT_SIZE(AXP_ICACHE_TAG_IDX, 8);
	PRINT_SIZE(AXP_ICACHE_LINE, 128);

	return(0);
}
