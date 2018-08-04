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
 *  This header file contains the definitions to support the VHD formatted
 *  virtual disk image.
*
 * Revision History:
 *
 *  V01.000	08-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VHD_H_
#define _AXP_VHD_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_VHD_Utility.h"
#include <errno.h>

/*
 * The following set of definitions are based on the Virtual Hard Disk Image
 * Format Specification V1.0 (October 11, 2006) by Microsoft.  These are the
 * basis for disk emulation for the Digital Alpha AXP Emulator.
 */

typedef enum
{
    DiskNone = 0,
    DiskFixed = 2,
    DiskDynamic = 3,
    DiskDifferencing = 4
} AXP_VHD_DiskType;

#define AXP_VHD_TypeValid(type)	\
    (((type) >= DiskFixed) && ((type) <= DiskDifferencing))

/*
 * Hard Disk Footer Format						Page 5
 *
 * All hard disk images share a basic footer format. Each hard disk type
 * extends this format according to its needs.
 *
 * NOTE: Versions previous to Microsoft Virtual PC 2004 create disk images that
 * have a 511-byte disk footer. So the hard disk footer can exist in the last
 * 511 or 512 bytes of the file that holds the hard disk image.
 */
#define AXP_VHD_FOOTER_RES_LEN		427
typedef struct
{
    u16		cylinders;
    u8		heads;
    u8		sectors;
} AXP_VHD_DiskGeo;
typedef struct
{
    u64			cookie;		/* conectix */
    u32			features;
    u32			formatVer;
    u64			dataOffset;
    u32			timestamp;	/* seconds since 01/01/2000 00:00 GMT */
    u32			creator;
    u32			creatorVer;
    u32			creatorHostOS;
    u64			originalSize;
    u64			currentSize;
    union
    {
	u32		diskGeo;
	AXP_VHD_DiskGeo chs;
    };
    AXP_VHD_DiskType	diskType;
    u32			checksum;
    AXP_VHDX_GUID	guid;
    u8			saveState;
    u8			res_1[AXP_VHD_FOOTER_RES_LEN];
} AXP_VHD_Footer;

#define AXP_VHDFILE_SIG		0x78697463656e6f63ll
#define AXP_FEATURES_NONE	0x00000000l
#define AXP_FEATURES_TEMP	0x00000001l
#define AXP_FEATURES_RES	0x00000002l
#define AXP_FORMAT_VER		0x00010000l
#define AXP_FIXED_OFFSET	0xffffffffl
#define AXP_VHD_CREATOR		0x50584144l
#define AXP_CREATOR_VER		0x00010000l
#define AXP_CREATOR_HOST	0x5769326bl

/*
 * Dynamic Disk Header Format						Page 8
 *
 * For dynamic and differencing disk images, the “Data Offset” field within the
 * image footer points to a secondary structure that provides additional
 * information about the disk image. The dynamic disk header should appear on a
 * sector (512-byte) boundary.
 */
#define AXP_VHD_DYNAMIC_RES_LEN	256
#define AXP_VHD_PARENT_LOC_CNT	8
typedef struct
{
    u32			code;
    u32			dataSpace;	/* in sectors */
    u32			dataLen;	/* in bytes */
    u32			res_1;
    u32			dataOff;
} AXP_VHD_ParentLoc;

#define AXP_VHD_PCODE_NONE	0x00000000l
#define AXP_VHD_PCODE_WI2R	0x57693272l
#define AXP_VHD_PCODE_WI2K	0x5769326Bl
#define AXP_VHD_PCODE_W2ru	0x57327275l
#define AXP_VHD_PCODE_W2ku	0x57326B75l
#define AXP_VHD_PCODE_MAC	0x4D616320l
#define AXP_VHD_PCODE_MACX	0x4D616358l

typedef struct
{
    u64			cookie;		/* cxsparse */
    u64			dataOff;
    u64			tableOff;
    u32			headerVer;
    u32			maxTableEnt;
    u32			blockSize;
    u32			checksum;
    AXP_VHDX_GUID	parentGuid;
    u32			parentTimestamp;
    u32			res_1;
    uint16_t		parentName;
    AXP_VHD_ParentLoc	parentLoc[AXP_VHD_PARENT_LOC_CNT];
    u8			res_2[AXP_VHD_DYNAMIC_RES_LEN];
} AXP_VHD_Dynamic;

#define AXP_VHD_DYNAMIC_SIG	0x6573726170737863ll
#define AXP_VHD_DATA_OFFSET	0xffffffffl
#define AXP_VHD_HEADER_VER	0x00010000l

/*
 * Block Allocation Table and Data Blocks				Page 12
 *
 * The Block Allocation Table (BAT) is a table of absolute sector offsets into
 * the file backing the hard disk. It is pointed to by the “Table Offset” field
 * of the Dynamic Disk Header.
 *
 * The size of the BAT is calculated during creation of the hard disk. The
 * number of entries in the BAT is the number of blocks needed to store the
 * contents of the disk when fully expanded. For example, a 2G disk image that
 * uses 2M blocks requires 1024 BAT entries. Each entry is four bytes long. All
 * unused table entries are initialized to 0xFFFFFFFF.
 *
 * The BAT is always extended to a sector boundary. The “Max Table Entries”
 * field within the Dynamic Disk Header indicates how many entries are valid.
 */
typedef u32		AXP_VHD_BAT_ENT;
#define AXP_VHD_BAT_UNUSED	0xffffffffl

/*
 * Function Prototypes
 */
u32 _AXP_VHD_Create(
		char *,
		AXP_VHD_CREATE_FLAG,
		char *,
		u32,
		u64,
		u32,
		u32,
		u32,
		AXP_VHD_HANDLE *);
u32 _AXP_VHD_Open(char *, AXP_VHD_OPEN_FLAG, u32, AXP_VHD_HANDLE *);

#endif /* _AXP_VHD_H_ */
