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
 *  This header file contains the definitions required to be able to utilize
 *  a file, a device, or memory as an emulated physical disk.
 *
 * Revision History:
 *
 *  V01.000	30-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_DISK_H_
#define _AXP_DISK_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"

typedef enum
{
    Memory,
    File,
    Device
} AXP_DiskType;

/*
 * Define some constants needed here.
 */
#define MAX_FILENAME_LEN	255
#define MAX_SERIALNO_LEN	32

/*
 * Disk block sizes (CDROM is 2K and Disk is 512)
 */
#define AXP_BLOCK_SIZE(cdrom)	((cdrom) ? 2048 : 512)

/*
 * We always have 8 heads and 32 sectors.
 */
#define AXP_DISK_HEADS		8
#define AXP_DISK_SECTORS	32

typedef struct
{

    /*
     * This field needs to be at the top of all data blocks/structures
     * that need to be specifically allocated by the Blocks module.
     */
    AXP_BLOCK_DSC	header;

    /*
     * Now we need some information to describe the disk.
     */
    AXP_DiskType	type;
    FILE		*fp;
    char		fileName[MAX_FILENAME_LEN+1];

    /*
     * Information about the disk drive itself.
     */
    char		serialNumber[MAX_SERIALNO_LEN+1];
    char		modelNumber[MAX_SERIALNO_LEN+1];
    char		revisionNumber[MAX_SERIALNO_LEN+1];
    off_t		diskSize;
    off_t		cylinders;
    size_t		blockSize;
    bool		readOnly;
    bool		isCdrom;
    bool		atapiMode;

    /*
     * Where are we currently within the file, device, memory.
     */
    off_t		position;
} AXP_PhysicalDisk;

#endif /* _AXP_DISK_H_ */
