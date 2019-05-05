/*
 * Copyright (C) Jonathan D. Belanger 2018.
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
 *	This header file contains the function prototypes for the initialization
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written from AXP_21264_Ibox.c.
 */
#ifndef _AXP_IBOX_INIT_DEFS_
#define _AXP_IBOX_INIT_DEFS_	1

#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Utility.h"

void AXP_21264_Ibox_ResetRegMap(AXP_21264_CPU *);
bool AXP_21264_Ibox_Init(AXP_21264_CPU *);

#endif	/* _AXP_IBOX_INIT_DEFS_ */
