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
    AXP_RAW_Handle	*raw;
    i64			fileSize;
    size_t		outLen;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * Let's allocate the block we need to maintain access to the physical disk
     * image.
     */
    raw = (AXP_RAW_Handle *) AXP_Allocate_Block(AXP_RAW_BLK);
    if (raw != NULL)
    {

	/*
	 * Allocate a buffer long enough for for the filename (plus null
	 * character).
	 */
	raw->filePath = AXP_Allocate_Block(-(strlen(path) + 1));
	if (raw->filePath != NULL)
	{
	    strcpy(raw->filePath, path);
	    raw->deviceID = deviceID;

	    /*
	     * Open the device/file.  If it is an ISO file or a CDROM device,
	     * then do some initialization and we are done.  If it is a
	     * physical device, then we re-open the device for binary
	     * read/write.
	     */
	    raw->fp = fopen(path, "rb");
	    if (raw->fp != NULL)
	    {
		raw->deviceID = deviceID;
		if (deviceID == STORAGE_TYPE_DEV_ISO)
		{
		    raw->readOnly = true;
		    raw->diskSize = 0;
		    raw->blkSize = 0;
		    raw->sectorSize = 0;
		    raw->cylinders = 0;
		    raw->heads = 0;
		    raw->sectors = 0;
		}
		else
		{
		    raw->readOnly = true;
		    raw->diskSize = 0;
		    raw->blkSize = 0;
		    raw->sectorSize = 0;
		    raw->cylinders = 0;
		    raw->heads = 0;
		    raw->sectors = 0;
		}
	    }
	    else
		retVal = AXP_VHD_FILE_NOT_FOUND;
	}
	else
	    retVal = AXP_VHD_OUTOFMEMORY;
    }
    else
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we don't have a success at this point, and we allocated a VHD
     * handle, then deallocate the handle, since the VHD was not successfully
     * opened.
     */
    if ((retVal != AXP_VHD_SUCCESS) && (raw != NULL))
	AXP_Deallocate_Block(raw);

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}
