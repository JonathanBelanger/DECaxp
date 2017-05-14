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
 *	This header file contains useful definitions to be used throughout the
 *	Digital Alpha AXP emulation software that are common to all Alpha AXP
 *	Processors.
 *
 *	Revision History:
 *
 *	V01.000	 10-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 */
#ifndef _AXP_BASE_CPU_DEFS_
#define _AXP_BASE_CPU_DEFS_

#include "AXP_Utility.h"

#define AXP_MAX_REGISTERS	32

/*
 * Program Counter (PC) Definition
 */
typedef struct
{
	u64 pal : 1;
	u64 res : 1;
	u64 pc  : 62;
} AXP_PC;

#endif /* _AXP_BASE_CPU_DEFS_ */
