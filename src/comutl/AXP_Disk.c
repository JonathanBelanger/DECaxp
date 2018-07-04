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
 *  This source file contains the code to emulate a physical disk drive.  This
 *  code does not care how the disk is controlled, that code will be layered on
 *  top of this code.  All this code does is open, close, seek, read, and write
 *  to the emulated disk.
 *
 * Revision History:
 *
 *  V01.000	30-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_Disk.h"

/*
 * AXP_Disk_Create
 *  This function is called to create a VHDX formatted file to contain the data
 *  needed to support a virtual hard disk.  This is a standard format defined
 *  by Microsoft and used in Hyper-V.
 *
 * Input Parameters:
 *  filePath:
 *	A pointer to a null-terminated string containing the file name, with
 *	path, where the VHD will be located.
 *  sectors:
 *	A value representing the total number of sectors present in the disk
 *	image.
 *  blkSize:
 *	A value representing the block size in the disk image.  The sectors per
 *	block must always be a power of 2.
 *  fixed:
 *	A boolean to indicate if this disk image is a fixed or
 *	dynamic/difference disk.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  !NULL:	Normal Successful Completion.
 *  NULL:	Failed to create VHD.
 */
AXP_Disk *AXP_Disk_Create(
		const char *filePath,
		u32 sectors,
		u32 blkSize,
		bool fixed)
{
    AXP_Disk		*retVal = NULL;

    /*
     * Return the handle to the disk back to the caller.
     */
    return(retVal);
}
