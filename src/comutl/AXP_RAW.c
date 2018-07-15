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
 *  This source file contains the functions required to support accessing
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
#include "AXP_RAW.h"

/*
 * _AXP_RAW_Create
 *  Creates a RAW hard disk (RAW) image file.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the new RAW
 *	disk image file.
 *  flags:
 *	Creation flags, which must be a valid combination of the
 *	AXP_VHD_CREATE_FLAG enumeration.
 *  parentPath:
 *	A pointer to a valid string that represents the path to the parent
 *	virtual disk image file.  This means that we are creating a
 *	differential RAW.
 *  parentDevID:
 *	An unsigned 32-bit value indicating the disk type of the parent.
 *  diskSize:
 *	An unsigned 64-bit value for the size of the disk to be created, in
 *	bytes.
 *  blkSize:
 *	An unsigned 32-bit value for the size of each block.
 *  sectorSize:
 *	An unsigned 32-bit value for the size of each sector.
 *  deviceID:
 *	An unsigned 32-bit value indicating the desired disk type.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly created
 *  	RAW disk.
 *
 * Return Values:
 *  AXP_VHD_CALL_NOT_IMPL:	This function is not implemented.
 */
u32 _AXP_RAW_Create(
		char *path,
		AXP_VHD_CREATE_FLAG flags,
		char *parentPath,
		u32 parentDevID,
		u64 diskSize,
		u32 blkSize,
		u32 sectorSize,
		u32 deviceID,
		AXP_VHD_HANDLE *handle)
{
    u32		retVal = AXP_VHD_CALL_NOT_IMPL;

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_RAW_Open
 *  This function is called to open a RAW device or CD.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the new RAW
 *	disk image file.
 *  flags:
 *	Open flags, which must be a valid combination of the AXP_VHD_OPEN_FLAG
 *	enumeration.
 *  deviceID:
 *	An unsigned 32-bit value indicating the disk type being opened.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly opened
 *  	RAW disk.
 */
u32 _AXP_RAW_Open(
		char *path,
		AXP_VHD_OPEN_FLAG flags,
		u32 deviceID,
		AXP_VHD_HANDLE *handle)
{
    u32		retVal = AXP_VHD_SUCCESS;

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}
