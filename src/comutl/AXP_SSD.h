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
 *  This header file contains the definitions needed to support a solid state
 *  disk (SSD).
 *
 * Revision History:
 *
 *  V01.000	05-Aug-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef AXP_SSD_H_
#define AXP_SSD_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"

#define AXP_SSD_SIG1	0x424a707861434544ll
#define AXP_SSD_SIG2	0x4445436178704a42ll

typedef struct
{
    u8		ID1[8];
    u64		diskSize;
    u32		blkSize;
    u32		sectorSize;
    u32		cylinders;
    u32		heads;
    u32		sectors;
    u8		reserved[20];
    u8		ID2[8];
} AXP_SSD_Geometry;

typedef struct
{

    /*
     * These are parameters provided by the interface and stored for later
     * usage.
     */
    u32		deviceID;
    char	*filePath;

    /*
     * This is the file pointer associated with backing store for the SSD.
     */
    FILE	*fp;

    /*
     * This is the actual solid state drive.  This is exactly the size of the
     * disk (there is no header or trailer information.
     */
    u8		*memory;

    /*
     * These are things read from (or written to) the backing store file that
     * are used while accessing the contents.
     */
    u64		diskSize;
    u32		blkSize;
    u32		sectorSize;
    u32		cylinders;
    u32		heads;
    u32		sectors;
} AXP_SSD_Handle;

#endif /* AXP_SSD_H_ */
