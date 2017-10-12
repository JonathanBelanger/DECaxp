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
 *	entry.  This should make keeping these 2 arrays, along with the Dcahe
 *	itself, in synch.  NOTE: The set information is the same for all.
 */
#ifndef _AXP_21264_CBOX_DEFS_
#define _AXP_21264_CBOX_DEFS_

#include "AXP_Utility.h"

/*
 * This structure is the definition for one Cbox copy of Dcache Tag Array.  A
 * block contains the following:
 *
 *		Virtual tag bits
 *		Index into corresponding DTAG array (which is the same as the Dcache)
 *		Valid bit
 *			The CTAG array entry is in use.
 */
typedef struct
{
	u64					virtTag;
	u32					dtagIndex;
	bool				valid;
} AXP_CTAG_BLK;


#endif /* _AXP_21264_CBOX_DEFS_ */
