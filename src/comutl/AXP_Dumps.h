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
 *	This header file contains the prototypes needed to call the dump functions
 *	for the Alpha AXP Emulator.
 *
 *	Revision History:
 *
 *	V01.000		06-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 *
 */
#ifndef _AXP_DUMPS_DEFS_
#define _AXP_DUMPS_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_Instructions.h"

void AXP_Decode_Instruction(AXP_PC *pc, AXP_INS_FMT, bool, char *);
void AXP_Dump_Registers(AXP_INSTRUCTION *, u64 *, u64 *, char *);

#endif	/* _AXP_DUMPS_DEFS_ */
