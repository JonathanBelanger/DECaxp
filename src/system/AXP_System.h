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
 *	This header file contains the function prototypes for the System interface.
 *
 *	Revision History:
 *
 *	V01.000		31-Dec-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_SYSTEM_DEFS_
#define _AXP_SYSTEM_DEFS_	1

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_System_InterfaceDefs.h"

void AXP_System_CommandSend(
					AXP_21264_TO_SYS_CMD, bool, int, bool,
					u64, bool, u64, u8 *, int);
void AXP_System_ProbeResponse(bool, bool, u8, bool, u8, AXP_21264_PROBE_STAT);

#endif	/* _AXP_SYSTEM_DEFS_ */
