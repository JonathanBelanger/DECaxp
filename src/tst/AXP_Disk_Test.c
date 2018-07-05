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
 *  This module contains the code to test disk emulation.
 *
 * Revision History:
 *
 *  V01.000	01-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Trace.h"
#include "AXP_virtdisk.h"

int main(void)
{
    AXP_VHD_CREATE_PARAM 	createParam;
    AXP_VHD_STORAGE_TYPE	storageType;
    AXP_VHD_HANDLE		handle;
    char			*diskPath = "/cygdrive/g/git/DECaxp/src/tst/VHDTests";
    char			*diskName = "RZ1CD-CS.vhdx";
    char			fullPath[256];
#if 0
    char			*modelNumber = "ST34501WC";
    char			*serialNumber = "LG564729";
    char			*revisionNumber = "A02";
#endif
    u32				retVal = AXP_VHD_SUCCESS;

    printf("\nDECaxp Disk Testing...\n");

    createParam.ver = CREATE_VER_1;
    uuid_clear(createParam.ver_1.GUID.uuid);
    createParam.ver_1.maxSize = 3 * ONE_M;
    createParam.ver_1.blkSize = AXP_VHD_DEF_BLK;
    createParam.ver_1.sectorSize = AXP_VHD_DEF_SEC;
    createParam.ver_1.parentPath = NULL;
    createParam.ver_1.srcPath = NULL;

    storageType.deviceID = STORAGE_TYPE_DEV_VHDX;
    STORAGE_TYPE_VENDOR_MSFT(storageType.vendorID);

    printf(
	"\nTest 1: Create a VHDX(v%d) disk in %s with the name of %s of %lld "
	"bytes in size...\n",
	createParam.ver,
	diskPath,
	diskName,
	createParam.ver_1.maxSize);
    sprintf(fullPath, "%s/%s", diskPath, diskName);

    retVal = AXP_VHD_Create(
			&storageType,
			fullPath,
			ACCESS_NONE,
			NULL,
			CREATE_NONE,
			0,
			&createParam,
			NULL,
			&handle);
    if (retVal == AXP_VHD_SUCCESS)
    {
	printf("\t...Succeeded...\n");
	AXP_VHD_CloseHandle(handle);
    }
    else
	printf("\t...Failed...\n");

    /*
     * Return back to the caller.
     */
    printf("...Done.\n");
    return(0);
}

