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
 *	This header file contains the prototypes needed to call the utilities to
 *	process a file containing a name/value pair.
 *
 *	Revision History:
 *
 *	V01.000		06-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_NAME_VALUE_PAIR_READ_DEFS_
#define _AXP_NAME_VALUE_PAIR_READ_DEFS_
#include "AXP_Utility.h"

FILE *AXP_Open_NVP_File(char *);
void AXP_Close_NVP_File(FILE *);
bool AXP_Read_NVP_File(FILE *, char *, u32 *);

#endif	/* _AXP_NAME_VALUE_PAIR_READ_DEFS_ */
