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
 *	This source file contains the functions needed to dump various information
 *	for the Alpha AXP Emulator.
 *
 *	Revision History:
 *
 *	V01.000		04-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		06-Nov-2017	Jonathan D. Belanger
 *	Included just a single file, that will contain all the includes needed
 *	to successfully compile this file.
 */
#include "AXP_DumpsDefs.h"

typedef struct
{
	const u16	func;
	const char	*cmd;
} AXP_FUNC_CMD;

static const char *instrCmd[] =
{
	"CALL_PAL",
	"OPC01",
	"OPC02",
	"OPC03",
	"OPC04",
	"OPC05",
	"OPC06",
	"OPC07",
	"LDA",
	"LDAH",
	"LDBU",
	"LDQ_U",
	"LWW_U",
	"STW",
	"STB",
	"STQ_U",
	"INTA",
	"INTL",
	"INTS",
	"INTM",
	"ITFP",
	"FLTV",
	"FLTI",
	"FLTL",
	"MISC",
	"HW_MFPR",
	"JSR",
	"HW_LD",
	"FPTI",
	"HW_MTPR",
	"HW_RET",
	"HW_ST",
	"LDF",
	"LDG",
	"LDS",
	"LDT",
	"STF",
	"STG",
	"STS",
	"STT",
	"LDL",
	"LDQ",
	"LDL_L",
	"LDQ_L",
	"STL",
	"STQ",
	"STL_C",
	"STQ_C",
	"BR",
	"FBEQ",
	"FBLT",
	"FBLE",
	"BSR",
	"FBNE",
	"FBGE",
	"FBGT",
	"BLBC",
	"BEQ",
	"BLT",
	"BLE",
	"BLBS",
	"BNE",
	"BGE",
	"BGT"
};
const char *jmpCmd[] =
{
	"JMP",
	"JSR",
	"RET",
	"JSR_COROUTINE"
};

static const AXP_FUNC_CMD instaCmd[] =
{
	{AXP_FUNC_ADDL, "ADDL"},
	{AXP_FUNC_S4ADDL, "S4ADDL"},
	{AXP_FUNC_SUBL, "SUBL"},
	{AXP_FUNC_S4SUBL, "S4SUBL"},
	{AXP_FUNC_CMPBGE, "CMPBGE"},
	{AXP_FUNC_S8ADDL, "S8ADDL"},
	{AXP_FUNC_S8SUBL, "S8SUBL"},
	{AXP_FUNC_CMPULT, "CMPULT"},
	{AXP_FUNC_ADDQ, "ADDQ"},
	{AXP_FUNC_S4ADDQ, "S4ADDQ"},
	{AXP_FUNC_SUBQ, "SUBQ"},
	{AXP_FUNC_S4SUBQ, "S4SUBQ"},
	{AXP_FUNC_CMPEQ, "CMPEQ"},
	{AXP_FUNC_S8ADDQ, "S8ADDQ"},
	{AXP_FUNC_S8SUBQ, "S8SUBQ"},
	{AXP_FUNC_CMPULE, "CMPULE"},
	{AXP_FUNC_ADDL_V, "ADDL/V"},
	{AXP_FUNC_SUBL_V, "SUBL/V"},
	{AXP_FUNC_CMPLT, "CMPLT"},
	{AXP_FUNC_ADDQ_V, "ADDQ/V"},
	{AXP_FUNC_SUBQ_V, "SUBQ/V"},
	{AXP_FUNC_CMPLE, "CMPLE"},
	{0, NULL}
};

static const AXP_FUNC_CMD instlCmd[] =
{
	{AXP_FUNC_AND, "AND"},
	{AXP_FUNC_BIC, "BIC"},
	{AXP_FUNC_CMOVLBS, "CMOVLBS"},
	{AXP_FUNC_CMOVLBC, "CMOVLBC"},
	{AXP_FUNC_BIS, "BIS"},
	{AXP_FUNC_CMOVEQ, "CMOVEQ"},
	{AXP_FUNC_CMOVNE, "CMOVNE"},
	{AXP_FUNC_ORNOT, "ORNOT"},
	{AXP_FUNC_XOR, "XOR"},
	{AXP_FUNC_CMOVLT, "CMOVLT"},
	{AXP_FUNC_CMOVGE, "CMOVGE"},
	{AXP_FUNC_EQV, "EQV"},
	{AXP_FUNC_AMASK, "AMASK"},
	{AXP_FUNC_CMOVLE, "CMOVLE"},
	{AXP_FUNC_CMOVGT, "CMOVGT"},
	{AXP_FUNC_IMPLVER, "IMPLVER"},
	{0, NULL}
};

static const AXP_FUNC_CMD instsCmd[] =
{
	{AXP_FUNC_MSKBL, "MSKBL"},
	{AXP_FUNC_EXTBL, "EXTBL"},
	{AXP_FUNC_INSBL, "INSBL"},
	{AXP_FUNC_MSKWL, "MSKWL"},
	{AXP_FUNC_EXTWL, "EXTWL"},
	{AXP_FUNC_INSWL, "INSWL"},
	{AXP_FUNC_MSKLL, "MSKLL"},
	{AXP_FUNC_EXTLL, "EXTLL"},
	{AXP_FUNC_INSLL, "INSLL"},
	{AXP_FUNC_ZAP, "ZAP"},
	{AXP_FUNC_ZAPNOT, "ZAPNOT"},
	{AXP_FUNC_MSKQL, "MSKQL"},
	{AXP_FUNC_SRL, "SRL"},
	{AXP_FUNC_EXTQL, "EXTQL"},
	{AXP_FUNC_SLL, "SLL"},
	{AXP_FUNC_INSQL, "INSQL"},
	{AXP_FUNC_SRA, "SRA"},
	{AXP_FUNC_MSKWH, "MSKWH"},
	{AXP_FUNC_INSWH, "INSWH"},
	{AXP_FUNC_EXTWH, "EXTWH"},
	{AXP_FUNC_MSKLH, "MSKLH"},
	{AXP_FUNC_INSLH, "INSLH"},
	{AXP_FUNC_EXTLH, "EXTLH"},
	{AXP_FUNC_MSKQH, "MSKQH"},
	{AXP_FUNC_INSQH, "INSQH"},
	{AXP_FUNC_EXTQH, "EXTQH"},
	{0, NULL}
};

static const AXP_FUNC_CMD instmCmd[] =
{
	{AXP_FUNC_MULL, "MULL"},
	{AXP_FUNC_MULQ, "MULQ"},
	{AXP_FUNC_UMULH, "UMULH"},
	{AXP_FUNC_MULL_V, "MULL/V"},
	{AXP_FUNC_MULQ_V, "MULQ/V"},
	{0, NULL}
};

static const AXP_FUNC_CMD itfpCmd[] =
{
	{AXP_FUNC_ITOFS, "ITOFS"},
	{AXP_FUNC_SQRTF_C, "SQRTF/C"},
	{AXP_FUNC_SQRTS_C, "SQRTS/C"},
	{AXP_FUNC_ITOFF, "ITOFF"},
	{AXP_FUNC_ITOFT, "ITOFT"},
	{AXP_FUNC_SQRTG_C, "SQRTG/C"},
	{AXP_FUNC_SQRTT_C, "SQRTT/C"},
	{AXP_FUNC_SQRTS_M, "SQRTS/M"},
	{AXP_FUNC_SQRTT_M, "SQRTT/M"},
	{AXP_FUNC_SQRTF, "SQRTF"},
	{AXP_FUNC_SQRTS, "SQRTS"},
	{AXP_FUNC_SQRTG, "SQRTG"},
	{AXP_FUNC_SQRTT, "SQRTT"},
	{AXP_FUNC_SQRTS_D, "SQRTS/D"},
	{AXP_FUNC_SQRTT_D, "SQRTT/D"},
	{AXP_FUNC_SQRTF_UC, "SQRTF/UC"},
	{AXP_FUNC_SQRTS_UC, "SQRTS/UC"},
	{AXP_FUNC_SQRTG_UC, "SQRTG/UC"},
	{AXP_FUNC_SQRTT_UC, "SQRTT/UC"},
	{AXP_FUNC_SQRTS_UM, "SQRTS/UM"},
	{AXP_FUNC_SQRTT_UM, "SQRTT/UM"},
	{AXP_FUNC_SQRTF_U, "SQRTF/U"},
	{AXP_FUNC_SQRTS_U, "SQRTS/U"},
	{AXP_FUNC_SQRTG_U, "SQRTG/U"},
	{AXP_FUNC_SQRTT_U, "SQRTT/U"},
	{AXP_FUNC_SQRTS_UD, "SQRTS/UD"},
	{AXP_FUNC_SQRTT_UD, "SQRTT/UD"},
	{AXP_FUNC_SQRTF_SC, "SQRTF/SC"},
	{AXP_FUNC_SQRTG_SC, "SQRTG/SC"},
	{AXP_FUNC_SQRTF_S, "SQRTF/S"},
	{AXP_FUNC_SQRTG_S, "SQRTG/S"},
	{AXP_FUNC_SQRTF_SUC, "SQRTF/SUC"},
	{AXP_FUNC_SQRTS_SUC, "SQRTS/SUC"},
	{AXP_FUNC_SQRTG_SUC, "SQRTG/SUC"},
	{AXP_FUNC_SQRTT_SUC, "SQRTT/SUC"},
	{AXP_FUNC_SQRTS_SUM, "SQRTS/SUM"},
	{AXP_FUNC_SQRTT_SUM, "SQRTT/SUM"},
	{AXP_FUNC_SQRTF_SU, "SQRTF/SU"},
	{AXP_FUNC_SQRTS_SU, "SQRTS/SU"},
	{AXP_FUNC_SQRTG_SU, "SQRTG/SU"},
	{AXP_FUNC_SQRTT_SU, "SQRTT/SU"},
	{AXP_FUNC_SQRTS_SUD, "SQRTS/SUD"},
	{AXP_FUNC_SQRTT_SUD, "SQRTT/SUD"},
	{AXP_FUNC_SQRTS_SUIC, "SQRTS/SUIC"},
	{AXP_FUNC_SQRTT_SUIC, "SQRTT/SUIC"},
	{AXP_FUNC_SQRTS_SUIM, "SQRTS/SUIM"},
	{AXP_FUNC_SQRTT_SUIM, "SQRTT/SUIM"},
	{AXP_FUNC_SQRTS_SUI, "SQRTS/SUI"},
	{AXP_FUNC_SQRTT_SUI, "SQRTT/SUI"},
	{AXP_FUNC_SQRTS_SUID, "SQRTS/SUID"},
	{AXP_FUNC_SQRTT_SUID, "SQRTT/SUID"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltvCmd[] =
{
	{AXP_FUNC_ADDF_C, "ADDF/C"},
	{AXP_FUNC_SUBF_C, "SUBF/C"},
	{AXP_FUNC_MULF_C, "MULF/C"},
	{AXP_FUNC_DIVF_C, "DIVF/C"},
	{AXP_FUNC_CVTDG_C, "CVTDG/C"},
	{AXP_FUNC_ADDG_C, "ADDG/C"},
	{AXP_FUNC_SUBG_C, "SUBG/C"},
	{AXP_FUNC_MULG_C, "MULG/C"},
	{AXP_FUNC_DIVG_C, "DIVG/C"},
	{AXP_FUNC_CVTGF_C, "CVTGF/C"},
	{AXP_FUNC_CVTGD_C, "CVTGD/C"},
	{AXP_FUNC_CVTGQ_C, "CVTGQ/C"},
	{AXP_FUNC_CVTQF_C, "CVTQF/C"},
	{AXP_FUNC_CVTQG_C, "CVTQG/C"},
	{AXP_FUNC_ADDF, "ADDF"},
	{AXP_FUNC_SUBF, "SUBF"},
	{AXP_FUNC_MULF, "MULF"},
	{AXP_FUNC_DIVF, "DIVF"},
	{AXP_FUNC_CVTDG, "CVTDG"},
	{AXP_FUNC_ADDG, "ADDG"},
	{AXP_FUNC_SUBG, "SUBG"},
	{AXP_FUNC_MULG, "MULG"},
	{AXP_FUNC_DIVG, "DIVG"},
	{AXP_FUNC_CMPGEQ, "CMPGEQ"},
	{AXP_FUNC_CMPGLT, "CMPGLT"},
	{AXP_FUNC_CMPGLE, "CMPGLE"},
	{AXP_FUNC_CVTGF, "CVTGF"},
	{AXP_FUNC_CVTGD, "CVTGD"},
	{AXP_FUNC_CVTGQ, "CVTGQ"},
	{AXP_FUNC_CVTQF, "CVTQF"},
	{AXP_FUNC_CVTQG, "CVTQG"},
	{AXP_FUNC_ADDF_UC, "ADDF/UC"},
	{AXP_FUNC_SUBF_UC, "SUBF/UC"},
	{AXP_FUNC_MULF_UC, "MULF/UC"},
	{AXP_FUNC_DIVF_UC, "DIVF/UC"},
	{AXP_FUNC_CVTDG_UC, "CVTDG/UC"},
	{AXP_FUNC_ADDG_UC, "ADDG/UC"},
	{AXP_FUNC_SUBG_UC, "SUBG/UC"},
	{AXP_FUNC_MULG_UC, "MULG/UC"},
	{AXP_FUNC_DIVG_UC, "DIVG/UC"},
	{AXP_FUNC_CVTGF_UC, "CVTGF/UC"},
	{AXP_FUNC_CVTGD_UC, "CVTGD/UC"},
	{AXP_FUNC_CVTGQ_VC, "CVTGQ/VC"},
	{AXP_FUNC_ADDF_U, "ADDF/U"},
	{AXP_FUNC_SUBF_U, "SUBF/U"},
	{AXP_FUNC_MULF_U, "MULF/U"},
	{AXP_FUNC_DIVF_U, "DIVF/U"},
	{AXP_FUNC_CVTDG_U, "CVTDG/U"},
	{AXP_FUNC_ADDG_U, "ADDG/U"},
	{AXP_FUNC_SUBG_U, "SUBG/U"},
	{AXP_FUNC_MULG_U, "MULG/U"},
	{AXP_FUNC_DIVG_U, "DIVG/U"},
	{AXP_FUNC_CVTGF_U, "CVTGF/U"},
	{AXP_FUNC_CVTGD_U, "CVTGD/U"},
	{AXP_FUNC_CVTGQ_V, "CVTGQ/V"},
	{AXP_FUNC_ADDF_SC, "ADDF/SC"},
	{AXP_FUNC_SUBF_SC, "SUBF/SC"},
	{AXP_FUNC_MULF_SC, "MULF/SC"},
	{AXP_FUNC_DIVF_SC, "DIVF/SC"},
	{AXP_FUNC_CVTDG_SC, "CVTDG/SC"},
	{AXP_FUNC_ADDG_SC, "ADDG/SC"},
	{AXP_FUNC_SUBG_SC, "SUBG/SC"},
	{AXP_FUNC_MULG_SC, "MULG/SC"},
	{AXP_FUNC_DIVG_SC, "DIVG/SC"},
	{AXP_FUNC_CVTGF_SC, "CVTGF/SC"},
	{AXP_FUNC_CVTGD_SC, "CVTGD/SC"},
	{AXP_FUNC_CVTGQ_SC, "CVTGQ/SC"},
	{AXP_FUNC_ADDF_S, "ADDF/S"},
	{AXP_FUNC_SUBF_S, "SUBF/S"},
	{AXP_FUNC_MULF_S, "MULF/S"},
	{AXP_FUNC_DIVF_S, "DIVF/S"},
	{AXP_FUNC_CVTDG_S, "CVTDG/S"},
	{AXP_FUNC_ADDG_S, "ADDG/S"},
	{AXP_FUNC_SUBG_S, "SUBG/S"},
	{AXP_FUNC_MULG_S, "MULG/S"},
	{AXP_FUNC_DIVG_S, "DIVG/S"},
	{AXP_FUNC_CMPGEQ_S, "CMPGEQ/S"},
	{AXP_FUNC_CMPGLT_S, "CMPGLT/S"},
	{AXP_FUNC_CMPGLE_S, "CMPGLE/S"},
	{AXP_FUNC_CVTGF_S, "CVTGF/S"},
	{AXP_FUNC_CVTGD_S, "CVTGD/S"},
	{AXP_FUNC_CVTGQ_S, "CVTGQ/S"},
	{AXP_FUNC_ADDF_SUC, "ADDF/SUC"},
	{AXP_FUNC_SUBF_SUC, "SUBF/SUC"},
	{AXP_FUNC_MULF_SUC, "MULF/SUC"},
	{AXP_FUNC_DIVF_SUC, "DIVF/SUC"},
	{AXP_FUNC_CVTDG_SUC, "CVTDG/SUC"},
	{AXP_FUNC_ADDG_SUC, "ADDG/SUC"},
	{AXP_FUNC_SUBG_SUC, "SUBG/SUC"},
	{AXP_FUNC_MULG_SUC, "MULG/SUC"},
	{AXP_FUNC_DIVG_SUC, "DIVG/SUC"},
	{AXP_FUNC_CVTGF_SUC, "CVTGF/SUC"},
	{AXP_FUNC_CVTGD_SUC, "CVTGD/SUC"},
	{AXP_FUNC_CVTGQ_SVC, "CVTGQ/SVC"},
	{AXP_FUNC_ADDF_SU, "ADDF/SU"},
	{AXP_FUNC_SUBF_SU, "SUBF/SU"},
	{AXP_FUNC_MULF_SU, "MULF/SU"},
	{AXP_FUNC_DIVF_SU, "DIVF/SU"},
	{AXP_FUNC_CVTDG_SU, "CVTDG/SU"},
	{AXP_FUNC_ADDG_SU, "ADDG/SU"},
	{AXP_FUNC_SUBG_SU, "SUBG/SU"},
	{AXP_FUNC_MULG_SU, "MULG/SU"},
	{AXP_FUNC_DIVG_SU, "DIVG/SU"},
	{AXP_FUNC_CVTGF_SU, "CVTGF/SU"},
	{AXP_FUNC_CVTGD_SU, "CVTGD/SU"},
	{AXP_FUNC_CVTGQ_SV, "CVTGQ/SV"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltiCmd[] =
{
	{AXP_FUNC_ADDS_C, "ADDS/C"},
	{AXP_FUNC_SUBS_C, "SUBS/C"},
	{AXP_FUNC_MULS_C, "MULS/C"},
	{AXP_FUNC_DIVS_C, "DIVS/C"},
	{AXP_FUNC_ADDT_C, "ADDT/C"},
	{AXP_FUNC_SUBT_C, "SUBT/C"},
	{AXP_FUNC_MULT_C, "MULT/C"},
	{AXP_FUNC_DIVT_C, "DIVT/C"},
	{AXP_FUNC_CVTTS_C, "CVTTS/C"},
	{AXP_FUNC_CVTTQ_C, "CVTTQ/C"},
	{AXP_FUNC_CVTQS_C, "CVTQS/C"},
	{AXP_FUNC_CVTQT_C, "CVTQT/C"},
	{AXP_FUNC_ADDS_M, "ADDS/M"},
	{AXP_FUNC_SUBS_M, "SUBS/M"},
	{AXP_FUNC_MULS_M, "MULS/M"},
	{AXP_FUNC_DIVS_M, "DIVS/M"},
	{AXP_FUNC_ADDT_M, "ADDT/M"},
	{AXP_FUNC_SUBT_M, "SUBT/M"},
	{AXP_FUNC_MULT_M, "MULT/M"},
	{AXP_FUNC_DIVT_M, "DIVT/M"},
	{AXP_FUNC_CVTTS_M, "CVTTS/M"},
	{AXP_FUNC_CVTTQ_M, "CVTTQ/M"},
	{AXP_FUNC_CVTQS_M, "CVTQS/M"},
	{AXP_FUNC_CVTQT_M, "CVTQT/M"},
	{AXP_FUNC_ADDS, "ADDS"},
	{AXP_FUNC_SUBS, "SUBS"},
	{AXP_FUNC_MULS, "MULS"},
	{AXP_FUNC_DIVS, "DIVS"},
	{AXP_FUNC_ADDT, "ADDT"},
	{AXP_FUNC_SUBT, "SUBT"},
	{AXP_FUNC_MULT, "MULT"},
	{AXP_FUNC_DIVT, "DIVT"},
	{AXP_FUNC_CMPTUN, "CMPTUN"},
	{AXP_FUNC_CMPTEQ, "CMPTEQ"},
	{AXP_FUNC_CMPTLT, "CMPTLT"},
	{AXP_FUNC_CMPTLE, "CMPTLE"},
	{AXP_FUNC_CVTTS, "CVTTS"},
	{AXP_FUNC_CVTTQ, "CVTTQ"},
	{AXP_FUNC_CVTQS, "CVTQS"},
	{AXP_FUNC_CVTQT, "CVTQT"},
	{AXP_FUNC_ADDS_D, "ADDS/D"},
	{AXP_FUNC_SUBS_D, "SUBS/D"},
	{AXP_FUNC_MULS_D, "MULS/D"},
	{AXP_FUNC_DIVS_D, "DIVS/D"},
	{AXP_FUNC_ADDT_D, "ADDT/D"},
	{AXP_FUNC_SUBT_D, "SUBT/D"},
	{AXP_FUNC_MULT_D, "MULT/D"},
	{AXP_FUNC_DIVT_D, "DIVT/D"},
	{AXP_FUNC_CVTTS_D, "CVTTS/D"},
	{AXP_FUNC_CVTTQ_D, "CVTTQ/D"},
	{AXP_FUNC_CVTQS_D, "CVTQS/D"},
	{AXP_FUNC_CVTQT_D, "CVTQT/D"},
	{AXP_FUNC_ADDS_UC, "ADDS/UC"},
	{AXP_FUNC_SUBS_UC, "SUBS/UC"},
	{AXP_FUNC_MULS_UC, "MULS/UC"},
	{AXP_FUNC_DIVS_UC, "DIVS/UC"},
	{AXP_FUNC_ADDT_UC, "ADDT/UC"},
	{AXP_FUNC_SUBT_UC, "SUBT/UC"},
	{AXP_FUNC_MULT_UC, "MULT/UC"},
	{AXP_FUNC_DIVT_UC, "DIVT/UC"},
	{AXP_FUNC_CVTTS_UC, "CVTTS/UC"},
	{AXP_FUNC_CVTTQ_VC, "CVTTQ/VC"},
	{AXP_FUNC_ADDS_UM, "ADDS/UM"},
	{AXP_FUNC_SUBS_UM, "SUBS/UM"},
	{AXP_FUNC_MULS_UM, "MULS/UM"},
	{AXP_FUNC_DIVS_UM, "DIVS/UM"},
	{AXP_FUNC_ADDT_UM, "ADDT/UM"},
	{AXP_FUNC_SUBT_UM, "SUBT/UM"},
	{AXP_FUNC_MULT_UM, "MULT/UM"},
	{AXP_FUNC_DIVT_UM, "DIVT/UM"},
	{AXP_FUNC_CVTTS_UM, "CVTTS/UM"},
	{AXP_FUNC_CVTTQ_VM, "CVTTQ/VM"},
	{AXP_FUNC_ADDS_U, "ADDS/U"},
	{AXP_FUNC_SUBS_U, "SUBS/U"},
	{AXP_FUNC_MULS_U, "MULS/U"},
	{AXP_FUNC_DIVS_U, "DIVS/U"},
	{AXP_FUNC_ADDT_U, "ADDT/U"},
	{AXP_FUNC_SUBT_U, "SUBT/U"},
	{AXP_FUNC_MULT_U, "MULT/U"},
	{AXP_FUNC_DIVT_U, "DIVT/U"},
	{AXP_FUNC_CVTTS_U, "CVTTS/U"},
	{AXP_FUNC_CVTTQ_V, "CVTTQ/V"},
	{AXP_FUNC_ADDS_UD, "ADDS/UD"},
	{AXP_FUNC_SUBS_UD, "SUBS/UD"},
	{AXP_FUNC_MULS_UD, "MULS/UD"},
	{AXP_FUNC_DIVS_UD, "DIVS/UD"},
	{AXP_FUNC_ADDT_UD, "ADDT/UD"},
	{AXP_FUNC_SUBT_UD, "SUBT/UD"},
	{AXP_FUNC_MULT_UD, "MULT/UD"},
	{AXP_FUNC_DIVT_UD, "DIVT/UD"},
	{AXP_FUNC_CVTTS_UD, "CVTTS/UD"},
	{AXP_FUNC_CVTTQ_VD, "CVTTQ/VD"},
	{AXP_FUNC_CVTST, "CVTST"},
	{AXP_FUNC_ADDS_SUC, "ADDS/SUC"},
	{AXP_FUNC_SUBS_SUC, "SUBS/SUC"},
	{AXP_FUNC_MULS_SUC, "MULS/SUC"},
	{AXP_FUNC_DIVS_SUC, "DIVS/SUC"},
	{AXP_FUNC_ADDT_SUC, "ADDT/SUC"},
	{AXP_FUNC_SUBT_SUC, "SUBT/SUC"},
	{AXP_FUNC_MULT_SUC, "MULT/SUC"},
	{AXP_FUNC_DIVT_SUC, "DIVT/SUC"},
	{AXP_FUNC_CVTTS_SUC, "CVTTS/SUC"},
	{AXP_FUNC_CVTTQ_SVC, "CVTTQ/SVC"},
	{AXP_FUNC_ADDS_SUM, "ADDS/SUM"},
	{AXP_FUNC_SUBS_SUM, "SUBS/SUM"},
	{AXP_FUNC_MULS_SUM, "MULS/SUM"},
	{AXP_FUNC_DIVS_SUM, "DIVS/SUM"},
	{AXP_FUNC_ADDT_SUM, "ADDT/SUM"},
	{AXP_FUNC_SUBT_SUM, "SUBT/SUM"},
	{AXP_FUNC_MULT_SUM, "MULT/SUM"},
	{AXP_FUNC_DIVT_SUM, "DIVT/SUM"},
	{AXP_FUNC_CVTTS_SUM, "CVTTS/SUM"},
	{AXP_FUNC_CVTTQ_SVM, "CVTTQ/SVM"},
	{AXP_FUNC_ADDS_SU, "ADDS/SU"},
	{AXP_FUNC_SUBS_SU, "SUBS/SU"},
	{AXP_FUNC_MULS_SU, "MULS/SU"},
	{AXP_FUNC_DIVS_SU, "DIVS/SU"},
	{AXP_FUNC_ADDT_SU, "ADDT/SU"},
	{AXP_FUNC_SUBT_SU, "SUBT/SU"},
	{AXP_FUNC_MULT_SU, "MULT/SU"},
	{AXP_FUNC_DIVT_SU, "DIVT/SU"},
	{AXP_FUNC_CMPTUN_SU, "CMPTUN/SU"},
	{AXP_FUNC_CMPTEQ_SU, "CMPTEQ/SU"},
	{AXP_FUNC_CMPTLT_SU, "CMPTLT/SU"},
	{AXP_FUNC_CMPTLE_SU, "CMPTLE/SU"},
	{AXP_FUNC_CVTTS_SU, "CVTTS/SU"},
	{AXP_FUNC_CVTTQ_SV, "CVTTQ/SV"},
	{AXP_FUNC_ADDS_SUD, "ADDS/SUD"},
	{AXP_FUNC_SUBS_SUD, "SUBS/SUD"},
	{AXP_FUNC_MULS_SUD, "MULS/SUD"},
	{AXP_FUNC_DIVS_SUD, "DIVS/SUD"},
	{AXP_FUNC_ADDT_SUD, "ADDT/SUD"},
	{AXP_FUNC_SUBT_SUD, "SUBT/SUD"},
	{AXP_FUNC_MULT_SUD, "MULT/SUD"},
	{AXP_FUNC_DIVT_SUD, "DIVT/SUD"},
	{AXP_FUNC_CVTTS_SUD, "CVTTS/SUD"},
	{AXP_FUNC_CVTTQ_SVD, "CVTTQ/SVD"},
	{AXP_FUNC_CVTST_S, "CVTST/S"},
	{AXP_FUNC_ADDS_SUIC, "ADDS/SUIC"},
	{AXP_FUNC_SUBS_SUIC, "SUBS/SUIC"},
	{AXP_FUNC_MULS_SUIC, "MULS/SUIC"},
	{AXP_FUNC_DIVS_SUIC, "DIVS/SUIC"},
	{AXP_FUNC_ADDT_SUIC, "ADDT/SUIC"},
	{AXP_FUNC_SUBT_SUIC, "SUBT/SUIC"},
	{AXP_FUNC_MULT_SUIC, "MULT/SUIC"},
	{AXP_FUNC_DIVT_SUIC, "DIVT/SUIC"},
	{AXP_FUNC_CVTTS_SUIC, "CVTTS/SUIC"},
	{AXP_FUNC_CVTTQ_SVIC, "CVTTQ/SVIC"},
	{AXP_FUNC_CVTQS_SUIC, "CVTQS/SUIC"},
	{AXP_FUNC_CVTQT_SUIC, "CVTQT/SUIC"},
	{AXP_FUNC_ADDS_SUIM, "ADDS/SUIM"},
	{AXP_FUNC_SUBS_SUIM, "SUBS/SUIM"},
	{AXP_FUNC_MULS_SUIM, "MULS/SUIM"},
	{AXP_FUNC_DIVS_SUIM, "DIVS/SUIM"},
	{AXP_FUNC_ADDT_SUIM, "ADDT/SUIM"},
	{AXP_FUNC_SUBT_SUIM, "SUBT/SUIM"},
	{AXP_FUNC_MULT_SUIM, "MULT/SUIM"},
	{AXP_FUNC_DIVT_SUIM, "DIVT/SUIM"},
	{AXP_FUNC_CVTTS_SUIM, "CVTTS/SUIM"},
	{AXP_FUNC_CVTTQ_SVIM, "CVTTQ/SVIM"},
	{AXP_FUNC_CVTQS_SUIM, "CVTQS/SUIM"},
	{AXP_FUNC_CVTQT_SUIM, "CVTQT/SUIM"},
	{AXP_FUNC_ADDS_SUI, "ADDS/SUI"},
	{AXP_FUNC_SUBS_SUI, "SUBS/SUI"},
	{AXP_FUNC_MULS_SUI, "MULS/SUI"},
	{AXP_FUNC_DIVS_SUI, "DIVS/SUI"},
	{AXP_FUNC_ADDT_SUI, "ADDT/SUI"},
	{AXP_FUNC_SUBT_SUI, "SUBT/SUI"},
	{AXP_FUNC_MULT_SUI, "MULT/SUI"},
	{AXP_FUNC_DIVT_SUI, "DIVT/SUI"},
	{AXP_FUNC_CVTTS_SUI, "CVTTS/SUI"},
	{AXP_FUNC_CVTTQ_SVI, "CVTTQ/SVI"},
	{AXP_FUNC_CVTQS_SUI, "CVTQS/SUI"},
	{AXP_FUNC_CVTQT_SUI, "CVTQT/SUI"},
	{AXP_FUNC_ADDS_SUID, "ADDS/SUID"},
	{AXP_FUNC_SUBS_SUID, "SUBS/SUID"},
	{AXP_FUNC_MULS_SUID, "MULS/SUID"},
	{AXP_FUNC_DIVS_SUID, "DIVS/SUID"},
	{AXP_FUNC_ADDT_SUID, "ADDT/SUID"},
	{AXP_FUNC_SUBT_SUID, "SUBT/SUID"},
	{AXP_FUNC_MULT_SUID, "MULT/SUID"},
	{AXP_FUNC_DIVT_SUID, "DIVT/SUID"},
	{AXP_FUNC_CVTTS_SUID, "CVTTS/SUID"},
	{AXP_FUNC_CVTTQ_SVID, "CVTTQ/SVID"},
	{AXP_FUNC_CVTQS_SUID, "CVTQS/SUID"},
	{AXP_FUNC_CVTQT_SUID, "CVTQT/SUID"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltlCmd[] =
{
	{AXP_FUNC_CVTLQ, "CVTLQ"},
	{AXP_FUNC_CPYS, "CPYS"},
	{AXP_FUNC_CPYSN, "CPYSN"},
	{AXP_FUNC_CPYSE, "CPYSE"},
	{AXP_FUNC_MT_FPCR, "MT_FPCR"},
	{AXP_FUNC_MF_FPCR, "MF_FPCR"},
	{AXP_FUNC_FCMOVEQ, "FCMOVEQ"},
	{AXP_FUNC_FCMOVNE, "FCMOVNE"},
	{AXP_FUNC_FCMOVLT, "FCMOVLT"},
	{AXP_FUNC_FCMOVGE, "FCMOVGE"},
	{AXP_FUNC_FCMOVLE, "FCMOVLE"},
	{AXP_FUNC_FCMOVGT, "FCMOVGT"},
	{AXP_FUNC_CVTQL, "CVTQL"},
	{AXP_FUNC_CVTQL_V, "CVTQL/V"},
	{AXP_FUNC_CVTQL_SV, "CVTQL/SV"},
	{0, NULL}
};

static const AXP_FUNC_CMD miscCmd[] =
{
	{AXP_FUNC_TRAPB, "TRAPB"},
	{AXP_FUNC_EXCB, "EXCB"},
	{AXP_FUNC_MB, "MB"},
	{AXP_FUNC_WMB, "WMB"},
	{AXP_FUNC_FETCH, "FETCH"},
	{AXP_FUNC_FETCH_M, "FETCH_M"},
	{AXP_FUNC_RPCC, "RPCC"},
	{AXP_FUNC_RC, "RC"},
	{AXP_FUNC_ECB, "ECB"},
	{AXP_FUNC_RS, "RS"},
	{AXP_FUNC_WH64, "WH64"},
	{AXP_FUNC_WH64EN, "WH64EN"},
	{0, NULL}
};

static const char *prefetchCmd[] =
{
	"PREFETCH",
	"PREFETCH_EN",
	"PREFETCH_M",
	"PREFETCH_MEN"
};
#define AXP_LDL_PREFETCH	0
#define AXP_LDQ_PREFETCH	1
#define AXP_LDS_PREFETCH	2
#define AXP_LDT_PREFETCH	3

static const AXP_FUNC_CMD palcodeCmd[] =
{
	{VMS_HALT,			"HALT - halt"},
	{VMS_CFLUSH,		"CFLUSH - cflush"},
	{VMS_DRAINA,		"DRAINA - draina"},
	{VMS_LDQP,			"LDQP"},
	{VMS_STQP,			"STQP"},
	{VMS_SWPCTX,		"SWPCTX"},
	{VMS_MFPR_ASN,		"MFPR_ASN"},
	{VMS_MTPR_ASTEN,	"MTPR_ASTEN"},
	{VMS_MTPR_ASTSR,	"MTPR_ASTSR"},
	{VMS_CSERVE,		"CSERVE - cserve"},
	{VMS_SWPPAL,		"SWPPAL - swppal"},
	{VMS_MFPR_FEN,		"MFPR_FEN"},
	{VMS_MTPR_FEN,		"MTPR_FEN"},
	{VMS_MTPR_IPIR,		"MTPR_IPIR - wripir"},
	{VMS_MFPR_IPL,		"MFPR_IPL"},
	{VMS_MTPR_IPL,		"MTPR_IPL"},
	{VMS_MFPR_MCES,		"MFPR_MCES - rdmces"},
	{VMS_MTPR_MCES,		"MTPR_MCES - wrmces"},
	{VMS_MFPR_PCBB,		"MFPR_PCBB"},
	{VMS_MFPR_PRBR,		"MFPR_PRBR"},
	{VMS_MTPR_PRBR,		"MTPR_PRBR"},
	{VMS_MFPR_PTBR,		"MFPR_PTBR"},
	{VMS_MFPR_SCBB,		"MFPR_SCBB"},
	{VMS_MTPR_SCBB,		"MTPR_SCBB"},
	{VMS_MTPR_SIRR,		"MTPR_SIRR"},
	{VMS_MFPR_SISR,		"MFPR_SISR"},
	{VMS_MFPR_TBCHK,	"MFPR_TBCHK"},
	{VMS_MTPR_TBIA,		"MTPR_TBIA"},
	{VMS_MTPR_TBIAP,	"MTPR_TBIAP"},
	{VMS_MTPR_TBIS,		"MTPR_TBIS"},
	{VMS_MFPR_ESP,		"MFPR_ESP"},
	{VMS_MTPR_ESP,		"MTPR_ESP"},
	{VMS_MFPR_SSP,		"MFPR_SSP"},
	{VMS_MTPR_SSP,		"MTPR_SSP"},
	{VMS_MFPR_USP,		"MFPR_USP"},
	{VMS_MTPR_USP,		"MTPR_USP"},
	{VMS_MTPR_TBISD,	"MTPR_TBISD"},
	{VMS_MTPR_TBISI,	"MTPR_TBISI"},
	{VMS_MFPR_ASTEN,	"MFPR_ASTEN"},
	{VMS_MFPR_ASTSR,	"MFPR_ASTSR"},
	{VMS_MFPR_VPTB,		"MFPR_VPTB"},
	{VMS_MTPR_VPTB,		"MTPR_VPTB"},
	{VMS_MTPR_PERFMON,	"MTPR_PERFMON - wrfen"},
	{OSF_WRVPTPTR,		"wrvptptr"},
	{VMS_MTPR_DATFX,	"MTPR_DATFX - wrasn"},
	{OSF_SWPCTX,		"swpctx"},
	{OSF_WRVAL,			"wrval"},
	{OSF_RDVAL,			"rdval"},
	{OSF_TBI,			"tbi"},
	{OSF_WRENT,			"wrent"},
	{OSF_SWPIPL,		"swpipl"},
	{OSF_RDPS,			"rdps"},
	{OSF_WRKGP,			"wrkgp"},
	{OSF_WRUSP,			"wrusp"},
	{OSF_WRPERFMON,		"wrperfmon"},
	{OSF_RDUSP,			"rdusp"},
	{OSF_WHAMI,			"whami"},
	{OSF_RETSYS,		"retsys"},
	{VMS_WTINT,			"WTINT - wtint"},
	{VMS_MFPR_WHAMI,	"MFPR_WHAMI - rti"},
	{VMS_BPT,			"BPT - bpt"},
	{VMS_BUGCHK,		"BUGCHK - bugchk"},
	{VMS_CHME,			"CHME"},
	{VMS_CHMK,			"CHMK - callsys"},
	{VMS_CHMS,			"CHMS"},
	{VMS_CHMU,			"CHMU"},
	{VMS_IMB,			"IMB - imb"},
	{VMS_INSQHIL,		"INSQHIL"},
	{VMS_INSQTIL,		"INSQTIL"},
	{VMS_INSQHIQ,		"INSQHIQ"},
	{VMS_INSQTIQ,		"INSQTIQ"},
	{VMS_INSQUEL,		"INSQUEL"},
	{VMS_INSQUEQ,		"INSQUEQ"},
	{VMS_INSQUEL_D,		"INSQUEL/D"},
	{VMS_INSQUEQ_D,		"INSQUEQ/D"},
	{VMS_PROBER,		"PROBER"},
	{VMS_PROBEW,		"PROBEW"},
	{VMS_RD_PS,			"RD_PS"},
	{VMS_REI,			"REI - urti"},
	{VMS_REMQHIL,		"REMQHIL"},
	{VMS_REMQTIL,		"REMQTIL"},
	{VMS_REMQHIQ,		"REMQHIQ"},
	{VMS_REMQTIQ,		"REMQTIQ"},
	{VMS_REMQUEL,		"REMQUEL"},
	{VMS_REMQUEQ,		"REMQUEQ"},
	{VMS_REMQUEL_D,		"REMQUEL/D"},
	{VMS_REMQUEQ_D,		"REMQUEQ/D"},
	{VMS_SWASTEN,		"SWASTEN"},
	{VMS_WR_PS_SW,		"WR_PS_SW"},
	{VMS_RSCC,			"RSCC"},
	{VMS_READ_UNQ,		"READ_UNQ - rduniue"},
	{VMS_WRITE_UNQ,		"WRITE_UNQ - wrunique"},
	{VMS_AMOVRR,		"AMOVRR"},
	{VMS_AMOVRM,		"AMOVRM"},
	{VMS_INSQHILR,		"INSQHILR"},
	{VMS_INSQTILR,		"INSQTILR"},
	{VMS_INSQHIQR,		"INSQHIQR"},
	{VMS_INSQTIQR,		"INSQTIQR"},
	{VMS_REMQHILR,		"REMQHILR"},
	{VMS_REMQTILR,		"REMQTILR"},
	{VMS_REMQHIQR,		"REMQHIQR"},
	{VMS_REMQTIQR,		"REMQTIQR"},
	{VMS_GENTRAP,		"GENTRAP - gentrap"},
	{VMS_CLRFEN,		"CLRFEN - clrfen"},
	{0, NULL}
};

static const AXP_FUNC_CMD fptiCmd[] =
{
	{AXP_FUNC_SEXTB, "SEXTB"},
	{AXP_FUNC_SEXTW, "SEXTW"},
	{AXP_FUNC_CTPOP, "CTPOP"},
	{AXP_FUNC_PERR, "PERR"},
	{AXP_FUNC_CTLZ, "CTLZ"},
	{AXP_FUNC_CTTZ, "CTTZ"},
	{AXP_FUNC_UNPKBW, "UNPKBW"},
	{AXP_FUNC_UNPKBL, "UNPKBL"},
	{AXP_FUNC_PKWB, "PKWB"},
	{AXP_FUNC_PKLB, "PKLB"},
	{AXP_FUNC_MINSB8, "MINSB8"},
	{AXP_FUNC_MINSW4, "MINSW4"},
	{AXP_FUNC_MINUB8, "MINUB8"},
	{AXP_FUNC_MINUW4, "MINUW4"},
	{AXP_FUNC_MAXUB8, "MAXUB8"},
	{AXP_FUNC_MAXUW4, "MAXUW4"},
	{AXP_FUNC_MAXSB8, "MAXSB8"},
	{AXP_FUNC_MAXSW4, "MAXSW4"},
	{AXP_FUNC_FTOIT, "FTOIT"},
	{AXP_FUNC_FTOIS, "FTOIS"},
	{0, NULL}
};

static const AXP_FUNC_CMD iprFunc[] =
{
	{AXP_IPR_ITB_TAG,		"ITB_TAG"},
	{AXP_IPR_ITB_PTE,		"ITB_PTE"},
	{AXP_IPR_ITB_IAP,		"ITB_IAP"},
	{AXP_IPR_ITB_IA,		"ITB_IA"},
	{AXP_IPR_ITB_IS,		"ITB_IS"},
	{AXP_IPR_EXC_ADDR,		"EXC_ADDR"},
	{AXP_IPR_IVA_FORM,		"IVA_FORM"},
	{AXP_IPR_CM,			"CM"},
	{AXP_IPR_IER,			"IER"},
	{AXP_IPR_IER_CM,		"IER_CM"},
	{AXP_IPR_SIRR,			"SIRR"},
	{AXP_IPR_ISUM,			"ISUM"},
	{AXP_IPR_HW_INT_CLR,	"HW_INT_CLR"},
	{AXP_IPR_EXC_SUM,		"EXC_SUM"},
	{AXP_IPR_PAL_BASE,		"PAL_BASE"},
	{AXP_IPR_I_CTL,			"I_CTL"},
	{AXP_IPR_IC_FLUSH_ASM,	"IC_FLUSH_ASM"},
	{AXP_IPR_IC_FLUSH,		"IC_FLUSH"},
	{AXP_IPR_PCTR_CTL,		"PCTR_CTL"},
	{AXP_IPR_CLR_MAP,		"CLR_MAP"},
	{AXP_IPR_I_STAT,		"I_STAT"},
	{AXP_IPR_SLEEP,			"SLEEP"},
	{AXP_IPR_DTB_TAG0,		"DTB_TAG0"},
	{AXP_IPR_DTB_PTE0,		"DTB_PTE0"},
	{AXP_IPR_DTB_IS0,		"DTB_IS0"},
	{AXP_IPR_DTB_ASN0,		"DTB_ASN0"},
	{AXP_IPR_DTB_ALTMODE,	"DTB_ALTMODE"},
	{AXP_IPR_MM_STAT,		"MM_STAT"},
	{AXP_IPR_M_CTL,			"M_CTL"},
	{AXP_IPR_DC_CTL,		"DC_CTL"},
	{AXP_IPR_DC_STAT,		"DC_STAT"},
	{AXP_IPR_C_DATA,		"C_DATA"},
	{AXP_IPR_C_SHFT,		"C_SHFT"},
	{AXP_IPR_PCXT0,			"PCXT0"},
	{AXP_IPR_PCXT1,			"PCXT1"},
	{AXP_IPR_DTB_TAG1,		"DTB_TAG1"},
	{AXP_IPR_DTB_PTE1,		"DTB_PTE1"},
	{AXP_IPR_DTB_IAP,		"DTB_IAP"},
	{AXP_IPR_DTB_IA,		"DTB_IA"},
	{AXP_IPR_DTB_IS1,		"DTB_IS1"},
	{AXP_IPR_DTB_ASN1,		"DTB_ASN1"},
	{AXP_IPR_CC,			"IPR_CC"},
	{AXP_IPR_CC_CTL,		"CC_CTL"},
	{AXP_IPR_VA,			"VA"},
	{AXP_IPR_VA_FORM,		"VA_FORM"},
	{AXP_IPR_VA_CTL,		"VA_CTL"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwLdCmd[] =
{
	{AXP_HW_LD_PHYS,		"PHYS"},
	{AXP_HW_LD_PHYS_LOCK,	"PHYS_LOCK"},
	{AXP_HW_LD_VPTE,		"VPTE"},
	{AXP_HW_LD_VIRT,		"VIRT"},
	{AXP_HW_LD_VIRT_WCHK,	"VIRT_WRCHK"},
	{AXP_HW_LD_VIRT_ALT,	"VIRT_ALT"},
	{AXP_HW_LD_VIRT_WALT,	"VIRT_WRCHK_ALT"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwStCmd[] =
{
	{AXP_HW_ST_PHYS,		"PHYS"},
	{AXP_HW_ST_PHYS_LOCK,	"PHYS_COND"},
	{AXP_HW_ST_VIRT,		"VIRT"},
	{AXP_HW_ST_VIRT_ALT,	"VIRT_ALT"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwLen[] =
{
	{AXP_HW_LONGWORD,	"LONG"},
	{AXP_HW_QUADWORD,	"QUAD"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwRet[] =
{
	{AXP_HW_JMP,			"HW_JMP"},
	{AXP_HW_JSR,			"HW_JSR"},
	{AXP_HW_RET,			"HW_RET"},
	{AXP_HW_COROUTINE,		"HW_COROUTINE"},
	{0, NULL},
};

const char *hwRetStall[] =
{
	"",
	", STALL"
};

#define INDEX_TO_BINARY(byte)	\
	((byte & 0x80) ? '1' : '0'),	\
	((byte & 0x40) ? '1' : '0'),	\
	((byte & 0x20) ? '1' : '0'),	\
	((byte & 0x10) ? '1' : '0'),	\
	((byte & 0x08) ? '1' : '0'),	\
	((byte & 0x04) ? '1' : '0'),	\
	((byte & 0x02) ? '1' : '0'),	\
	((byte & 0x01) ? '1' : '0')

const char regLetter[]	= {'R', 'F'};
#define AXP_REG_LETTER(reg)	(reg & AXP_REG_FP) == FP ? regLetter[1] : regLetter[0]

const char *lineFmt		= "0x%016llx: %-31s %-31s ; 0x%08x '%c%c%c%c'";
const char *Reg			= "R%02d";
const char *DispReg 	= "#%-d(R%02d)";
const char *Disp 		= "#%-d";
const char *Hint		= "%-d";
const char *Reg2		= "(R%02d)";
const char *Lit			= "#%-d";
const char *ZeroReg		= "0(R%02d)";
const char *Freg		= "F%02d";
const char *iprIdx		= "%c%c%c%c %c%c%c%c";
const char *Scbd		= "%c%c%c%c%c%c%c%c";
const char *Len			= "%-s";
const char *Stall		= "%-s";
const char *Pad			= "%#s";
const char *Regv		= "%c%02d=0x%016llx";
const char *Comma		= ", ";
const char *Colon		= ": ";
const char *Space		= " ";

/*
 * AXP_Get_Func_Str
 * 	This function is called with a Function Command structure and search for
 * 	the function value and return the associated string.
 *
 * Input Parameters:
 * 	axpFuncCmd:
 * 		The structure containing the function values and strings.  It ends with
 * 		a value of 0 and a null pointer.
 * 	funcVal:
 * 		The function value to look up.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	A pointer to the string containing the function name.
 */
const char *AXP_Get_Func_Str(const AXP_FUNC_CMD *axpFuncCmd, u16 funcVal)
{
	const char 	*retVal = NULL;
	int			ii;

	for (ii = 0; ((axpFuncCmd[ii].cmd != NULL) && (retVal == NULL)); ii++)
		if (axpFuncCmd[ii].func == funcVal)
			retVal = axpFuncCmd[ii].cmd;

	/*
	 * Return what we found back to the caller.
	 */
	return(retVal);
}

/*
 * AXP_Decode_Instruction
 * 	This function is called to decode a single instruction.
 *
 * Input Parameters:
 *	pcAddr:
 *		The value associated with the program counter (PC) for the instruction
 *		to be decoded.
 * 	instr:
 * 		The value of a single Digital Alpha AXP instruction to be decoded.
 * 	kernelMode:
 * 		A boolean indicating that the instruction can be decoded as if the
 * 		instruction was executing in Kernel mode.
 *
 * Output Parameters:
 * 	instrStr:
 * 		A string containing the decoded instruction.
 *
 * Return Value:
 *	None.
 */
void AXP_Decode_Instruction(
			AXP_PC		*pcAddr,
			AXP_INS_FMT	instr,
			bool 		kernelMode,
			char 		*instrStr)
{
	const char	*binFuncStr;
	const char	*funcName = NULL;
	const char	*iprName = NULL;
	char		regStr[24];
	char		funcStr[24];
	u16			index;
	int			strLoc;

	/*
	 * Start by parsing out the opcode for the instruction.  Depending upon
	 * this value, will determine how to parse the remaining instruction
	 * encoding.
	 */
	switch(instr.pal.opcode)
	{
		case PAL00:
			funcName = instrCmd[instr.pal.opcode];
			iprName = AXP_Get_Func_Str(palcodeCmd, instr.pal.palcode_func);
			if (iprName == NULL)
				funcName = NULL;
			else
			{
				sprintf(funcStr, "%s(%s)", funcName, iprName);
				funcName = funcStr;
				switch(instr.pal.palcode_func)
				{

					/*
					 * R16 is used.
					 */
					case VMS_BUGCHK:
					case VMS_CHME:
					case VMS_CHMK:
					case VMS_CHMS:
					case VMS_CHMU:
					case VMS_CSERVE:
					case VMS_GENTRAP:
					case VMS_WR_PS_SW:
					case VMS_WRITE_UNQ:
					case VMS_CFLUSH:
					case VMS_SWPCTX:
					case OSF_TBI:
					case OSF_WRKGP:
					case OSF_WRUSP:
					case OSF_WRVAL:
					case OSF_WRVPTPTR:
						sprintf(regStr, Reg, 16);
						break;

					/*
					 * R0 is used.
					 */
					case VMS_RD_PS:
					case VMS_RSCC:
					case VMS_READ_UNQ:
					case VMS_MFPR_ASN:
					case VMS_MFPR_ESP:
					case VMS_MFPR_FEN:
					case VMS_MFPR_IPL:
					case VMS_MFPR_MCES:
					case VMS_MFPR_PCBB:
					case VMS_MFPR_PRBR:
					case VMS_MFPR_PTBR:
					case VMS_MFPR_SCBB:
					case VMS_MFPR_SISR:
					case VMS_MFPR_SSP:
					case VMS_MFPR_TBCHK:
					case VMS_MFPR_USP:
					case VMS_MFPR_VPTB:
					case VMS_MFPR_WHAMI:
					case OSF_RDPS:
					case OSF_RDUSP:
					case OSF_RDVAL:
					case OSF_WHAMI:
						sprintf(regStr, Reg, 0);
						break;

					/*
					 * R16, R17, and R0 are used.
					 */
					case VMS_MTPR_ASTEN:
					case VMS_MTPR_ASTSR:
					case VMS_MTPR_DATFX:
					case VMS_MTPR_ESP:
					case VMS_MTPR_FEN:
					case VMS_MTPR_IPIR:
					case VMS_MTPR_IPL:
					case VMS_MTPR_MCES:
					case VMS_MTPR_PERFMON:
					case VMS_MTPR_PRBR:
					case VMS_MTPR_SCBB:
					case VMS_MTPR_SIRR:
					case VMS_MTPR_SSP:
					case VMS_MTPR_TBIA:
					case VMS_MTPR_TBIAP:
					case VMS_MTPR_TBIS:
					case VMS_MTPR_TBISD:
					case VMS_MTPR_TBISI:
					case VMS_MTPR_USP:
					case VMS_MTPR_VPTB:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 17);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 0);
						break;

					/*
					 * R16, R17, R18, and R0 are used.
					 */
					case VMS_PROBER:
					case VMS_PROBEW:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 17);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 18);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 0);
						break;

					/*
					 * R16 and R0 are used.
					 */
					case VMS_SWASTEN:
					case VMS_LDQP:
					case VMS_WTINT:
					case OSF_SWPCTX:
					case OSF_SWPIPL:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 0);
						break;

					/*
					 * R16, R17, and R0 are used.
					 */
					case VMS_INSQHIL:
					case VMS_INSQHILR:
					case VMS_INSQHIQ:
					case VMS_INSQHIQR:
					case VMS_INSQTIL:
					case VMS_INSQTILR:
					case VMS_INSQTIQ:
					case VMS_INSQTIQR:
					case VMS_INSQUEL:
					case VMS_INSQUEL_D:
					case VMS_INSQUEQ:
					case VMS_INSQUEQ_D:
					case OSF_WRPERFMON:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 17);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 0);
						break;

					/*
					 * R16, R0, and R1 are used.
					 */
					case VMS_REMQHIL:
					case VMS_REMQHILR:
					case VMS_REMQHIQ:
					case VMS_REMQHIQR:
					case VMS_REMQTIL:
					case VMS_REMQTILR:
					case VMS_REMQTIQ:
					case VMS_REMQTIQR:
					case VMS_REMQUEL:
					case VMS_REMQUEL_D:
					case VMS_REMQUEQ:
					case VMS_REMQUEQ_D:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 0);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 1);
						break;

					/*
					 * R16, R17, R18, R19, R20, and R21 are used.
					 */
					case VMS_AMOVRR:
					case VMS_AMOVRM:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 17);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 18);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 19);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 20);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 21);
						break;

					/*
					 * R16 and R17 are used.
					 */
					case VMS_STQP:
					case OSF_WRENT:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 17);
						break;

					/*
					 * R16, R17, R18, R19, R20, R21, and R0 are used.
					 */
					case VMS_SWPPAL:
						strLoc = sprintf(regStr, Reg, 16);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 17);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 18);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 19);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 20);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, 21);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, 0);
						break;

					/*
					 * No registers are used.
					 */
					default:
						regStr[0] = ' ';
						regStr[1] = '\0';
						break;
				}
			}
			break;

		case OPC01:
		case OPC02:
		case OPC03:
		case OPC04:
		case OPC05:
		case OPC06:
		case OPC07:
			break;

		case LDL:		/* PREFETCH */
		case LDQ:		/* PREFETCH_EN */
			if (instr.mem.ra == 31)
			{
				if (instr.mem.opcode == LDL)
					funcName = prefetchCmd[AXP_LDL_PREFETCH];
				else
					funcName = prefetchCmd[AXP_LDQ_PREFETCH];
			}
			else
				funcName = instrCmd[instr.mem.opcode];
			strLoc = sprintf(regStr, Reg, instr.mem.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], DispReg, instr.mem.mem.disp, instr.mem.rb);
			break;

		case LDA:
		case LDBU:
		case LDW_U:
		case LDQ_U:
		case LDL_L:
		case LDQ_L:
		case LDAH:
		case STB:
		case STW:
		case STL:
		case STQ:
		case STQ_U:
		case STL_C:
		case STQ_C:
			funcName = instrCmd[instr.mem.opcode];
			strLoc = sprintf(regStr, Reg, instr.mem.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], DispReg, instr.mem.mem.disp, instr.mem.rb);
			break;

		case INTA:
			funcName = AXP_Get_Func_Str(instaCmd, instr.oper1.func);
			if (funcName != NULL)
			{
				strLoc = sprintf(regStr, Reg, instr.oper1.ra);
				strLoc += sprintf(&regStr[strLoc], Comma);
				if (instr.oper1.fmt == 0)
					strLoc += sprintf(&regStr[strLoc], Reg, instr.oper1.rb);
				else
					strLoc += sprintf(&regStr[strLoc], Lit, instr.oper2.lit);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(&regStr[strLoc], Reg, instr.oper1.rc);
			}
			break;

		case INTL:
			funcName = AXP_Get_Func_Str(instlCmd, instr.oper1.func);
			if (funcName != NULL)
			{
				switch (instr.oper1.func)
				{
					case AXP_FUNC_AMASK:
						if (instr.oper1.fmt == 0)
							strLoc = sprintf(regStr, Reg, instr.oper1.rb);
						else
							strLoc = sprintf(regStr, Lit, instr.oper2.lit);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;

					case AXP_FUNC_IMPLVER:
						strLoc = 0;
						break;

					default:
						strLoc = sprintf(regStr, Reg, instr.oper1.ra);
						strLoc += sprintf(&regStr[strLoc], Comma);
						if (instr.oper1.fmt == 0)
							strLoc += sprintf(
										&regStr[strLoc],
										Reg,
										instr.oper1.rb);
						else
							strLoc += sprintf(
										&regStr[strLoc],
										Lit,
										instr.oper2.lit);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;
				}
				sprintf(&regStr[strLoc], Reg, instr.oper1.rc);
			}
			break;

		case INTS:
		case INTM:
			if (instr.oper1.opcode == INTS)
				funcName = AXP_Get_Func_Str(instsCmd, instr.oper1.func);
			else
				funcName = AXP_Get_Func_Str(instmCmd, instr.oper1.func);
			if (funcName != NULL)
			{
				strLoc = sprintf(regStr, Reg, instr.oper1.ra);
				strLoc += sprintf(&regStr[strLoc], Comma);
				if (instr.oper1.fmt == 0)
					strLoc += sprintf(&regStr[strLoc], Reg, instr.oper1.rb);
				else
					strLoc += sprintf(&regStr[strLoc], Lit, instr.oper2.lit);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(&regStr[strLoc], Reg, instr.oper2.rc);
			}
			break;

		case ITFP:
			funcName = AXP_Get_Func_Str(itfpCmd, instr.fp.func);
			strLoc = sprintf(regStr, Freg, instr.fp.fb);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], Freg, instr.fp.fc);
			break;

		case FLTV:
			funcName = AXP_Get_Func_Str(fltvCmd, instr.fp.func);
			if (funcName != NULL)
			{
				switch (instr.fp.func)
				{
					case AXP_FUNC_CVTDG_C:
					case AXP_FUNC_CVTGF_C:
					case AXP_FUNC_CVTGD_C:
					case AXP_FUNC_CVTGQ_C:
					case AXP_FUNC_CVTQF_C:
					case AXP_FUNC_CVTQG_C:
					case AXP_FUNC_CVTDG:
					case AXP_FUNC_CVTGF:
					case AXP_FUNC_CVTGD:
					case AXP_FUNC_CVTGQ:
					case AXP_FUNC_CVTQF:
					case AXP_FUNC_CVTQG:
					case AXP_FUNC_CVTDG_UC:
					case AXP_FUNC_CVTGF_UC:
					case AXP_FUNC_CVTGD_UC:
					case AXP_FUNC_CVTGQ_VC:
					case AXP_FUNC_CVTDG_U:
					case AXP_FUNC_CVTGF_U:
					case AXP_FUNC_CVTGD_U:
					case AXP_FUNC_CVTGQ_V:
					case AXP_FUNC_CVTDG_SC:
					case AXP_FUNC_CVTGF_SC:
					case AXP_FUNC_CVTGD_SC:
					case AXP_FUNC_CVTGQ_SC:
					case AXP_FUNC_CVTDG_S:
					case AXP_FUNC_CVTGF_S:
					case AXP_FUNC_CVTGD_S:
					case AXP_FUNC_CVTGQ_S:
					case AXP_FUNC_CVTDG_SUC:
					case AXP_FUNC_CVTGF_SUC:
					case AXP_FUNC_CVTGD_SUC:
					case AXP_FUNC_CVTGQ_SVC:
					case AXP_FUNC_CVTDG_SU:
					case AXP_FUNC_CVTGF_SU:
					case AXP_FUNC_CVTGD_SU:
					case AXP_FUNC_CVTGQ_SV:
						strLoc = 0;
						break;

					default:
						strLoc = sprintf(regStr, Freg, instr.fp.fa);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;
				}
				strLoc += sprintf(&regStr[strLoc], Freg, instr.fp.fb);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(&regStr[strLoc], Freg, instr.fp.fc);
			}
			break;

		case FLTI:
			funcName = AXP_Get_Func_Str(fltiCmd, instr.fp.func);
			if (funcName != NULL)
			{
				switch(instr.fp.func)
				{
					case AXP_FUNC_CVTTS_UM:
					case AXP_FUNC_CVTTQ_VM:
					case AXP_FUNC_CVTTS_U:
					case AXP_FUNC_CVTTQ_V:
					case AXP_FUNC_CVTTS_UD:
					case AXP_FUNC_CVTTQ_VD:
					case AXP_FUNC_CVTST:
					case AXP_FUNC_CVTTS_SUC:
					case AXP_FUNC_CVTTQ_SVC:
					case AXP_FUNC_CVTTS_SUM:
					case AXP_FUNC_CVTTQ_SVM:
					case AXP_FUNC_CVTTS_SU:
					case AXP_FUNC_CVTTQ_SV:
					case AXP_FUNC_CVTTS_SUD:
					case AXP_FUNC_CVTTQ_SVD:
					case AXP_FUNC_CVTST_S:
					case AXP_FUNC_CVTTS_SUIC:
					case AXP_FUNC_CVTTQ_SVIC:
					case AXP_FUNC_CVTQS_SUIC:
					case AXP_FUNC_CVTQT_SUIC:
					case AXP_FUNC_CVTTS_SUIM:
					case AXP_FUNC_CVTTQ_SVIM:
					case AXP_FUNC_CVTQS_SUIM:
					case AXP_FUNC_CVTQT_SUIM:
					case AXP_FUNC_CVTTS_SUI:
					case AXP_FUNC_CVTTQ_SVI:
					case AXP_FUNC_CVTQS_SUI:
					case AXP_FUNC_CVTQT_SUI:
					case AXP_FUNC_CVTTS_SUID:
					case AXP_FUNC_CVTTQ_SVID:
					case AXP_FUNC_CVTQS_SUID:
					case AXP_FUNC_CVTQT_SUID:
						strLoc = 0;
						break;

					default:
						strLoc = sprintf(regStr, Freg, instr.fp.fa);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;
				}
				strLoc += sprintf(&regStr[strLoc], Freg, instr.fp.fb);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(&regStr[strLoc], Freg, instr.fp.fc);
			}
			break;

		case FLTL:
			funcName = AXP_Get_Func_Str(fltlCmd, instr.fp.func);
			if (funcName != NULL)
			{
				switch(instr.fp.func)
				{
					case AXP_FUNC_CVTLQ:
					case AXP_FUNC_CVTQL:
					case AXP_FUNC_CVTQL_V:
					case AXP_FUNC_CVTQL_SV:
						strLoc = 0;
						break;

					default:
						strLoc = sprintf(regStr, Freg, instr.fp.fa);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;
				}
				strLoc += sprintf(&regStr[strLoc], Freg, instr.fp.fb);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(&regStr[strLoc], Freg, instr.fp.fc);
			}
			break;

		case MISC:
			funcName = AXP_Get_Func_Str(miscCmd, instr.mem.mem.func);
			if (funcName != NULL)
			{
				switch (instr.mem.mem.func)
				{
					case AXP_FUNC_ECB:
					case AXP_FUNC_WH64:
					case AXP_FUNC_WH64EN:
						sprintf(regStr, Reg, instr.mem.rb);
						break;

					case AXP_FUNC_EXCB:
					case AXP_FUNC_MB:
					case AXP_FUNC_TRAPB:
					case AXP_FUNC_WMB:
						regStr[0] = ' ';
						regStr[1] = '\0';
						break;

					case AXP_FUNC_FETCH:
					case AXP_FUNC_FETCH_M:
						sprintf(regStr, ZeroReg, instr.mem.rb);
						break;

					case AXP_FUNC_RPCC:
						strLoc = sprintf(regStr, Reg, instr.mem.ra);
						strLoc += sprintf(&regStr[strLoc], Comma);
						sprintf(&regStr[strLoc], Reg, instr.mem.rb);
						break;

					case AXP_FUNC_RC:
					case AXP_FUNC_RS:
						sprintf(regStr, Reg, instr.oper1.ra);
						break;
				}
			}
			break;

		case FPTI:
			funcName = AXP_Get_Func_Str(fptiCmd, instr.oper1.func);
			if (funcName != NULL)
			{
				switch (instr.oper1.func)
				{
					case AXP_FUNC_SEXTB:
					case AXP_FUNC_SEXTW:
						if (instr.oper1.fmt == 0)
							strLoc = sprintf(regStr, Reg, instr.oper1.rb);
						else
							strLoc = sprintf(regStr, Lit, instr.oper2.lit);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;

					case AXP_FUNC_CTPOP:
					case AXP_FUNC_CTLZ:
					case AXP_FUNC_CTTZ:
					case AXP_FUNC_PKLB:
					case AXP_FUNC_PKWB:
					case AXP_FUNC_UNPKBL:
					case AXP_FUNC_UNPKBW:
						strLoc = sprintf(regStr, Reg, instr.oper1.rb);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;

					case AXP_FUNC_PERR:
						strLoc = sprintf(regStr, Reg, instr.oper1.ra);
						strLoc += sprintf(&regStr[strLoc], Comma);
						strLoc += sprintf(&regStr[strLoc], Reg, instr.oper1.rb);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;

					case AXP_FUNC_FTOIS:
					case AXP_FUNC_FTOIT:
						strLoc = sprintf(regStr, Freg, instr.fp.fa);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;

					case AXP_FUNC_MINUB8:
					case AXP_FUNC_MINSB8:
					case AXP_FUNC_MINUW4:
					case AXP_FUNC_MINSW4:
					case AXP_FUNC_MAXUB8:
					case AXP_FUNC_MAXSB8:
					case AXP_FUNC_MAXUW4:
					case AXP_FUNC_MAXSW4:
						strLoc = sprintf(regStr, Reg, instr.oper1.ra);
						strLoc += sprintf(&regStr[strLoc], Comma);
						if (instr.oper1.fmt == 0)
							strLoc += sprintf(
										&regStr[strLoc],
										Reg,
										instr.oper1.rb);
						else
							strLoc += sprintf(
										&regStr[strLoc],
										Lit,
										instr.oper2.lit);
						strLoc += sprintf(&regStr[strLoc], Comma);
						break;
				}
				sprintf(&regStr[strLoc], Reg, instr.oper2.rc);
			}
			break;

		case HW_MFPR:
		case HW_MTPR:
			if ((pcAddr->pal == 0) && (kernelMode == false))
				funcName = NULL;
			else
			{
				funcName = instrCmd[instr.hw_mxpr.opcode];
				if ((instr.hw_mxpr.index & AXP_IPR_CC) == AXP_IPR_PCXT0)
					index = instr.hw_mxpr.index & AXP_IPR_PCXT1;
				else
					index = instr.hw_mxpr.index;
				iprName = AXP_Get_Func_Str(iprFunc, index);
				if (iprName != NULL)
				{
					sprintf(funcStr, "%s(%s)", funcName, iprName);
					funcName = funcStr;
					if (instr.hw_mxpr.opcode == HW_MFPR)
						strLoc = sprintf(regStr, Reg, instr.hw_mxpr.ra);
					else
						strLoc = sprintf(regStr, Reg, instr.hw_mxpr.rb);
					strLoc += sprintf(&regStr[strLoc], Comma);
					strLoc += sprintf(
								&regStr[strLoc],
								iprIdx,
								INDEX_TO_BINARY(instr.hw_mxpr.index));
					strLoc += sprintf(&regStr[strLoc], Comma);
					sprintf(
						&regStr[strLoc],
						Scbd,
						INDEX_TO_BINARY(instr.hw_mxpr.scbd_mask));
				}
				else
					funcName = NULL;
			}
			break;

		case HW_LD:
			if ((pcAddr->pal == 0) && (kernelMode == false))
				funcName = NULL;
			else
			{
				funcName = instrCmd[instr.hw_ld.opcode];
				iprName = AXP_Get_Func_Str(hwLdCmd, instr.hw_ld.type);
				if (iprName != NULL)
				{
					sprintf(funcStr, "%s(%s)", funcName, iprName);
					funcName = funcStr;
					strLoc = sprintf(regStr, Reg, instr.hw_ld.ra);
					strLoc += sprintf(&regStr[strLoc], Comma);
					strLoc += sprintf(&regStr[strLoc], Disp, instr.hw_ld.disp);
					strLoc += sprintf(&regStr[strLoc], Comma);
					strLoc += sprintf(&regStr[strLoc], Reg, instr.hw_ld.rb);
					strLoc += sprintf(&regStr[strLoc], Comma);
					sprintf(
						&regStr[strLoc],
						Len,
						AXP_Get_Func_Str(hwLen, instr.hw_ld.len));
				}
				else
					funcName = NULL;
			}
			break;

		case HW_ST:
			if ((pcAddr->pal == 0) && (kernelMode == false))
				funcName = NULL;
			else
			{
				funcName = instrCmd[instr.hw_st.opcode];
				iprName = AXP_Get_Func_Str(hwStCmd, instr.hw_st.type);
				if (iprName != NULL)
				{
					sprintf(funcStr, "%s(%s)", funcName, iprName);
					funcName = funcStr;
					strLoc = sprintf(regStr, Reg, instr.hw_st.ra);
					strLoc += sprintf(&regStr[strLoc], Comma);
					strLoc += sprintf(&regStr[strLoc], Disp, instr.hw_st.disp);
					strLoc += sprintf(&regStr[strLoc], Comma);
					strLoc += sprintf(&regStr[strLoc], Reg, instr.hw_st.rb);
					strLoc += sprintf(&regStr[strLoc], Comma);
					sprintf(
						&instrStr[strLoc],
						Len,
						AXP_Get_Func_Str(hwLen, instr.hw_st.len));
				}
				else
					funcName = NULL;
			}
			break;

		case HW_RET:
			if ((pcAddr->pal == 0) && (kernelMode == false))
				funcName = NULL;
			else
			{
				funcName = AXP_Get_Func_Str(hwRet, instr.hw_ret.hint);
				if (funcName != NULL)
				{
					if ((instr.hw_ret.hint == AXP_HW_JMP) ||
						(instr.hw_ret.hint == AXP_HW_JSR))
						strLoc = sprintf(regStr, Reg, instr.hw_ret.rb);
					else
						strLoc = sprintf(regStr, Disp, instr.hw_ret.disp);
					sprintf(
						&regStr[strLoc],
						Stall,
						hwRetStall[instr.hw_ret.stall]);
				}
			}
			break;

		case LDS:		/* PREFETCH_M */
		case LDT:		/* PREFETCH_MEN */
			if (instr.mem.ra == 31)
			{
				if (instr.mem.opcode == LDS)
					funcName = prefetchCmd[AXP_LDS_PREFETCH];
				else
					funcName = prefetchCmd[AXP_LDT_PREFETCH];
			}
			else
				funcName = instrCmd[instr.mem.opcode];
			strLoc = sprintf(regStr, Freg, instr.mem.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			strLoc += sprintf(&regStr[strLoc], Disp, instr.mem.mem.disp);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], Reg, instr.mem.rb);
			break;

		case LDF:
		case LDG:
		case STF:
		case STG:
		case STS:
		case STT:
			funcName = instrCmd[instr.mem.opcode];
			strLoc = sprintf(regStr, Freg, instr.mem.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			strLoc += sprintf(&regStr[strLoc], Disp, instr.mem.mem.disp);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], Reg, instr.mem.rb);
			break;

		case FBEQ:
		case FBLT:
		case FBLE:
		case FBNE:
		case FBGE:
		case FBGT:
			funcName = instrCmd[instr.br.opcode];
			strLoc = sprintf(regStr, Freg, instr.br.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], Disp, instr.br.branch_disp);
			break;

		case BEQ:
		case BGE:
		case BGT:
		case BLBC:
		case BLBS:
		case BLE:
		case BLT:
		case BNE:
		case BR:
		case BSR:
			funcName = instrCmd[instr.br.opcode];
			strLoc = sprintf(regStr, Reg, instr.br.ra);
			strLoc += sprintf(&regStr[strLoc], Comma);
			sprintf(&regStr[strLoc], Disp, instr.br.branch_disp);
			break;

		case JMP:
			funcName = jmpCmd[AXP_JMP_TYPE(instr.mem.mem.func)];
			if (funcName != NULL)
			{
				strLoc = sprintf(regStr, Reg, instr.mem.ra);
				strLoc += sprintf(&regStr[strLoc], Comma);
				strLoc += sprintf(&regStr[strLoc], Reg, instr.mem.rb);
				strLoc += sprintf(&regStr[strLoc], Comma);
				sprintf(
					&regStr[strLoc],
					Hint,
					AXP_JMP_HINT(instr.mem.mem.func));
			}
			break;
	}

	/*
	 * So, if we did not find a function name, then we have a line that is not
	 * instruction code.  We replace the function name and register string to
	 * be filled with blanks.
	 *
	 * NOTE: We cannot always known when a 32-bit value is a real instruction
	 *		 or not.  We do know when it is not, though, as the encoding cannot
	 *		 be interpreted.
	 */
	if (funcName == NULL)
	{
		funcName = regStr;
		regStr[0] = ' ';
		regStr[1] = '\0';
	}

	/*
	 * OK, we should now have everything we need to generate the string
	 * representing the line of code.  Let's go generate it and get out of
	 * here.
	 */
	binFuncStr = (const char *) &instr;
	sprintf(
		instrStr,
		lineFmt,
		*((u64 *) pcAddr),
		funcName,
		regStr,
		*((u32 *) &instr),
		(isprint(binFuncStr[0]) ? binFuncStr[0] : '.'),
		(isprint(binFuncStr[1]) ? binFuncStr[1] : '.'),
		(isprint(binFuncStr[2]) ? binFuncStr[2] : '.'),
		(isprint(binFuncStr[3]) ? binFuncStr[3] : '.'));

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_Dump_Registers
 *	This function is called to dump the contents of the registers associated
 *	with an instruction.  It is anticipated that this is called after the
 *	instruction has been retired, so that the updated register values can be
 *	displayed.
 *
 * Input Parameters:
 *	instr:
 *		A pointer to the decoded, executed, and retired instruction.
 *	reg:
 *		An array containing the physical register values.
 *
 * Output Parameters:
 *	regStr:
 *		A pointer to a string buffer to receive the dumped register values.
 *
 * Return Values:
 *	None.
 */
void AXP_Dump_Registers(
				AXP_INSTRUCTION	*instr,
				AXP_REGISTERS	*pr,
				AXP_REGISTERS	*pf,
				char			*regStr)
{
	int		offset;

	/*
	 * Initialize the output string to a zero-length string.
	 */
	regStr[0] = '\0';

	/*
	 * All three registers Ra, Rb, Rc or Fa, Fb, Fc
	 */
	if ((instr->decodedReg.bits.src1 != 0) &&
		(instr->decodedReg.bits.src2 != 0) &&
		(instr->decodedReg.bits.dest != 0))
	{
		offset = sprintf(
					regStr,
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src1),
					instr->aSrc1,
					((instr->decodedReg.bits.src1 & AXP_REG_FP) ?
					 pf[instr->src1].value :
					 pr[instr->src1].value));
		offset += sprintf(&regStr[offset], Comma);
		offset += sprintf(
					&regStr[offset],
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src2),
					instr->aSrc2,
					((instr->decodedReg.bits.src2 & AXP_REG_FP) ?
					 pf[instr->src2].value :
					 pr[instr->src2].value));
		offset += sprintf(&regStr[offset], Comma);
		sprintf(
			&regStr[offset],
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.dest),
			instr->aDest,
			((instr->decodedReg.bits.dest & AXP_REG_FP) ?
			 pf[instr->dest].value :
			 pr[instr->dest].value));
	}

	/*
	 * Just two registers Ra, Rc
	 */
	else if ((instr->decodedReg.bits.src1 != 0) &&
			 (instr->decodedReg.bits.dest != 0))
	{
		offset = sprintf(
					regStr,
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src1),
					instr->aSrc1,
					((instr->decodedReg.bits.src1 & AXP_REG_FP) ?
					 pf[instr->src1].value :
					 pr[instr->src1].value));
		offset += sprintf(&regStr[offset], Comma);
		sprintf(
			&regStr[offset],
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.dest),
			instr->aDest,
			((instr->decodedReg.bits.dest & AXP_REG_FP) ?
			 pf[instr->dest].value :
			 pr[instr->dest].value));
	}

	/*
	 * Just two registers Rb, Rc
	 */
	else if ((instr->decodedReg.bits.src2 != 0) &&
			 (instr->decodedReg.bits.dest != 0))
	{
		offset = sprintf(
					regStr,
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src2),
					instr->aSrc2,
					((instr->decodedReg.bits.src2 & AXP_REG_FP) ?
					 pf[instr->src2].value :
					 pr[instr->src2].value));
		offset += sprintf(&regStr[offset], Comma);
		sprintf(
			&regStr[offset],
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.dest),
			instr->aDest,
			((instr->decodedReg.bits.dest & AXP_REG_FP) ?
			 pf[instr->dest].value :
			 pr[instr->dest].value));
	}

	/*
	 * Just two registers Ra, Rb
	 */
	else if ((instr->decodedReg.bits.src1 != 0) &&
			 (instr->decodedReg.bits.src2 != 0))
	{
		offset = sprintf(
					regStr,
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src1),
					instr->aSrc1,
					((instr->decodedReg.bits.src1 & AXP_REG_FP) ?
					 pf[instr->src1].value :
					 pr[instr->src1].value));
		offset += sprintf(&regStr[offset], Comma);
		offset += sprintf(
					&regStr[offset],
					Regv,
					AXP_REG_LETTER(instr->decodedReg.bits.src2),
					instr->aSrc2,
					((instr->decodedReg.bits.src2 & AXP_REG_FP) ?
					 pf[instr->src2].value :
					 pr[instr->src2].value));
	}

	/*
	 * Just one registers Ra
	 */
	else if (instr->decodedReg.bits.src1 != 0)
	{
		sprintf(
			regStr,
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.src1),
			instr->aSrc1,
			((instr->decodedReg.bits.src1 & AXP_REG_FP) ?
			 pf[instr->src1].value :
			 pr[instr->src1].value));
	}

	/*
	 * Just one register Rb
	 */
	else if (instr->decodedReg.bits.src2 != 0)
	{
		sprintf(
			regStr,
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.src2),
			instr->aSrc2,
			((instr->decodedReg.bits.src2 & AXP_REG_FP) ?
			 pf[instr->src2].value :
			 pr[instr->src2].value));
	}

	/*
	 * Just one register Rc
	 */
	else if (instr->decodedReg.bits.dest != 0)
	{
		sprintf(
			regStr,
			Regv,
			AXP_REG_LETTER(instr->decodedReg.bits.dest),
			instr->aDest,
			((instr->decodedReg.bits.dest & AXP_REG_FP) ?
			 pf[instr->dest].value :
			 pr[instr->dest].value));
	}

	/*
	 * Return back to the caller.
	 */
	return;
}
