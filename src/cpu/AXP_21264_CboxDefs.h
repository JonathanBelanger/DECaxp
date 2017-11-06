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
 *	AXP_21264_Cbox.c module.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		12-Oct-2017	Jonathan D. Belanger
 *	Change the CTAG structure to contain the DTAG index for the corresponding
 *	entry.  This should make keeping these 2 arrays, along with the Dcache
 *	itself, in synch.  NOTE: The set information is the same for all.
 *
 *	V01.002		14-Oct-2017	Jonathan D. Belanger
 *	Started defining the CPU to System and System to CPU messages.
 *
 *	V01.003		22-Oct-2017	Jonathan D. Belanger
 *	Differentiated between messages that come over the SysAddIn_L[14:0] pins
 *	and SysAddOut_L[14:0] pins.  Also defined a number of the remaining
 *	structures needed by the Cbox,
 *
 *	V01.004		06-Nov-2017	Jonathan D. Belanger
 *	Split this fine into 2 separate files.  One to include the definitions for
 *	the Cbox CSRs and one to be included by AXP_21264_Cbox.c so that it will
 *	compile cleanly.  This file is the one that will allow the source module
 *	to compile cleanly.
 */
#ifndef _AXP_21264_CBOX_DEFS_DEFS_
#define _AXP_21264_CBOX_DEFS_DEFS_

#include "AXP_Configure.h"
#include "AXP_21264_CPU.h"

#endif /* _AXP_21264_CBOX_DEFS_DEFS_ */

