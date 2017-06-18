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
 *	This header file contains the function definitions implemented in the
 *	AXP_21264_Ibox.c module.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		01-Jun-2017	Jonathan D. Belanger
 *	Added a function prototype to add an Icache line/block.
 */
#ifndef _AXP_21264_IBOX_DEFS_
#define _AXP_21264_IBOX_DEFS_

#include "AXP_21264_CPU.h"

#define AXP_NONE	0
#define AXP_IQ		1
#define AXP_FQ		2
#define AXP_COND	3

/*
 * Function prototypes
 */
bool AXP_Branch_Prediction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool *localTaken,
				bool *globalTaken,
				bool *choice);
void AXP_Branch_Direction(
				AXP_21264_CPU *cpu,
				AXP_PC vpc,
				bool taken,
				bool localTaken,
				bool globalTaken);
AXP_INS_TYPE AXP_InstructionFormat(AXP_INS_FMT inst);
AXP_CACHE_FETCH AXP_ICacheFetch(AXP_21264_CPU *cpu,
								AXP_PC pc,
								AXP_INS_LINE *next);
void AXP_ICacheAdd(AXP_21264_CPU *cpu,
				   AXP_PC pc,
				   AXP_INS_FMT *nextInst,
				   AXP_ICACHE_ITB *itb);
void AXP_ITBAdd(AXP_21264_CPU *cpu,
				AXP_IBOX_ITB_TAG itbTag,
				AXP_IBOX_ITB_PTE *itbPTE);

void AXP_21264_IboxMain(AXP_21264_CPU *);

/*
 * PALcode Exception Entry Points
 * 	When hardware encounters an exception, Ibox execution jumps to a PALcode
 * 	entry point at a PC determined by the type of exception.  The return PC of
 * 	the instruction the triggered the exception is placed in the EXC_ADDR
 * 	register and onto the return prediction stack.  The entry points are listed
 * 	in decreasing order of priority.
 *
 *		Entry Name		Type		Offset	Description
 *		DTBM_DOUBLE_3	Fault		0x100	Dstream TB miss on virtual page table
 *											entry fetch.  Use three-level flow.
 *		DTBM_DOUBLE_4	Fault		0x180	Dstream TB miss on virtual page table
 *											entry fetch.  Use four-level flow.
 *		FEN				Fault		0x200	Floating point disabled.
 *		UNALIGNED		Fault		0x280	Unaligned Dstream reference.
 *		DTBM_SINGLE		Fault		0x300	Dstream TB miss.
 *		DFAULT			Fault		0x380	Dstream fault or virtual address sign
 *											check error.
 *		OPCDEC			Fault		0x400	Illegal opcode or function field:
 *											- Opcode 1, 2, 3, 4, 5, 6 or 7
 *											- Opcode 0x19, 0x1b, 0x1d, 0x1e or
 *											  0x1f not PALmode or not
 *											  I_CTL[HWE]
 *											- Extended precision IEEE format
 *											- Unimplemented function field of
 *											  opcodes 0x14 or 0x1c.
 *		IACV			Fault		0x480	Istream access violation or virtual
 *											address sign check error.
 *		MCHK			Interrupt	0x500	Machine check.
 *		ITB_MISS		Fault		0x580	Istream TB miss.
 *		ARITH			Synch. Trap	0x600	Arithmetic exception or update to
 *											FPCR.
 *		INTERRUPT		Interrupt	0x680	Interrupt: hardware, software, and
 *											AST.
 *		MT_FPCR			Synch. Trap	0x700	Invoked when an MT_FPCR instruction
 *											is issued.
 *		RESET/WAKEUP	Interrupt	0x780	Chip reset or wake-up from sleep
 *											mode.
 */
#define AXP_DTBM_DOUBLE_3	0x0100
#define AXP_DTBM_DOUBLE_4	0x0180
#define AXP_FEN				0x0200
#define AXP_UNALIGNED		0x0280
#define AXP_DTBM_SINGLE		0x0300
#define AXP_DFAULT			0x0380
#define AXP_OPCDEC			0x0400
#define AXP_IACV			0x0480
#define AXP_MCHK			0x0500
#define AXP_ITB_MISS		0x0580
#define AXP_ARITH			0x0600
#define AXP_INTERRUPT		0x0680
#define AXP_MT_FPCR_TRAP	0x0700
#define AXP_RESET_WAKEUP	0x0780

#endif /* _AXP_21264_IBOX_DEFS_ */
