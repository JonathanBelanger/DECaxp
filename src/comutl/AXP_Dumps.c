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
 */
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_21264_Instructions.h"

typedef struct
{
	u16		func;
	char	*cmd;
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
	{AXP_FUNC_SQRTF_U, "SQRTF/U"},
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

const char *instrFmt 	= "%-13s ";
const char *regRaDispRb = "R%02d, %10d(R%02d) ";	/* 'R01, -4294901760(R01) ' (22) */
const char *regRaDisp 	= "R%02d, %7d         ";	/* 'R01, -2097151         ' (22) */
const char *regRaRbHint = "R%02d, (R%02d), %4d     ";/* 'R01, (R01), -8191     ' (22) */
const char *regRaRb     = "R%02d, (R%02d)          ";/* 'R01, (R01)            ' (22) */
const char *regRaRbRc	= "R%02d, R%02d, R%02d         ";/* 'R01, R01, R01         ' (22) */
const char *regRaLitRc	= "R%02d, \#%03d, R%02d        ";/* 'R01, #255, R01        ' (22) */
const char *regFaFbFc	= "F%02f, F%02d, F%02d         ";/* 'F01, F01, F01         ' (22) */
const char *regFbFc		= "F%02d, F%02d              ";/* 'F01, F01              ' (22) */

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
const char *AXP_Get_Func_Str(AXP_FUNC_CMD *axpFuncCmd, u16 funcVal)
{
	char 	*retVal = NULL;
	int		ii;

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
 * 	instr:
 * 		The value of a single Digital Alpha AXP instruction to be decoded.
 * 	PALmode:
 * 		A boolean indicating that the instruction can be decoded as if the
 * 		instruction was executing in PALmode.
 *
 * Output Parameters:
 * 	instrStr:
 * 		A string containing the decoded instruction.
 *
 * Return Value:
 * 	true:	The instruction was successfully converted.
 * 	false:	The instruction was not a valid Digital Alpha AXP instruction.
 */
bool AXP_Decode_Instruction(AXP_INS_FMT instr, bool PALmode, char *instrStr)
{
	bool	retVal = true;
	int		strLoc = 0;

	switch(instr.pal.opcode)
	{
		case PAL00:
			strLoc = sprintf(
						&instrStr[strLoc],
						instrFmt,
						instrCmd[instr.pal.opcode]);
			break;

		case OPC01:
		case OPC02:
		case OPC03:
		case OPC04:
		case OPC05:
		case OPC06:
		case OPC07:
			instrStr[0] = '\0';
			retVal = false;
			break;

		case LDA:
		case LDBU:
		case LDL:
		case LDQ:
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
						AXP_Get_Func_Str(instlCmd, instr.oper1.func));
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
		case FLTL:
		case MISC:
			break;

		case FPTI:
			break;

		case HW_MFPR:
		case HW_LD:
		case HW_MTPR:
		case HW_RET:
		case HW_ST:
			if (PALmode == false)
			{
				instrStr[0] = '\0';
				retVal = false;
			}
			break;

		case LDF:
		case LDG:
		case LDS:
		case LDT:
		case STF:
		case STG:
		case STS:
		case STT:
		case FBEQ:
		case FBLT:
		case FBLE:
		case FBNE:
		case FBGE:
		case FBGT:
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

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}
