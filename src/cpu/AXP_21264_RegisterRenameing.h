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
 *	This header file contains definitions required to implement register
 *	renaming for the Alpha AXP 21264 CPU.
 *
 * Revision History:
 *
 *	V01.000		10-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_REGISTER_RENAMING_DEFS_
#define _AXP_REGISTER_RENAMING_DEFS_

/*
 * General Map Table (GMT)
 */
typedef struct
{
	u8	pReg;
	u8	refCnt;
} AXP_21264_GMT;

#endif /* _AXP_REGISTER_RENAMING_DEFS_ */
