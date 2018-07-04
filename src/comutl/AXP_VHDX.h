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
 *  This header file contains the definitions to support the VHDX formatted
 *  virtual disk image.
 *
 * Revision History:
 *
 *  V01.000	03-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VHDX_H_
#define _AXP_VHDX_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_VHDX_GUID.h"
#include <unicode/utypes.h>
#include <iconv.h>

/*
 * The following set of definitions are based on the VHDX Image Format
 * Specification by Microsoft.  These are the basis for disk emulation for the
 * Digital Alpha AXP Emulator.
 */
#define AXP_VHDX_ID_SIG_LEN		8
#define AXP_VHDX_SIG_LEN		4
#define AXP_VHDX_CREATOR_LEN		256
typedef struct
{
    char		sig[AXP_VHDX_ID_SIG_LEN];	/* 'vhdxfile' */
    uint16_t		creator[AXP_VHDX_CREATOR_LEN];
} AXP_VHDX_ID;
#define AXP_VHDX_RES_1_LEN		4016
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'head' */
    u32			checkSum;
    u64			seqNum;
    AXP_VHDX_GUID	fileWriteGuid;
    AXP_VHDX_GUID	dataWriteGuid;
    AXP_VHDX_GUID	logGuid;
    u16			logVer;
    u16			ver;
    u32			logLen;
    u64			logOff;
    u8			res_1[AXP_VHDX_RES_1_LEN];
} AXP_VHDX_HDR;
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'regi' */
    u32			checkSum;
    u32			entryCnt;
    u32			res_1;
} AXP_VHDX_REG_HDR;
typedef struct
{
    AXP_VHDX_GUID	guid;
    u64			fileOff;
    u32			len;
    u32			req : 1;
    u32			res_1 : 31;
} AXP_VHDX_REG_ENT;
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'loge' */
    u32			checkSum;
    u32			entryLen;
    u32			tail;
    u64			seqNum;
    u32			dscCnt;
    u32			res_1;
    AXP_VHDX_GUID	logGuid;
    u64			flushedFileOff;
    u64			lastFileOff;
} AXP_VHDX_LOG_HDR;
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'zero' */
    u32			res_1;
    u64			len;
    u64			fileOff;
    u64			seqNum;
} AXP_VHDX_ZERO_DSC;
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'desc' */
    u32			trailingBytes;
    u64			leadingBytes;
    u64			fileOff;
    u64			seqNum;
} AXP_VHDX_DATA_DSC;
#define AXP_VHDX_LOG_DATA_LEN	4084
typedef struct
{
    char		sig[AXP_VHDX_SIG_LEN];		/* 'data' */
    u32			seqHi;
    u8			data[AXP_VHDX_LOG_DATA_LEN];
    u32			seqLo;
} AXP_VHDX_LOG_DATA;
typedef struct
{
    u64			state : 3;
    u64			res_1 : 17;
    u64			fileOff : 44;
} AXP_VHDX_BAT_ENT;

/*
 * Payload BAT Entry States
 */
#define AXP_VHDX_PAYL_BLK_NOT_PRESENT	0
#define AXP_VHDX_PAYL_BLK_UNDEF		1
#define AXP_VHDX_PAYL_BLK_ZERO		2
#define AXP_VHDX_PAYL_BLK_UNMAPPED	3
#define AXP_VHDX_PAYL_BLK_FULLY_PRESENT	6
#define AXP_VHDX_PAYL_BLK_PART_PRESENT	7

/*
 * Sector Bitmap BAT Entry States
 */
#define AXP_VHDX_SB_BLK_NOT_PRESENT	0
#define AXP_VHDX_SB_BLK_PRESENT		6

#define AXP_VHDX_RES_2_LEN		5
typedef struct
{
    char		sig[AXP_VHDX_ID_SIG_LEN];	/* 'metadata' */
    u16			res_1;
    u16			entryCnt;
    u32			res_2[AXP_VHDX_RES_2_LEN];
} AXP_VHDX_META_HDR;
#define AXP_VHDX_FILE_PARAM_GUID(name)		\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0xcaa16737,			\
	.data2 = 0xfa36,			\
	.data3 = 0x4d43,			\
	.data4 = 0xb3b633f0aa44e76b		\
    }
#define AXP_VHDX_VIRT_DSK_SIZE_GUID(name)	\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0x2fa54224,			\
	.data2 = 0xcd1b,			\
	.data3 = 0x4876,			\
	.data4 = 0xb2115dbed83bf4b8		\
    }
#define AXP_VHDX_PAGE_83_DATA_GUID(name)	\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0xbeca12ab,			\
	.data2 = 0xb2e6,			\
	.data3 = 0x4523,			\
	.data4 = 0x93efc309e000c746		\
    }
#define AXP_VHDX_LOGI_SEC_SIZE_GUID(name)	\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0x8141bf1d,			\
	.data2 = 0xa96f,			\
	.data3 = 0x4709,			\
	.data4 = 0xba47f233a8faab5f		\
    }
#define AXP_VHDX_PHYS_SEC_SIZE_GUID(name)	\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0xcda348c7,			\
	.data2 = 0x445d,			\
	.data3 = 0x4471,			\
	.data4 = 0x9cc9e9885251c556		\
    }
#define AXP_VHDX_PARENT_LOC_GUID(name)		\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0xa8d35f2d,			\
	.data2 = 0xb30b,			\
	.data3 = 0x454d,			\
	.data4 = 0xabf7d3d84834ab0c		\
    }
#define AXP_VHDX_BAT_GUID(name)			\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0x2dc27766,			\
	.data2 = 0xf623,			\
	.data3 = 0x4200,			\
	.data4 = 0x9d64115e9Bfd4a08		\
    }
#define AXP_VHDX_META_GUID(name)		\
    (name) = (AXP_VHDX_GUID)			\
    {						\
	.data1 = 0x8b7ca206,			\
	.data2 = 0x4790,			\
	.data3 = 0x4b9a,			\
	.data4 = 0xb8fe575f050f886e		\
    }

typedef struct
{
    AXP_VHDX_GUID	guid;
    u32			off;
    u32			len;
    u32			isUser : 1;
    u32			isVirtualDisk : 1;
    u32			isRequired : 1;
    u32			res_1 : 29;
    u32			res_2;
} AXP_VHDX_META_ENT;
typedef struct
{
    u32			blkSize;
    u32			leaveBlksAlloc : 1;
    u32			hasParent : 1;
    u32			res_1 : 30;
} AXP_VHDX_META_FILE;
typedef struct
{
    u64			virDskSize;
} AXP_VHDX_META_DISK;
typedef struct
{
    AXP_VHDX_GUID	pg83Data;
} AXP_VHDX_META_PAGE83;
typedef struct
{
    u32			secSize;
} AXP_VHDX_META_SEC;
typedef struct
{
    AXP_VHDX_GUID	locType;
    u16			res_1;
    u16			keyValCnt;
} AXP_VHDX_META_PAR_HDR;
typedef struct
{
    u32			keyOff;
    u32			valOff;
    u16			keyLen;
    u16			valLen;
} AXP_VHDX_META_PAR_ENT;

#define AXP_VHDX_MAX_ENTRIES		2047
#define AXP_VHDX_CURRENT_VER		1
#define AXP_VHDX_LOG_LOC		ONE_M
#define AXP_VHDX_LOG_LEN		ONE_M
#define AXP_VHDX_LOG_VER		0
#define AXP_VHDX_BAT_LEN		ONE_M
#define AXP_VHDX_BAT_LOC		(AXP_VHDX_LOG_LOC + AXP_VHDX_LOG_LEN)
#define AXP_VHDX_METADATA_LEN		ONE_M
#define AXP_VHDX_METADATA_LOC		(AXP_VHDX_BAT_LOC + AXP_VHDX_BAT_LEN)
#define AXP_VHDX_METADATA_START_OFF	SIXTYFOUR_K
#define AXP_VHDX_FILE_ID_OFF		0
#define AXP_VHDX_HEADER1_OFF		(1 * SIXTYFOUR_K)
#define AXP_VHDX_HEADER2_OFF		(2 * SIXTYFOUR_K)
#define AXP_VHDX_REG_TBL_HDR1_OFF	(3 * SIXTYFOUR_K)
#define AXP_VHDX_REG_TBL_HDR2_OFF	(4 * SIXTYFOUR_K)
#define AXP_VHDX_PHYS_SECTOR_SIZE	FOUR_K
#define AXP_VHDX_MAX_DISK_SIZE		(64ULL * ONE_T)
#define AXP_VHDX_MIN_BLK_SIZE		ONE_K
#define AXP_VHDX_MAX_BLK_SIZE		(256 * ONE_K)
#define AXP_VHDX_DEF_BLK_SIZE		THIRTYTWO_K
#define AXP_VHDX_DEF_SEC_SIZE		512
#define AXP_VHDX_MIN_ALIGNMENT		ONE_K
#define AXP_VHDX_BLK_1MB		ONE_K

/*
 * The below structure is used to maintain information about the virtual hard
 * disk file that is being used to simulate a hard disk.
 */
typedef struct
{
    AXP_BLOCK_DSC	header;
    FILE		*fp;
    AXP_VHDX_ID		fileID;
    AXP_VHDX_HDR	hdr;
} AXP_VHDX_Handle;

#endif /* _AXP_VHDX_H_ */
