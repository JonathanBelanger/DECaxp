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

#include "AXP_Blocks.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_ICache.h"
#include "AXP_21264_Ibox.h"

/*
 * Prototype definitions.
 */
AXP_INS_TYPE AXP_InstructionFormat(AXP_INS_FMT);
AXP_OPER_TYPE AXP_OperationType(AXP_INS_FMT);
AXP_REG_DECODE AXP_RegisterDecoding(AXP_INS_FMT);
u16 AXP_InstructionQueue(AXP_INS_FMT);

#endif	/* _AXP_21264_IBOX_INSTRUCTION_INFO_DEFS_ */