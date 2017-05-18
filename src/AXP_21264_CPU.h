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
 *	This header file contains the structures and definitions required to
 *	implement the instruction emulation for the Alpha 21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000		04-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(May the 4th be with you).
 *
 *	V01.001		14-May-2017	Jonathan D. Belanger
 *	Included the AXP_Base_CPU header file and a location for the Program
 *	Counter (PC) in the CPU record.
 */
#ifndef _AXP_21264_CPU_DEFS_
#define _AXP_21264_CPU_DEFS_

#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_Base_CPU.h"
#include "AXP_21264_Predictions.h"
#include "AXP_21264_Instructions.h"

#define AXP_RESULTS_REG		41
#define AXP_NUM_FETCH_INS	4
#define AXP_IQ_LEN			20
#define AXP_FQ_LEN			15
#define AXP_SHADOW_REG		8
#define AXP_R04_SHADOW		(AXP_MAX_REGISTERS + 0)
#define AXP_R05_SHADOW		(AXP_MAX_REGISTERS + 1)
#define AXP_R06_SHADOW		(AXP_MAX_REGISTERS + 2)
#define AXP_R07_SHADOW		(AXP_MAX_REGISTERS + 3)
#define AXP_R20_SHADOW		(AXP_MAX_REGISTERS + 4)
#define AXP_R21_SHADOW		(AXP_MAX_REGISTERS + 5)
#define AXP_R22_SHADOW		(AXP_MAX_REGISTERS + 6)
#define AXP_R23_SHADOW		(AXP_MAX_REGISTERS + 7)
#define AXP_ITB_LEN			128

/*
 * TODO: 	We probably want to decode to determine things like, opcode type,
 *			functional unit (U0, L0, U1, L1, F0, F1), etc.
 */
typedef struct
{
	AXP_INS_FMT	instructions[AXP_NUM_FETCH_INS];
	u8			br_pred : 1;
	u8			line_pred : 1;
	u8			res_1 : 6;		/* align to the byte boundary */
} AXP_INS_QUE;

typedef struct
{
	/*
	 * This structure needs to be at the top of all data blocks/structures
	 * that need to be specifically allocated by the Blocks module.
	 */
	AXP_BLOCK_DSC header;

	/**************************************************************************
	 *	Ibox Definitions													  *
	 **************************************************************************/

	 /*
	 * The following definitions are used by the branch prediction code.
	 */
	LHT			localHistoryTable;
	LPT			localPredictor;
	GPT			globalPredictor;
	CPT			choicePredictor;
	u16			globalPathHistory;

	/*
	 * Architectural (virtual) registers.
	 */
	u64			r[AXP_MAX_REGISTERS + AXP_SHADOW_REG];	/* Integer (Virt) Reg */
	u64 		f[AXP_MAX_REGISTERS];	/* Floating-point (Virtual) Registers */

	/*
	 * Virtual Program Counter Queue
	 */
	AXP_PC		vpc[AXP_IQ_LEN];
	int			vpcIdx;

	/*
	 * Physical registers.
	 *
	 * There are 80 register file entries for integer registers.  This is the
	 * 31 Integer registers (R31 is not stored), plus the 8 shadow registers,
	 * plus the 41 results for instructions that can potentially have not been
	 * retired.
	 *
	 * Since the integer execution unit has 2 cluster, there are a set of 80
	 * register files for each.
	 *
	 * There are 72 register file entries for the floating-point registers.
	 * This is the 31 Floating-point registers (F31 is not stored), plus the 41
	 * results for instructions that can potentially have not been retired.
	 */
	u64			pr0[AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1];
	u64			pr1[AXP_MAX_REGISTERS + AXP_SHADOW_REG + AXP_RESULTS_REG - 1];
	u64			pf[AXP_MAX_REGISTERS + AXP_RESULTS_REG - 1];
	
	char		itb[AXP_ITB_LEN];

	/*
	 * Instruction Queues (Integer and Floating-Point).
	 */
	AXP_INS_QUE	iq[AXP_IQ_LEN];
	AXP_INS_QUE	fq[AXP_FQ_LEN];

	/*
	 * All the IPRs
	 */
} AXP_21264_CPU;

#endif /* _AXP_21264_CPU_DEFS_ */