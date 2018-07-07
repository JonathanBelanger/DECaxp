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
 *  This header file contains the definitions needed by its source companion,
 *  which contains utility functions utilized by the VHD functionality.
 *
 * Revision History:
 *
 *  V01.000	07-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VHD_UTILITY_H_
#define _AXP_VHD_UTILITY_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_virtdisk.h"

/*
 * This enumeration is used to select the correct well known GUIDs.
 */
typedef enum
{
    AXP_Vendor_Microsoft,
    AXP_Vendor_Unknown,
    AXP_Zero_GUID = AXP_Vendor_Unknown,
    AXP_File_Parameter,
    AXP_Disk_Size,
    AXP_Page_83,
    AXP_Logical_Sector,
    AXP_Physical_Sector,
    AXP_Parent_Locator,
    AXP_Block_Allocation_Table,
    AXP_Metadata_Region,
    AXP_Known_MAX
} AXP_VHD_KnownGUIDs;

/*
 * Define the function prototypes.
 */
void AXP_VHD_CopyGUID(AXP_VHDX_GUID *, AXP_VHDX_GUID *);
bool AXP_VHD_CompareGUID(AXP_VHDX_GUID *, AXP_VHDX_GUID *);
AXP_VHD_KnownGUIDs AXP_VHD_KnownGUID(AXP_VHDX_GUID *);
void AXP_VHD_SetGUIDMemory(AXP_VHDX_GUID *);
void AXP_VHD_SetGUIDDisk(AXP_VHDX_GUID *);
void AXP_VHD_KnownGUIDMemory(AXP_VHD_KnownGUIDs, AXP_VHDX_GUID *);
void AXP_VHD_KnownGUIDDisk(AXP_VHD_KnownGUIDs, AXP_VHDX_GUID *);
u64 AXP_VHD_PerformFileSize(FILE *fp);
u32 AXP_VHD_ValidateCreate(
		AXP_VHD_STORAGE_TYPE *,
		char *,
		AXP_VHD_ACCESS_MASK,
		AXP_VHD_CREATE_FLAG,
		AXP_VHD_CREATE_PARAM *,
		AXP_VHD_HANDLE *,
		u64 *,
		u32 *,
		u32 *,
		u32 *);
void AXP_Dump_VHD_Info(AXP_VHD_HANDLE);

#endif /* _AXP_VHD_UTILITY_H_ */
