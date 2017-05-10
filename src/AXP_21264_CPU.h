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
 *	This header file contains the structures and definitions required to
 *	implement the instruction emulation for the Alpha 21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000		04-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(May the 4th be with you).
 */
#ifndef _AXP_CPU_DEFS_
#define _AXP_CPU_DEFS_

#include "AXP_Utility.h"
#include "AXP_Predictions.h"

typedef struct
{

	/*
	 * The following definitions are used by the branch prediction code.
	 */
	LHT	localHistoryTable;
	LPT	localPredictor;
	GPT	globalPredictor;
	CPT	choicePredictor;
	u16	globalPathHistory;
} AXP_21264_CPU;

#endif /* _AXP_CPU_DEFS_ */