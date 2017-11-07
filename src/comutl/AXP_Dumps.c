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
#include "AXP_DumpDefs.h"

typedef struct
{
	const u16	func;
	const char	*cmd;
} AXP_FUNC_CMD;

static const char *instrCmd[] =
{
	"CALL_PAL"
	"OPC01"
	"OPC02"
	"OPC03"
	"OPC04"
	"OPC05"
	"OPC06"
	"OPC07"
	"LDA"
	"LDAH"
	"LDBU"
	"LDQ_U"
	"LWW_U"
	"STW"
	"STB"
	"STQ_U"
	"INTA"
	"INTL"
	"INTS"
	"INTM"
	"ITFP"
	"FLTV"
	"FLTI"
	"FLTL"
	"MISC"
	"HW_MFPR"
	"JSR"
	"HW_LD"
	"FPTI"
	"HW_MTPR"
	"HW_RET"
	"HW_ST"
	"LDF"
	"LDG"
	"LDS"
	"LDT"
	"STF"
	"STG"
	"STS"
	"STT"
	"LDL"
	"LDQ"
	"LDL_L"
	"LDQ_L"
	"STL"
	"STQ"
	"STL_C"
	"STQ_C"
	"BR"
	"FBEQ"
	"FBLT"
	"FBLE"
	"BSR"
	"FBNE"
	"FBGE"
	"FBGT"
	"BLBC"
	"BEQ"
	"BLT"
	"BLE"
	"BLBS"
	"BNE"
	"BGE"
	"BGT"
};
const char *jmpCmd[] =
{
	"JMP",
	"JSR",
	NULL,
	"JSR_COROUTINE"
};

static const AXP_FUNC_CMD instaCmd[] =
{
	{AXP_FUNC_ADDL,		"ADDL"},
	{AXP_FUNC_S4ADDL,	"S4ADDL"},
	{AXP_FUNC_SUBL,		"SUBL"},
	{AXP_FUNC_S4SUBL,	"S4SUBL"},
	{AXP_FUNC_CMPBGE,	"CMPBGE"},
	{AXP_FUNC_S8ADDL,	"S8ADDL"},
	{AXP_FUNC_S8SUBL,	"S8SUBL"},
	{AXP_FUNC_CMPULT,	"CMPULT"},
	{AXP_FUNC_ADDQ,		"ADDQ"},
	{AXP_FUNC_S4ADDQ,	"S4ADDQ"},
	{AXP_FUNC_SUBQ,		"SUBQ"},
	{AXP_FUNC_S4SUBQ,	"S4SUBQ"},
	{AXP_FUNC_CMPEQ,	"CMPEQ"},
	{AXP_FUNC_S8ADDQ,	"S8ADDQ"},
	{AXP_FUNC_S8SUBQ,	"S8SUBQ"},
	{AXP_FUNC_CMPULE,	"CMPULE"},
	{AXP_FUNC_ADDL_V,	"ADDL/V"},
	{AXP_FUNC_SUBL_V,	"SUBL/V"},
	{AXP_FUNC_CMPLT,	"CMPLT"},
	{AXP_FUNC_ADDQ_V,	"ADDQ/V"},
	{AXP_FUNC_SUBQ_V,	"SUBQ/V"},
	{AXP_FUNC_CMPLE,	"CMPLE"},
	{0, NULL}
};

static const AXP_FUNC_CMD instlCmd[] =
{
	{AXP_FUNC_AND,		"AND"},
	{AXP_FUNC_BIC,		"BIC"},
	{AXP_FUNC_CMOVLBS,	"CMOVLBS"},
	{AXP_FUNC_CMOVLBC,	"CMOVLBC"},
	{AXP_FUNC_BIS,		"BIS"},
	{AXP_FUNC_CMOVEQ,	"CMOVEQ"},
	{AXP_FUNC_CMOVNE,	"CMOVNE"},
	{AXP_FUNC_ORNOT,	"ORNOT"},
	{AXP_FUNC_XOR,		"XOR"},
	{AXP_FUNC_CMOVLT,	"CMOVLT"},
	{AXP_FUNC_CMOVGE,	"CMOVGE"},
	{AXP_FUNC_EQV,		"EQV"},
	{AXP_FUNC_AMASK,	"AMASK"},
	{AXP_FUNC_IMPLVER,	"IMPLVER"},
	{0, NULL}
};

static const AXP_FUNC_CMD instsCmd[] =
{
	{AXP_FUNC_MSKBL,	"MSKBL"},
	{AXP_FUNC_EXTBL,	"EXTBL"},
	{AXP_FUNC_INSBL,	"INSBL"},
	{AXP_FUNC_MSKWL,	"MSKWL"},
	{AXP_FUNC_EXTWL,	"EXTWL"},
	{AXP_FUNC_INSWL,	"INSWL"},
	{AXP_FUNC_MSKLL,	"MSKLL"},
	{AXP_FUNC_EXTLL,	"EXTLL"},
	{AXP_FUNC_INSLL,	"INSLL"},
	{AXP_FUNC_ZAP,		"ZAP"},
	{AXP_FUNC_ZAPNOT,	"ZAPNOT"},
	{AXP_FUNC_MSKQL,	"MSKQL"},
	{AXP_FUNC_SRL,		"SRL"},
	{AXP_FUNC_EXTQL,	"EXTQL"},
	{AXP_FUNC_SLL,		"SLL"},
	{AXP_FUNC_INSQL,	"INSQL"},
	{AXP_FUNC_SRA,		"SRA"},
	{AXP_FUNC_MSKWH,	"MSKWH"},
	{AXP_FUNC_INSWH,	"INSWH"},
	{AXP_FUNC_EXTWH,	"EXTWH"},
	{AXP_FUNC_MSKLH,	"MSKLH"},
	{AXP_FUNC_INSLH,	"INSLH"},
	{AXP_FUNC_EXTLH,	"EXTLH"},
	{AXP_FUNC_MSKQH,	"MSKQH"},
	{AXP_FUNC_EXTQH,	"EXTQH"},
	{0, NULL}
};

static const AXP_FUNC_CMD instmCmd[] =
{
	{AXP_FUNC_MULL,		"MULL"},
	{AXP_FUNC_MULQ,		"MULQ"},
	{AXP_FUNC_UMULH,	"UMULH"},
	{AXP_FUNC_MULL_V,	"MULL/V"},
	{AXP_FUNC_MULQ_V,	"MULQ/V"},
	{0, NULL}
};

static const AXP_FUNC_CMD itfpCmd[] =
{
	{AXP_FUNC_ITOFS,	"ITOFS"},
	{AXP_FUNC_SQRTF_C,	"SQRTF/C"},
	{AXP_FUNC_SQRTS_C,	"SQRTS/C"},
	{AXP_FUNC_ITOFF,	"ITOFF"},
	{AXP_FUNC_ITOFT,	"ITOFT"},
	{AXP_FUNC_SQRTG_C,	"SQRTG/C"},
	{AXP_FUNC_SQRTT_C,	"SQRTT/C"},
	{AXP_FUNC_SQRTS_M,	"SQRTS/M"},
	{AXP_FUNC_SQRTT_M,	"SQRTT/M"},
	{AXP_FUNC_SQRTF,	"SQRTF"},
	{AXP_FUNC_SQRTS,	"SQRTS"},
	{AXP_FUNC_SQRTG,	"SQRTG"},
	{AXP_FUNC_SQRTT,	"SQRTT"},
	{AXP_FUNC_SQRTS_D,	"SQRTS/D"},
	{AXP_FUNC_SQRTT_D,	"SQRTT/D"},
	{AXP_FUNC_SQRTF_UC,	"SQRTF/UC"},
	{AXP_FUNC_SQRTS_UC,	"SQRTS/UC"},
	{AXP_FUNC_SQRTG_UC,	"SQRTG/UC"},
	{AXP_FUNC_SQRTT_UC,	"SQRTT/UC"},
	{AXP_FUNC_SQRTS_UM,	"SQRTS/UM"},
	{AXP_FUNC_SQRTT_UM,	"SQRTT/UM"},
	{AXP_FUNC_SQRTF_U,	"SQRTF/U"},
	{AXP_FUNC_SQRTS_U,	"SQRTS/U"},
	{AXP_FUNC_SQRTG_U,	"SQRTG/U"},
	{AXP_FUNC_SQRTT_U,	"SQRTT/U"},
	{AXP_FUNC_SQRTS_UD,	"SQRTS/UD"},
	{AXP_FUNC_SQRTT_UD,	"SQRTT/UD"},
	{AXP_FUNC_SQRTF_SC, "SQRTF/SC"},
	{AXP_FUNC_SQRTG_SC,	"SQRTG/SC"},
	{AXP_FUNC_SQRTF_S,	"SQRTF/S"},
	{AXP_FUNC_SQRTG_S,	"SQRTG/S"},
	{AXP_FUNC_SQRTF_SUC,"SQRTF/SUC"},
	{AXP_FUNC_SQRTS_SUC,"SQRTS/SUC"},
	{AXP_FUNC_SQRTG_SUC,"SQRTG/SUC"},
	{AXP_FUNC_SQRTT_SUC,"SQRTT/SUC"},
	{AXP_FUNC_SQRTS_SUM,"SQRTS/SUM"},
	{AXP_FUNC_SQRTT_SUM,"SQRTT/SUM"},
	{AXP_FUNC_SQRTF_SU,	"SQRTF/SU"},
	{AXP_FUNC_SQRTS_SU,	"SQRTS/SU"},
	{AXP_FUNC_SQRTG_SU,	"SQRTG/SU"},
	{AXP_FUNC_SQRTT_SU,	"SQRTT/SU"},
	{AXP_FUNC_SQRTS_SUD,"SQRTS/SUD"},
	{AXP_FUNC_SQRTT_SUD,"SQRTT/SUD"},
	{AXP_FUNC_SQRTS_SUIC,"SQRTS/SUIC"},
	{AXP_FUNC_SQRTT_SUIC,"SQRTT/SUIC"},
	{AXP_FUNC_SQRTS_SUIM,"SQRTS/SUIM"},
	{AXP_FUNC_SQRTT_SUIM,"SQRTT/SUIM"},
	{AXP_FUNC_SQRTS_SUI,"SQRTS/SUI"},
	{AXP_FUNC_SQRTT_SUI,"SQRTT/SUI"},
	{AXP_FUNC_SQRTS_SUID,"SQRTS/SUID"},
	{AXP_FUNC_SQRTT_SUID,"SQRTT/SUID"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltvCmd[] =
{
	{AXP_FUNC_ADDF_C,	"ADDF/C"},
	{AXP_FUNC_SUBF_C,	"SUBF/C"},
	{AXP_FUNC_MULF_C,	"MULF/C"},
	{AXP_FUNC_DIVF_C,	"DIVF/C"},
	{AXP_FUNC_CVTDG_C,	"CVTDG/C"},
	{AXP_FUNC_ADDG_C,	"ADDG/C"},
	{AXP_FUNC_SUBG_C,	"SUBG/C"},
	{AXP_FUNC_MULG_C,	"MULG/C"},
	{AXP_FUNC_DIVG_C,	"DIVG/C"},
	{AXP_FUNC_CVTGF_C,	"CVTGF/C"},
	{AXP_FUNC_CVTGD_C,	"CVTGD/C"},
	{AXP_FUNC_CVTGQ_C,	"CVTGQ/C"},
	{AXP_FUNC_CVTQF_C,	"CVTQF/C"},
	{AXP_FUNC_CVTQG_C,	"CVTQG/C"},
	{AXP_FUNC_ADDF,		"ADDF"},
	{AXP_FUNC_SUBF,		"SUBF"},
	{AXP_FUNC_MULF,		"MULF"},
	{AXP_FUNC_DIVF,		"DIVF"},
	{AXP_FUNC_CVTDG,	"CVTDG"},
	{AXP_FUNC_ADDG,		"ADDG"},
	{AXP_FUNC_SUBG,		"SUBG"},
	{AXP_FUNC_MULG,		"MULG"},
	{AXP_FUNC_DIVG,		"DIVG"},
	{AXP_FUNC_CMPGEQ,	"CMPGEQ"},
	{AXP_FUNC_CMPGLT,	"CMPGLT"},
	{AXP_FUNC_CMPGLE,	"CMPGLE"},
	{AXP_FUNC_CVTGF,	"CVTGF"},
	{AXP_FUNC_CVTGD,	"CVTGD"},
	{AXP_FUNC_CVTGQ,	"CVTGQ"},
	{AXP_FUNC_CVTQF,	"CVTQF"},
	{AXP_FUNC_CVTQG,	"CVTQG"},
	{AXP_FUNC_ADDF_UC,	"ADDF/UC"},
	{AXP_FUNC_SUBF_UC,	"SUBF/UC"},
	{AXP_FUNC_MULF_UC,	"MULF/UC"},
	{AXP_FUNC_DIVF_UC,	"DIVF/UC"},
	{AXP_FUNC_CVTDG_UC,	"CVTDG/UC"},
	{AXP_FUNC_ADDG_UC,	"ADDG/UC"},
	{AXP_FUNC_SUBG_UC,	"SUBG/UC"},
	{AXP_FUNC_MULG_UC,	"MULG/UC"},
	{AXP_FUNC_DIVG_UC,	"DIVG/UC"},
	{AXP_FUNC_CVTGF_UC,	"CVTGF/UC"},
	{AXP_FUNC_CVTGD_UC,	"CVTGD/UC"},
	{AXP_FUNC_CVTGQ_VC,	"CVTGQ/VC"},
	{AXP_FUNC_ADDF_U,	"ADDF/U"},
	{AXP_FUNC_SUBF_U,	"SUBF/U"},
	{AXP_FUNC_MULF_U,	"MULF/U"},
	{AXP_FUNC_DIVF_U,	"DIVF/U"},
	{AXP_FUNC_SQRTF_U,	"SQRTF/U"},
	{AXP_FUNC_CVTDG_U,	"CVTDG/U"},
	{AXP_FUNC_ADDG_U,	"ADDG/U"},
	{AXP_FUNC_SUBG_U,	"SUBG/U"},
	{AXP_FUNC_MULG_U,	"MULG/U"},
	{AXP_FUNC_DIVG_U,	"DIVG/U"},
	{AXP_FUNC_CVTGF_U,	"CVTGF/U"},
	{AXP_FUNC_CVTGD_U,	"CVTGD/U"},
	{AXP_FUNC_CVTGQ_V,	"CVTGQ/V"},
	{AXP_FUNC_ADDF_SC,	"ADDF/SC"},
	{AXP_FUNC_SUBF_SC,	"SUBF/SC"},
	{AXP_FUNC_MULF_SC,	"MULF/SC"},
	{AXP_FUNC_DIVF_SC,	"DIVF/SC"},
	{AXP_FUNC_CVTDG_SC,	"CVTDG/SC"},
	{AXP_FUNC_ADDG_SC,	"ADDG/SC"},
	{AXP_FUNC_SUBG_SC,	"SUBG/SC"},
	{AXP_FUNC_MULG_SC,	"MULG/SC"},
	{AXP_FUNC_DIVG_SC,	"DIVG/SC"},
	{AXP_FUNC_CVTGF_SC,	"CVTGF/SC"},
	{AXP_FUNC_CVTGD_SC,	"CVTGD/SC"},
	{AXP_FUNC_CVTGQ_SC,	"CVTGQ/SC"},
	{AXP_FUNC_ADDF_S,	"ADDF/S"},
	{AXP_FUNC_SUBF_S,	"SUBF/S"},
	{AXP_FUNC_MULF_S,	"MULF/S"},
	{AXP_FUNC_DIVF_S,	"DIVF/S"},
	{AXP_FUNC_CVTDG_S,	"CVTDG/S"},
	{AXP_FUNC_ADDG_S,	"ADDG/S"},
	{AXP_FUNC_SUBG_S,	"SUBG/S"},
	{AXP_FUNC_MULG_S,	"MULG/S"},
	{AXP_FUNC_DIVG_S,	"DIVG/S"},
	{AXP_FUNC_CMPGEQ_S,	"CMPGEQ/S"},
	{AXP_FUNC_CMPGLT_S,	"CMPGLT/S"},
	{AXP_FUNC_CMPGLE_S,	"CMPGLE/S"},
	{AXP_FUNC_CVTGF_S,	"CVTGF/S"},
	{AXP_FUNC_CVTGD_S,	"CVTGD/S"},
	{AXP_FUNC_CVTGQ_S,	"CVTGQ/S"},
	{AXP_FUNC_ADDF_SUC,	"ADDF/SUC"},
	{AXP_FUNC_SUBF_SUC,	"SUBF/SUC"},
	{AXP_FUNC_MULF_SUC,	"MULF/SUC"},
	{AXP_FUNC_DIVF_SUC,	"DIVF/SUC"},
	{AXP_FUNC_CVTDG_SUC,"CVTDG/SUC"},
	{AXP_FUNC_ADDG_SUC,	"ADDG/SUC"},
	{AXP_FUNC_SUBG_SUC,	"SUBG/SUC"},
	{AXP_FUNC_MULG_SUC,	"MULG/SUC"},
	{AXP_FUNC_DIVG_SUC,	"DIVG/SUC"},
	{AXP_FUNC_CVTGF_SUC,"CVTGF/SUC"},
	{AXP_FUNC_CVTGD_SUC,"CVTGD/SUC"},
	{AXP_FUNC_CVTGQ_SVC,"CVTGQ/SVC"},
	{AXP_FUNC_ADDF_SU,	"ADDF/SU"},
	{AXP_FUNC_SUBF_SU,	"SUBF/SU"},
	{AXP_FUNC_MULF_SU,	"MULF/SU"},
	{AXP_FUNC_DIVF_SU,	"DIVF/SU"},
	{AXP_FUNC_CVTDG_SU,	"CVTDG/SU"},
	{AXP_FUNC_ADDG_SU,	"ADDG/SU"},
	{AXP_FUNC_SUBG_SU,	"SUBG/SU"},
	{AXP_FUNC_MULG_SU,	"MULG/SU"},
	{AXP_FUNC_DIVG_SU,	"DIVG/SU"},
	{AXP_FUNC_CVTGF_SU,	"CVTGF/SU"},
	{AXP_FUNC_CVTGD_SU,	"CVTGD/SU"},
	{AXP_FUNC_CVTGQ_SV,	"CVTGQ/SV"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltiCmd[] =
{
	{AXP_FUNC_ADDS_UM,	"ADDS/UM"},
	{AXP_FUNC_SUBS_UM,	"SUBS/UM"},
	{AXP_FUNC_MULS_UM,	"MULS/UM"},
	{AXP_FUNC_DIVS_UM,	"DIVS/UM"},
	{AXP_FUNC_ADDT_UM,	"ADDT/UM"},
	{AXP_FUNC_SUBT_UM,	"SUBT/UM"},
	{AXP_FUNC_MULT_UM,	"MULT/UM"},
	{AXP_FUNC_DIVT_UM,	"DIVT/UM"},
	{AXP_FUNC_CVTTS_UM,	"CVTTS/UM"},
	{AXP_FUNC_CVTTQ_VM,	"CVTTQ/VM"},
	{AXP_FUNC_ADDS_U,	"ADDS/U"},
	{AXP_FUNC_SUBS_U,	"SUBS/U"},
	{AXP_FUNC_MULS_U,	"MULS/U"},
	{AXP_FUNC_DIVS_U,	"DIVS/U"},
	{AXP_FUNC_ADDT_U,	"ADDT/U"},
	{AXP_FUNC_SUBT_U,	"SUBT/U"},
	{AXP_FUNC_MULT_U,	"MULT/U"},
	{AXP_FUNC_DIVT_U,	"DIVT/U"},
	{AXP_FUNC_CVTTS_U,	"CVTTS/U"},
	{AXP_FUNC_CVTTQ_V,	"CVTTQ/V"},
	{AXP_FUNC_ADDS_UD,	"ADDS/UD"},
	{AXP_FUNC_SUBS_UD,	"SUBS/UD"},
	{AXP_FUNC_MULS_UD,	"MULS/UD"},
	{AXP_FUNC_DIVS_UD,	"DIVS/UD"},
	{AXP_FUNC_ADDT_UD,	"ADDT/UD"},
	{AXP_FUNC_SUBT_UD,	"SUBT/UD"},
	{AXP_FUNC_MULT_UD,	"MULT/UD"},
	{AXP_FUNC_DIVT_UD,	"DIVT/UD"},
	{AXP_FUNC_CVTTS_UD,	"CVTTS/UD"},
	{AXP_FUNC_CVTTQ_VD,	"CVTTQ/VD"},
	{AXP_FUNC_CVTST,	"CVTST"},
	{AXP_FUNC_ADDS_SUC,	"ADDS/SUC"},
	{AXP_FUNC_SUBS_SUC,	"SUBS/SUC"},
	{AXP_FUNC_MULS_SUC,	"MULS/SUC"},
	{AXP_FUNC_DIVS_SUC,	"DIVS/SUC"},
	{AXP_FUNC_ADDT_SUC,	"ADDT/SUC"},
	{AXP_FUNC_SUBT_SUC,	"SUBT/SUC"},
	{AXP_FUNC_MULT_SUC,	"MULT/SUC"},
	{AXP_FUNC_DIVT_SUC,	"DIVT/SUC"},
	{AXP_FUNC_CVTTS_SUC,"CVTTS/SUC"},
	{AXP_FUNC_CVTTQ_SVC,"CVTTQ/SVC"},
	{AXP_FUNC_ADDS_SUM,	"ADDS/SUM"},
	{AXP_FUNC_SUBS_SUM,	"SUBS/SUM"},
	{AXP_FUNC_MULS_SUM,	"MULS/SUM"},
	{AXP_FUNC_DIVS_SUM,	"DIVS/SUM"},
	{AXP_FUNC_ADDT_SUM,	"ADDT/SUM"},
	{AXP_FUNC_SUBT_SUM,	"SUBT/SUM"},
	{AXP_FUNC_MULT_SUM,	"MULT/SUM"},
	{AXP_FUNC_DIVT_SUM,	"DIVT/SUM"},
	{AXP_FUNC_CVTTS_SUM,"CVTTS/SUM"},
	{AXP_FUNC_CVTTQ_SVM,"CVTTQ/SVM"},
	{AXP_FUNC_ADDS_SU,	"ADDS/SU"},
	{AXP_FUNC_SUBS_SU,	"SUBS/SU"},
	{AXP_FUNC_MULS_SU,	"MULS/SU"},
	{AXP_FUNC_DIVS_SU,	"DIVS/SU"},
	{AXP_FUNC_ADDT_SU,	"ADDT/SU"},
	{AXP_FUNC_SUBT_SU,	"SUBT/SU"},
	{AXP_FUNC_MULT_SU,	"MULT/SU"},
	{AXP_FUNC_DIVT_SU,	"DIVT/SU"},
	{AXP_FUNC_CMPTUN_SU,"CMPTUN/SU"},
	{AXP_FUNC_CMPTEQ_SU,"CMPTEQ/SU"},
	{AXP_FUNC_CMPTLT_SU,"CMPTLT/SU"},
	{AXP_FUNC_CMPTLE_SU,"CMPTLE/SU"},
	{AXP_FUNC_CVTTS_SU,	"CVTTS/SU"},
	{AXP_FUNC_CVTTQ_SV,	"CVTTQ/SV"},
	{AXP_FUNC_ADDS_SUD,	"ADDS/SUD"},
	{AXP_FUNC_SUBS_SUD,	"SUBS/SUD"},
	{AXP_FUNC_MULS_SUD,	"MULS/SUD"},
	{AXP_FUNC_DIVS_SUD,	"DIVS/SUD"},
	{AXP_FUNC_ADDT_SUD,	"ADDT/SUD"},
	{AXP_FUNC_SUBT_SUD,	"SUBT/SUD"},
	{AXP_FUNC_MULT_SUD,	"MULT/SUD"},
	{AXP_FUNC_DIVT_SUD,	"DIVT/SUD"},
	{AXP_FUNC_CVTTS_SUD,"CVTTS/SUD"},
	{AXP_FUNC_CVTTQ_SVD,"CVTTQ/SVD"},
	{AXP_FUNC_CVTST_S,	"CVTST/S"},
	{AXP_FUNC_ADDS_SUIC,"ADDS/SUIC"},
	{AXP_FUNC_SUBS_SUIC,"SUBS/SUIC"},
	{AXP_FUNC_MULS_SUIC,"MULS/SUIC"},
	{AXP_FUNC_DIVS_SUIC,"DIVS/SUIC"},
	{AXP_FUNC_ADDT_SUIC,"ADDT/SUIC"},
	{AXP_FUNC_SUBT_SUIC,"SUBT/SUIC"},
	{AXP_FUNC_MULT_SUIC,"MULT/SUIC"},
	{AXP_FUNC_DIVT_SUIC,"DIVT/SUIC"},
	{AXP_FUNC_CVTTS_SUIC,"CVTTS/SUIC"},
	{AXP_FUNC_CVTTQ_SVIC,"CVTTQ/SVIC"},
	{AXP_FUNC_CVTQS_SUIC,"CVTQS/SUIC"},
	{AXP_FUNC_CVTQT_SUIC,"CVTQT/SUIC"},
	{AXP_FUNC_ADDS_SUIM,"ADDS/SUIM"},
	{AXP_FUNC_SUBS_SUIM,"SUBS/SUIM"},
	{AXP_FUNC_MULS_SUIM,"MULS/SUIM"},
	{AXP_FUNC_DIVS_SUIM,"DIVS/SUIM"},
	{AXP_FUNC_ADDT_SUIM,"ADDT/SUIM"},
	{AXP_FUNC_SUBT_SUIM,"SUBT/SUIM"},
	{AXP_FUNC_MULT_SUIM,"MULT/SUIM"},
	{AXP_FUNC_DIVT_SUIM,"DIVT/SUIM"},
	{AXP_FUNC_CVTTS_SUIM,"CVTTS/SUIM"},
	{AXP_FUNC_CVTTQ_SVIM,"CVTTQ/SVIM"},
	{AXP_FUNC_CVTQS_SUIM,"CVTQS/SUIM"},
	{AXP_FUNC_CVTQT_SUIM,"CVTQT/SUIM"},
	{AXP_FUNC_ADDS_SUI,	"ADDS/SUI"},
	{AXP_FUNC_SUBS_SUI,	"SUBS/SUI"},
	{AXP_FUNC_MULS_SUI,	"MULS/SUI"},
	{AXP_FUNC_DIVS_SUI,	"DIVS/SUI"},
	{AXP_FUNC_ADDT_SUI,	"ADDT/SUI"},
	{AXP_FUNC_SUBT_SUI,	"SUBT/SUI"},
	{AXP_FUNC_MULT_SUI,	"MULT/SUI"},
	{AXP_FUNC_DIVT_SUI,	"DIVT/SUI"},
	{AXP_FUNC_CVTTS_SUI,"CVTTS/SUI"},
	{AXP_FUNC_CVTTQ_SVI,"CVTTQ/SVI"},
	{AXP_FUNC_CVTQS_SUI,"CVTQS/SUI"},
	{AXP_FUNC_CVTQT_SUI,"CVTQT/SUI"},
	{AXP_FUNC_ADDS_SUID,"ADDS/SUID"},
	{AXP_FUNC_SUBS_SUID,"SUBS/SUID"},
	{AXP_FUNC_MULS_SUID,"MULS/SUID"},
	{AXP_FUNC_DIVS_SUID,"DIVS/SUID"},
	{AXP_FUNC_ADDT_SUID,"ADDT/SUID"},
	{AXP_FUNC_SUBT_SUID,"SUBT/SUID"},
	{AXP_FUNC_MULT_SUID,"MULT/SUID"},
	{AXP_FUNC_DIVT_SUID,"DIVT/SUID"},
	{AXP_FUNC_CVTTS_SUID,"CVTTS/SUID"},
	{AXP_FUNC_CVTTQ_SVID,"CVTTQ/SVID"},
	{AXP_FUNC_CVTQS_SUID,"CVTQS/SUID"},
	{AXP_FUNC_CVTQT_SUID,"CVTQT/SUID"},
	{0, NULL}
};

static const AXP_FUNC_CMD fltlCmd[] =
{
	{AXP_FUNC_CVTLQ,	"CVTLQ"},
	{AXP_FUNC_CPYS,		"CPYS"},
	{AXP_FUNC_CPYSN,	"CPYSN"},
	{AXP_FUNC_CPYSE,	"CPYSE"},
	{AXP_FUNC_MT_FPCR,	"MT_FPCR"},
	{AXP_FUNC_MF_FPCR,	"MF_FPCR"},
	{AXP_FUNC_FCMOVEQ,	"FCMOVEQ"},
	{AXP_FUNC_FCMOVNE,	"FCMOVNE"},
	{AXP_FUNC_FCMOVLT,	"FCMOVLT"},
	{AXP_FUNC_FCMOVGE,	"FCMOVGE"},
	{AXP_FUNC_FCMOVLE,	"FCMOVLE"},
	{AXP_FUNC_FCMOVGT,	"FCMOVGT"},
	{AXP_FUNC_CVTQL,	"CVTQL"},
	{AXP_FUNC_CVTQL_V,	"CVTQL/V"},
	{AXP_FUNC_CVTQL_SV,	"CVTQL/SV"},
	{0, NULL}
};

static const AXP_FUNC_CMD miscCmd[] =
{
	{AXP_FUNC_TRAPB,	"TRAPB"},
	{AXP_FUNC_EXCB,		"EXCB"},
	{AXP_FUNC_MB,		"MB"},
	{AXP_FUNC_WMB,		"WMB"},
	{AXP_FUNC_FETCH,	"FETCH"},
	{AXP_FUNC_FETCH_M,	"FETCH_M"},
	{AXP_FUNC_RPCC,		"RPCC"},
	{AXP_FUNC_RC,		"RC"},
	{AXP_FUNC_ECB,		"ECB"},
	{AXP_FUNC_RS,		"RS"},
	{AXP_FUNC_WH64,		"WH64"},
	{AXP_FUNC_WH64EN,	"WH64EN"},
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
	{VMS_HALT,			"HALT (halt)"},
	{VMS_CFLUSH,		"CFLUSH (cflush)"},
	{VMS_DRAINA,		"DRAINA (draina)"},
	{VMS_LDQP,			"LDQP"},
	{VMS_STQP,			"STQP"},
	{VMS_SWPCTX,		"SWPCTX"},
	{VMS_MFPR_ASN,		"MFPR_ASN"},
	{VMS_MTPR_ASTEN,	"MTPR_ASTEN"},
	{VMS_MTPR_ASTSR,	"MTPR_ASTSR"},
	{VMS_CSERVE,		"CSERVE (cserve)"},
	{VMS_SWPPAL,		"SWPPAL (swppal)"},
	{VMS_MFPR_FEN,		"MFPR_FEN"},
	{VMS_MTPR_FEN,		"MTPR_FEN"},
	{VMS_MTPR_IPIR,		"MTPR_IPIR (wripir)"},
	{VMS_MFPR_IPL,		"MFPR_IPL"},
	{VMS_MTPR_IPL,		"MTPR_IPL"},
	{VMS_MFPR_MCES,		"MFPR_MCES (rdmces)"},
	{VMS_MTPR_MCES,		"MTPR_MCES (wrmces)"},
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
	{VMS_MTPR_PERFMON,	"MTPR_PERFMON (wrfen)"},
	{OSF_WRVPTPTR,		"wrvptptr"},
	{VMS_DATFX,			"DATFX"},
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
	{VMS_WTINT,			"WTINT (wtint)"},
	{VMS_MFPR_WHAMI,	"MFPR_WHAMI (rti)"},
	{VMS_BPT,			"BPT (bpt)"},
	{VMS_BUGCHK,		"BUGCHK (bugchk)"},
	{VMS_CHME,			"CHME"},
	{VMS_CHMK,			"CHMK (callsys)"},
	{VMS_CHMS,			"CHMS"},
	{VMS_CHMU,			"CHMU"},
	{VMS_IMB,			"IMB (imb)"},
	{VMS_INSQHIL,		"INSQHIL"},
	{VMS_INSQTIL,		"INSQTIL"},
	{VMS_INSQHIQ,		"INSQHIQ"},
	{VMS_INSQTIQ,		"INSQTIQ"},
	{VMS_INSQUEL,		"INSQUEL"},
	{VMS_INSQUEQ,		"INSQUEQ"},
	{VMS_INSQUEL_D,		"INSQUEL_D"},
	{VMS_INSQUEQ_D,		"INSQUEQ_D"},
	{VMS_PROBER,		"PROBER"},
	{VMS_PROBEW,		"PROBEW"},
	{VMS_RD_PS,			"RD_PS"},
	{VMS_REI,			"REI (urti)"},
	{VMS_REMQHIL,		"REMQHIL"},
	{VMS_REMQTIL,		"REMQTIL"},
	{VMS_REMQHIQ,		"REMQHIQ"},
	{VMS_REMQTIQ,		"REMQTIQ"},
	{VMS_REMQUEL,		"REMQUEL"},
	{VMS_REMQUEQ,		"REMQUEQ"},
	{VMS_REMQUEL_D,		"REMQUEL_D"},
	{VMS_REMQUEQ_D,		"REMQUEQ_D"},
	{VMS_SWASTEN,		"SWASTEN"},
	{VMS_WR_PS_SW,		"WR_PS_SW"},
	{VMS_RSCC,			"RSCC"},
	{VMS_READ_UNQ,		"READ_UNQ (rduniue)"},
	{VMS_WRITE_UNQ,		"WRITE_UNQ (wrunique)"},
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
	{VMS_GENTRAP,		"GENTRAP (gentrap)"},
	{VMS_CLRFEN,		"CLRFEN (clrfen)"},
	{0, NULL}
};

static const AXP_FUNC_CMD fptiCmd[] =
{
	{AXP_FUNC_SEXTW,	"SEXTW"},
	{AXP_FUNC_CTPOP,	"CTPOP"},
	{AXP_FUNC_PERR,		"PERR"},
	{AXP_FUNC_CTLZ,		"CTLZ"},
	{AXP_FUNC_CTTZ,		"CTTZ"},
	{AXP_FUNC_UNPKBW,	"UNPKBW"},
	{AXP_FUNC_UNPKBL,	"UNPKBL"},
	{AXP_FUNC_PKWB,		"PKWB"},
	{AXP_FUNC_PKLB,		"PKLB"},
	{AXP_FUNC_MINSB8,	"MINSB8"},
	{AXP_FUNC_MINSW4,	"MINSW4"},
	{AXP_FUNC_MINUB8,	"MINUB8"},
	{AXP_FUNC_MINUW4,	"MINUW4"},
	{AXP_FUNC_MAXUB8,	"MAXUB8"},
	{AXP_FUNC_MAXUW4,	"MAXUW4"},
	{AXP_FUNC_MAXSB8,	"MAXSB8"},
	{AXP_FUNC_MAXSW4,	"MAXSW4"},
	{AXP_FUNC_FTOIT,	"FTOIT"},
	{AXP_FUNC_FTOIS,	"FTOIS"},
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
	{0, NULL}
};

static const AXP_FUNC_CMD hwLdCmd[] =
{
	{AXP_HW_LD_PHYS,		"P"},
	{AXP_HW_LD_PHYS_LOCK,	"P_L"},
	{AXP_HW_LD_VPTE,		"VPTE"},
	{AXP_HW_LD_VIRT,		"VIRT"},
	{AXP_HW_LD_VIRT_WCHK,	"VIRT_WRCHK"},
	{AXP_HW_LD_VIRT_ALT,	"VIRT_ALT"},
	{AXP_HW_LD_VIRT_WALT,	"VIRT_WRCHK_ALT"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwStCmd[] =
{
	{AXP_HW_ST_PHYS,		"P"},
	{AXP_HW_ST_PHYS_LOCK,	"P_C"},
	{AXP_HW_ST_VIRT,		"VIRT"},
	{AXP_HW_ST_VIRT_ALT,	"VIRT_ALT"},
	{0, NULL}
};

static const AXP_FUNC_CMD hwLen[] =
{
	{AXP_HW_LD_LONGWORD,	"LONG"},
	{AXP_HW_LD_QUADWORD,	"QUAD"},
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
	"STALL"
};

const char *addrFmt		= "0x%016llx: ";
const char *instBinFmt	= "; 0x%08x '%c%c%c%c'";
const char *invOpcode	= "<INV-OPC>";
const char *resOpcode	= "<RES-OPC>";
const char *instrFmt 	= "%-13s ";
const char *palFunc		= "%-21s ";
const char *regRaDispRb = "R%02d, %6d(R%02d) ";	/* 'R01, -65535(R01)      ' (22) */
const char *regRaDispRbF= "%10d(R%02d)      ";		/* '-65535(R01)           ' (22) */
const char *regRaDisp 	= "R%02d, %7d         ";	/* 'R01, -2097151         ' (22) */
const char *regRaRbHint = "R%02d, (R%02d), %4d     ";/* 'R01, (R01), -8191     ' (22) */
const char *regRaRb     = "R%02d, (R%02d)          ";/* 'R01, (R01)            ' (22) */
const char *regRaRbRc	= "R%02d, R%02d, R%02d         ";/* 'R01, R01, R01         ' (22) */
const char *regRbRc		= "R%02d, R%02d              ";/* 'R01, R01              ' (22) */
const char *regLitRc	= "#%02d, R%02d              ";/* '#31, R01              ' (22) */
const char *regRaLitRc	= "R%02d, #%03d, R%02d        ";/* 'R01, #255, R01        ' (22) */
const char *regRa		= "R%02d                   ";/* 'R01                   ' (22) */
const char *regRb		= "(R%02d)                 ";/* '(R01)                 ' (22) */
const char *reg0Rb		= "0(R%02d)                ";/* '0(R01)                ' (22) */
const char *regRc		= "R%02d                   ";/* 'R01                   ' (22) */
const char *regFaFbFc	= "F%02f, F%02d, F%02d         ";/* 'F01, F01, F01         ' (22) */
const char *regFbFc		= "F%02d, F%02d              ";/* 'F01, F01              ' (22) */
const char *regFaDispRb = "F%02d, %10d(R%02d) ";	/* 'F01, -65535(R01)      ' (22) */
const char *regFaDisp	= "F%02d, %10d      ";		/* 'F01, -65535           ' (22) */
const char *regFaRc		= "F%02d, R%02d              ";/* 'F01, R01              ' (22) */
const char *regNone		= "                      ";	/* '                      ' (22) */
const char *mxprRegScbd	= "R%02d, %3d              ";/* 'R01, 127              ' (22) */
const char *hwRaRbDispLen = "R%02d, %5d(R%02d), %-4s ";/* 'R01, -4095(R01), QUAD ' (22) */
const char *hwRetRb		= "Rb, %-5s             ";	/* 'R01, STALL             ' (22) */
const char *hwJmpRb		= "#%5d, %-5s        ";	/* '#-4095, STALL        ' (22) */

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
			AXP_PC		pcAddr,
			AXP_INS_FMT	instr,
			bool 		kernelMode,
			char 		*instrStr)
{
	int			strLoc = 0;
	const char	*funcStr;
	char		iprName[16];
	char		funcName[24];
	u16			index;

	strLoc = sprintf(&instrStr[strLoc], addrFmt, *((u64 *) &pcAddr));
	switch(instr.pal.opcode)
	{
		case PAL00:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.pal.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						palFunc,
						AXP_Get_Func_Str(palcodeCmd, instr.pal.palcode_func));
			break;

		case OPC01:
		case OPC02:
		case OPC03:
		case OPC04:
		case OPC05:
		case OPC06:
		case OPC07:
			strLoc = sprintf(
						&instrStr[strLoc],
						"%s",
						invOpcode);
			strLoc = sprintf(
						&instrStr[strLoc],
						"%s",
						regNone);
			break;

		case LDL:		/* PREFETCH */
		case LDQ:		/* PREFETCH_EN */
			if (instr.mem.ra == 31)
			{
				if (instr.mem.opcode == LDL)
					strLoc = sprintf(
								&instrStr[strLoc],
								instrFmt,
								prefetchCmd[AXP_LDL_PREFETCH]);
				else
					strLoc = sprintf(
								&instrStr[strLoc],
								instrFmt,
								prefetchCmd[AXP_LDQ_PREFETCH]);
			}
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regRaDispRb,
						instr.mem.ra,
						instr.mem.mem.disp,
						instr.mem.rb);
			break;

		case LDA:
		case LDBU:
		case LDW_U:
		case LDQ_U:
		case LDL_L:
		case LDQ_L:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regRaDispRb,
						instr.mem.ra,
						instr.mem.mem.disp,
						instr.mem.rb);
			break;

		case LDAH:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regRaDispRb,
						instr.mem.ra,
						(instr.mem.mem.disp * AXP_LDAH_MULT),
						instr.mem.rb);
			break;

		case STB:
		case STW:
		case STL:
		case STQ:
		case STQ_U:
		case STL_C:
		case STQ_C:
			strLoc = sprintf(
				&instrStr[strLoc],
				instrFmt,
				instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regRaDispRb,
						instr.mem.ra,
						instr.mem.mem.disp,
						instr.mem.rb);
			break;

		case INTA:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(instaCmd, instr.oper1.func));
			if (instr.oper1.fmt == 0)
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaRbRc,
							instr.oper1.ra,
							instr.oper1.rb,
							instr.oper1.rc);
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaLitRc,
							instr.oper2.ra,
							instr.oper2.lit,
							instr.oper2.rc);
			break;

		case INTL:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(instlCmd, instr.oper1.func));
			if (instr.oper1.func == AXP_FUNC_AMASK)
			{
				if (instr.oper1.fmt == 0)
					strLoc = sprintf(
								&instrStr[strLoc],
								regRbRc,
								instr.oper1.rb,
								instr.oper1.rc);
				else
					strLoc = sprintf(
								&instrStr[strLoc],
								regLitRc,
								instr.oper2.lit,
								instr.oper2.rc);
			}
			else if (instr.oper1.func == AXP_FUNC_IMPLVER)
				strLoc = sprintf(
							&instrStr[strLoc],
							regRc,
							instr.oper1.rc);
			else if (instr.oper1.fmt == 0)
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaRbRc,
							instr.oper1.ra,
							instr.oper1.rb,
							instr.oper1.rc);
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaLitRc,
							instr.oper2.ra,
							instr.oper2.lit,
							instr.oper2.rc);
			break;

		case INTS:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(instsCmd, instr.oper1.func));
			if (instr.oper1.fmt == 0)
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaRbRc,
							instr.oper1.ra,
							instr.oper1.rb,
							instr.oper1.rc);
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaLitRc,
							instr.oper2.ra,
							instr.oper2.lit,
							instr.oper2.rc);
			break;

		case INTM:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(instmCmd, instr.oper1.func));
			if (instr.oper1.fmt == 0)
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaRbRc,
							instr.oper1.ra,
							instr.oper1.rb,
							instr.oper1.rc);
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							regRaLitRc,
							instr.oper2.ra,
							instr.oper2.lit,
							instr.oper2.rc);
			break;

		case ITFP:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(itfpCmd, instr.fp.func));
			strLoc = sprintf(
						&instrStr[strLoc],
						regFbFc,
						instr.fp.fb,
						instr.fp.fc);
			break;

		case FLTV:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(fltvCmd, instr.fp.func));
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
					strLoc = sprintf(
								&instrStr[strLoc],
								regFbFc,
								instr.fp.fb,
								instr.fp.fc);
					break;

				default:
					strLoc = sprintf(
								&instrStr[strLoc],
								regFaFbFc,
								instr.fp.fa,
								instr.fp.fb,
								instr.fp.fc);
					break;
			}
			break;

		case FLTI:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(fltiCmd, instr.fp.func));
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
					strLoc = sprintf(
								&instrStr[strLoc],
								regFbFc,
								instr.fp.fb,
								instr.fp.fc);
					break;

				default:
					strLoc = sprintf(
								&instrStr[strLoc],
								regFaFbFc,
								instr.fp.fa,
								instr.fp.fb,
								instr.fp.fc);
					break;
			}
			break;

		case FLTL:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(fltlCmd, instr.fp.func));
			switch(instr.fp.func)
			{
				case AXP_FUNC_CVTLQ:
				case AXP_FUNC_CVTQL:
				case AXP_FUNC_CVTQL_V:
				case AXP_FUNC_CVTQL_SV:
					strLoc = sprintf(
								&instrStr[strLoc],
								regFbFc,
								instr.fp.fb,
								instr.fp.fc);
					break;

				default:
					strLoc = sprintf(
								&instrStr[strLoc],
								regFaFbFc,
								instr.fp.fa,
								instr.fp.fb,
								instr.fp.fc);
					break;
			}
			break;

		case MISC:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(miscCmd, instr.mem.mem.func));
			switch (instr.mem.mem.func)
			{
				case AXP_FUNC_ECB:
				case AXP_FUNC_WH64:
				case AXP_FUNC_WH64EN:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRb,
								instr.mem.rb);
					break;

				case AXP_FUNC_EXCB:
				case AXP_FUNC_MB:
				case AXP_FUNC_TRAPB:
				case AXP_FUNC_WMB:
					strLoc = sprintf(
								&instrStr[strLoc],
								"%s",
								regNone);
					break;

				case AXP_FUNC_FETCH:
				case AXP_FUNC_FETCH_M:
					strLoc = sprintf(
								&instrStr[strLoc],
								reg0Rb,
								instr.mem.rb);
					break;

				case AXP_FUNC_RPCC:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRaRb,
								instr.mem.ra,
								instr.mem.rb);
					break;

				case AXP_FUNC_RC:
				case AXP_FUNC_RS:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRa,
								instr.oper1.ra);
					break;
			}
			break;

		case FPTI:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						AXP_Get_Func_Str(fptiCmd, instr.oper1.func));
			switch (instr.oper1.func)
			{
				case AXP_FUNC_SEXTB:
				case AXP_FUNC_SEXTW:
					if (instr.oper1.fmt == 0)
						strLoc = sprintf(
									&instrStr[strLoc],
									regRbRc,
									instr.oper1.rb,
									instr.oper1.rc);
					else
						strLoc = sprintf(
									&instrStr[strLoc],
									regLitRc,
									instr.oper2.lit,
									instr.oper2.rc);
					break;

				case AXP_FUNC_CTPOP:
				case AXP_FUNC_CTLZ:
				case AXP_FUNC_CTTZ:
				case AXP_FUNC_PKLB:
				case AXP_FUNC_PKWB:
				case AXP_FUNC_UNPKBL:
				case AXP_FUNC_UNPKBW:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRbRc,
								instr.oper1.rb,
								instr.oper1.rc);
					break;

				case AXP_FUNC_PERR:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRaRbRc,
								instr.oper1.ra,
								instr.oper1.rb,
								instr.oper1.rc);
					break;

				case AXP_FUNC_FTOIS:
				case AXP_FUNC_FTOIT:
					strLoc = sprintf(
								&instrStr[strLoc],
								regFaRc,
								instr.fp.fa,
								instr.fp.fc);
					break;

				case AXP_FUNC_MINUB8:
				case AXP_FUNC_MINSB8:
				case AXP_FUNC_MINUW4:
				case AXP_FUNC_MINSW4:
				case AXP_FUNC_MAXUB8:
				case AXP_FUNC_MAXSB8:
				case AXP_FUNC_MAXUW4:
				case AXP_FUNC_MAXSW4:
					if (instr.oper1.fmt == 0)
						strLoc = sprintf(
									&instrStr[strLoc],
									regRaRbRc,
									instr.oper1.ra,
									instr.oper1.rb,
									instr.oper1.rc);
					else
						strLoc = sprintf(
									&instrStr[strLoc],
									regRaLitRc,
									instr.oper1.ra,
									instr.oper2.lit,
									instr.oper2.rc);
					break;
			}
			break;

		case HW_MFPR:
		case HW_MTPR:
			if ((pcAddr.pal == 1) || (kernelMode == false))
			{
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							resOpcode);
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							regNone);
			}
			else
			{
				funcStr = instrCmd[instr.hw_mxpr.opcode];
				if (((instr.hw_mxpr.index & AXP_IPR_PCXT0) == AXP_IPR_PCXT0) ||
					((instr.hw_mxpr.index & AXP_IPR_PCXT1) == AXP_IPR_PCXT1))
					index = instr.hw_mxpr.index & AXP_IPR_PCXT1;
				else
					index = instr.hw_mxpr.index;
				strcpy((char *) AXP_Get_Func_Str(iprFunc, index), iprName);
				sprintf(funcName, "%s_%s", funcStr, iprName);
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							funcName);
				if (instr.hw_mxpr.opcode == HW_MFPR)
					strLoc = sprintf(
								&instrStr[strLoc],
								mxprRegScbd,
								instr.hw_mxpr.ra,
								instr.hw_mxpr.scbd_mask);
				else
					strLoc = sprintf(
								&instrStr[strLoc],
								mxprRegScbd,
								instr.hw_mxpr.rb,
								instr.hw_mxpr.scbd_mask);
			}
			break;

		case HW_LD:
			if ((pcAddr.pal == 1) || (kernelMode == false))
			{
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							resOpcode);
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							regNone);
			}
			else
			{
				funcStr = instrCmd[instr.hw_ld.opcode];
				strcpy(
					(char *) AXP_Get_Func_Str(hwLdCmd, instr.hw_ld.type),
					iprName);
				sprintf(funcName, "%s_%s", funcStr, iprName);
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							funcName);
				strLoc = sprintf(
							&instrStr[strLoc],
							hwRaRbDispLen,
							instr.hw_ld.ra,
							instr.hw_ld.disp,
							instr.hw_ld.rb,
							AXP_Get_Func_Str(hwLen, instr.hw_ld.len));
			}
			break;

		case HW_ST:
			if ((pcAddr.pal == 1) || (kernelMode == false))
			{
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							resOpcode);
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							regNone);
			}
			else
			{
				funcStr = instrCmd[instr.hw_st.opcode];
				strcpy(
					(char *) AXP_Get_Func_Str(hwStCmd, instr.hw_st.type),
					iprName);
				sprintf(funcName, "%s_%s", funcStr, iprName);
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							funcName);
				strLoc = sprintf(
							&instrStr[strLoc],
							hwRaRbDispLen,
							instr.hw_st.ra,
							instr.hw_st.disp,
							instr.hw_st.rb,
							AXP_Get_Func_Str(hwLen, instr.hw_st.len));
			}
			break;

		case HW_RET:
			if ((pcAddr.pal == 1) || (kernelMode == false))
			{
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							resOpcode);
				strLoc = sprintf(
							&instrStr[strLoc],
							"%s",
							regNone);
			}
			else
			{
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							AXP_Get_Func_Str(hwRet, instr.hw_ret.hint));
				if ((instr.hw_ret.hint == AXP_HW_JMP) ||
					(instr.hw_ret.hint == AXP_HW_JSR))
					strLoc = sprintf(
								&instrStr[strLoc],
								hwJmpRb,
								instr.hw_ret.rb,
								hwRetStall[instr.hw_ret.stall]);
				else
					strLoc = sprintf(
								&instrStr[strLoc],
								hwRetRb,
								instr.hw_ret.disp,
								hwRetStall[instr.hw_ret.stall]);
			}
			break;

		case LDS:		/* PREFETCH_M */
		case LDT:		/* PREFETCH_MEN */
			if (instr.mem.ra == 31)
			{
				if (instr.mem.opcode == LDS)
					strLoc = sprintf(
								&instrStr[strLoc],
								instrFmt,
								prefetchCmd[AXP_LDS_PREFETCH]);
				else
					strLoc = sprintf(
								&instrStr[strLoc],
								instrFmt,
								prefetchCmd[AXP_LDT_PREFETCH]);
			}
			else
				strLoc = sprintf(
							&instrStr[strLoc],
							instrFmt,
							instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regFaDispRb,
						instr.mem.ra,
						instr.mem.mem.disp,
						instr.mem.rb);
			break;

		case LDF:
		case LDG:
		case STF:
		case STG:
		case STS:
		case STT:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.mem.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regFaDispRb,
						instr.mem.ra,
						instr.mem.mem.disp,
						instr.mem.rb);
			break;

		case FBEQ:
		case FBLT:
		case FBLE:
		case FBNE:
		case FBGE:
		case FBGT:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.br.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regFaDisp,
						instr.br.ra,
						instr.br.branch_disp);
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
			strLoc = sprintf(&instrStr[strLoc],
				instrFmt,
				instrCmd[instr.br.opcode]);
			strLoc = sprintf(
						&instrStr[strLoc],
						regRaDisp,
						instr.br.ra,
						instr.br.branch_disp);
			break;

		case JMP:
			strLoc = sprintf(
				&instrStr[strLoc],
				instrFmt,
				jmpCmd[AXP_JMP_TYPE(instr.mem.mem.disp)]);
			switch (AXP_JMP_TYPE(instr.mem.mem.disp))
			{
				case AXP_FUNC_RET:
				case AXP_FUNC_JSR_COROUTINE:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRaRb,
								instr.mem.ra,
								instr.mem.rb);
					break;

				default:
					strLoc = sprintf(
								&instrStr[strLoc],
								regRaRbHint,
								instr.mem.ra,
								instr.mem.rb,
								AXP_JMP_HINT(instr.mem.mem.disp));
					break;
			}
			break;
	}
	funcStr = (const char *) &instr;
	strLoc = sprintf(
				&instrStr[strLoc],
				instBinFmt,
				*((u32 *) &instr),
				(isprint(funcStr[3]) ? funcStr[3] : '.'),
				(isprint(funcStr[2]) ? funcStr[2] : '.'),
				(isprint(funcStr[1]) ? funcStr[1] : '.'),
				(isprint(funcStr[0]) ? funcStr[0] : '.'));

	/*
	 * Return back to the caller.
	 */
	return;
}
