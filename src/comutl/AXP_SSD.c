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
 *  This source file contains the code needed to support a solid state disk
 *  (SSD).
 *
 * Revision History:
 *
 *  V01.000	05-Aug-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Blocks.h"
#include "AXP_SSD.h"

/*
 * _AXP_SSD_Create
 *  Creates a solid state disk (SSD) image file.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the backing
 *	store file.
 *  flags:
 *	Creation flags, which must be a valid combination of the
 *	AXP_VHD_CREATE_FLAG enumeration.
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
 *  	virtual disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_FILE_EXISTS:	File already exists.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
 *  AXP_VHD_WRITE_FAULT:	An error occurred writing to the VHDX file.
 *  AXP_VHD_OUTOFMEMORY:	Insufficient memory to perform operation.
 */
u32 _AXP_SSD_Create(
		char *path,
		AXP_VHD_CREATE_FLAG flags,
		u64 diskSize,
		u32 blkSize,
		u32 sectorSize,
		u32 deviceID,
		AXP_VHD_HANDLE *handle)
{
    AXP_SSD_Handle	*ssd;
    AXP_SSD_Geometry	header;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    ssd = (AXP_SSD_Handle *) AXP_Allocate_Block(AXP_SSD_BLK);
    if (ssd != NULL)
    {

	ssd->memory = (u8 *) AXP_Allocate_Block(-diskSize);
	if (ssd->memory != NULL)
	{

	    /*
	     * Allocate a buffer long enough for for the filename (plus null
	     * character).
	     */
	    ssd->filePath = AXP_Allocate_Block(-(strlen(path) + 1));
	    if (ssd->filePath != NULL)
	    {
		strcpy(ssd->filePath, path);
		ssd->deviceID = deviceID;
		ssd->diskSize = diskSize;
		ssd->blkSize = blkSize;
		ssd->sectorSize = sectorSize;
		ssd->cylinders = 0;
		ssd->heads = 0;
		ssd->sectors = 0;
	    }
	    else
		retVal = AXP_VHD_OUTOFMEMORY;
	}
	else
	    retVal = AXP_VHD_OUTOFMEMORY;
    }
    else
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we still have a success status, go create the backing store file.
     * If it already exists, we return an error.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	/*
	 * TODO: Try and open the file for read.
	 */
    }
    if (retVal == AXP_VHD_SUCCESS)
	*handle = (AXP_VHD_HANDLE) ssd;
    else
	AXP_Deallocate_Block(ssd);

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}

