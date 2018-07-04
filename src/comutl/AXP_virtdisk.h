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
 *  This header file contains the definitions in virtdisk.h, required to
 *  implement the virtual disk functions defined by Microsoft.
 *
 * Revision History:
 *
 *  V01.000	02-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef AXP_VIRTDISK_H_
#define AXP_VIRTDISK_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_vdssys.h"

typedef enum
{
    RESIZE_NONE,
    RESIZE_ALLOW_UNSAFE_VIRT_SIZE,
    RESIZE_RESIZE_SMALLEST_SAFE_SIZE
} AXP_VHD_RESIZE_FLAG;
typedef enum
{
    RESIZE_VER_UNSPECIFIED,
    RESIZE_VER_1
} AXP_VHD_RESIZE_VER;

typedef struct
{
    AXP_VHD_RESIZE_VER ver;
    union
    {
	struct
	{
	    u64 newSize;
	} ver_1;
    };
} AXP_VHD_RESIZE_PARAM;

/*
 * Function Prototypes
 *
 * Attaches a parent to a virtual disk opened with the OPEN_CUSTOM_DIFF_CHAIN
 * flag.
 */
u32 AXP_VHD_AddParent(AXP_VHD_HANDLE handle, char *parentPath);

/*
 * Deletes metadata from a virtual disk.
 */
u32 AXP_VHD_DeleteMeta(AXP_VHD_HANDLE handle, const AXP_VHDX_GUID *item);

/*
 * Enumerates the metadata associated with a virtual disk.
 */
u32 AXP_VHD_EnumerateMeta(
		AXP_VHD_HANDLE handle,
		u32 *numItems,
		AXP_VHDX_GUID *items);

/*
 * Retrieves the specified metadata from the virtual disk.
 */
u32 AXP_VHD_GetMeta(
		AXP_VHD_HANDLE handle,
		const AXP_VHDX_GUID *item,
		u32 *metaSize,
		void *metaData);

/*
 * Resizes a virtual disk.
 */
u32 AXP_VHD_Resize(
		AXP_VHD_HANDLE handle,
		AXP_VHD_RESIZE_FLAG plags,
		AXP_VHD_RESIZE_PARAM *param,
		AXP_VHD_ASYNC *async);

/*
 * Sets a metadata item for a virtual disk.
 */
u32 AXP_VHD_SetMeta(
		AXP_VHD_HANDLE handle,
		const AXP_VHDX_GUID *item,
		u32 metaSize,
		const void *metaData);

#endif /* AXP_VIRTDISK_H_ */
