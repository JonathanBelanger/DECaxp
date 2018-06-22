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
 *	This header file contains definitions needed by the AXP_Exceptions.c
 *	module.
 *
 *	Revision History:
 *
 *	V01.000		05-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_EXCEPTIONS_DEFS_
#define _AXP_EXCEPTIONS_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_Instructions.h"

void AXP_SetException(AXP_INSTRUCTION *, u32);

#define AXP_EXC_SW_COMPL		0x01
#define AXP_EXC_INV_OPER		0x02
#define AXP_EXC_DIV_BY_ZERO		0x04
#define AXP_EXC_FP_OVERFLOW		0x08
#define AXP_EXC_UNDERFLOW		0x10
#define AXP_EXC_INEXACT_RES		0x20
#define AXP_EXC_INT_OVERFLOW	0x40

#define AXP_IEEE_INV_OPER	0x00000010000000000000ll
#define AXP_IEEE_DIV_BY_ZERO	0x00000020000000000000ll
#define AXP_IEEE_FP_OVERFLOW	0x00000040000000000000ll
#define AXP_IEEE_UNDERFLOW	0x00000080000000000000ll
#define AXP_IEEE_INEXACT_RES	0x00000100000000000000ll
#define AXP_IEEE_INT_OVERFLOW	0x00000200000000000000ll

#endif /* _AXP_EXCEPTIONS_DEFS_ */
