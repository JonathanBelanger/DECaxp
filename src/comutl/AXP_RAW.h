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
 *  This header file contains the definitions required to support accessing
 *  either a device (disk) or CD in its raw form.  We do this for devices
 *  because we want those devices to ultimately look and be formatted just like
 *  the real thing.  We do this for CDs, because the format for these has been
 *  predetermined and is a standard, so let the operating system in the
 *  emulator handle the details for both of these disk types.
 *
 * Revision History:
 *
 *  V01.000	15-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef AXP_RAW_H_
#define AXP_RAW_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_VirtualDisk.h"

u32 _AXP_RAW_Create(
		char *,
		AXP_VHD_CREATE_FLAG,
		char *,
		u32,
		u64,
		u32,
		u32,
		u32,
		AXP_VHD_HANDLE *);
u32 _AXP_RAW_Open(char *, AXP_VHD_OPEN_FLAG, u32, AXP_VHD_HANDLE *);

#endif /* AXP_RAW_H_ */
