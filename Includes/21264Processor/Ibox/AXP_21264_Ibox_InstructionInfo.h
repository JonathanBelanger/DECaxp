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
 *	This header file contains the functions needed by the other Ibox modules
 *	to decode instructions.
 *
 * Revision History:
 *
 *	V01.000		22-Jun-2017	Jonathan D. Belanger
 *	Initially written, migrated from AXP_21264_Ibox.c.
 */
#ifndef _AXP_21264_IBOX_INSTRUCTION_INFO_DEFS_
#define _AXP_21264_IBOX_INSTRUCTION_INFO_DEFS_

#include "CommonUtilities/AXP_Utility.h"
#include "21264Processor/AXP_21264_Instructions.h"
#include "21264Processor/Ibox/AXP_21264_RegisterRenaming.h"
#include "21264Processor/Ibox/AXP_21264_Ibox.h"

/*
 * Prototype definitions.
 */
void AXP_Dispatcher(AXP_21264_CPU *, AXP_INSTRUCTION *);
AXP_INS_TYPE AXP_InstructionFormat(AXP_INS_FMT);
AXP_OPER_TYPE AXP_OperationType(u32);
AXP_REG_DECODE AXP_RegisterDecoding(u32);
u16 AXP_InstructionQueue(u32);
AXP_PIPELINE AXP_InstructionPipeline(u32, u32);

#endif	/* _AXP_21264_IBOX_INSTRUCTION_INFO_DEFS_ */
