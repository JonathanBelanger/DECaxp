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
 *  This header file contains the definitions needed to support a solid state
 *  disk (SSD).
 *
 * Revision History:
 *
 *  V01.000	05-Aug-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef AXP_SSD_H_
#define AXP_SSD_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"

typedef struct
{
    FILE	*fp;
    u8		*memory;
} AXP_SSD_Handle;

#endif /* AXP_SSD_H_ */
