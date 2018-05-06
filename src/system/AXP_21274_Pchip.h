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
 *	This header file contains the definitions required for the Tsunami/Typhoon
 *	Pchip emulation.
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_PCHIP_H_
#define _AXP_21274_PCHIP_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_21274_Registers.h"
#include "AXP_21274_Cchip.h"
#include "AXP_21274_Dchip.h"

/*
 * Pchip Function Prototypes
 */
void AXP_21274_PchipInit(AXP_21274_SYSTEM *);
void AXP_21264_ReadPCI(
					AXP_21274_SYSTEM *,
					AXP_21274_RQ_ENTRY *,
					AXP_21274_SYSBUS_CPU *);
void AXP_21264_WritePCI(
					AXP_21274_SYSTEM *,
					AXP_21274_RQ_ENTRY *,
					AXP_21274_SYSBUS_CPU *);

#endif /* _AXP_21274_PCHIP_H_ */
