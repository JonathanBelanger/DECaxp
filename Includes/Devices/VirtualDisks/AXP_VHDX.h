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
 *  V01.000 03-Jul-2018 Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VHDX_H_
#define _AXP_VHDX_H_
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Trace.h"
#include "Devices/VirtualDisks/AXP_VHD_Utility.h"
#include <errno.h>

/*
 * The following set of definitions are based on the VHDX Image Format
 * Specification by Microsoft.  These are the basis for disk emulation for the
 * Digital Alpha AXP Emulator.
 */
#define AXP_VHDXFILE_SIG    0x656c696678646876ll
#define AXP_HEAD_SIG        0x64616568l
#define AXP_REGI_SIG        0x69676572l
#define AXP_LOGE_SIG        0x65676f6cl
#define AXP_ZERO_SIG        0x6f72657al
#define AXP_DESC_SIG        0x63736564l
#define AXP_DATA_SIG        0x61746164l
#define AXP_METADATA_SIG    0x617461646174656dll

#define AXP_VHDX_CREATOR_LEN 256
typedef struct
{
    u64 sig;                                                /* 'vhdxfile' */
    uint16_t creator[AXP_VHDX_CREATOR_LEN];
} AXP_VHDX_ID;
#define AXP_VHDX_ID_LEN     520
#define AXP_VHDX_RES_1_LEN  4016
typedef struct
{
    u32 sig;                                                /* 'head' */
    u32 checkSum;
    u64 seqNum;
    AXP_VHDX_GUID fileWriteGuid;
    AXP_VHDX_GUID dataWriteGuid;
    AXP_VHDX_GUID logGuid;
    u16 logVer;
    u16 ver;
    u32 logLen;
    u64 logOff;
    u8 res_1[AXP_VHDX_RES_1_LEN];
} AXP_VHDX_HDR;
#define AXP_VHDX_HDR_LEN    4096
typedef struct
{
    u32 sig;                                                /* 'regi' */
    u32 checkSum;
    u32 entryCnt;
    u32 res_1;
} AXP_VHDX_REG_HDR;
#define AXP_VHDX_REG_HDR_LEN 16
typedef struct
{
    AXP_VHDX_GUID guid;
    u64 fileOff;
    u32 len;
    u32 req :1;
    u32 res_1 :31;
} AXP_VHDX_REG_ENT;
#define AXP_VHDX_REG_ENT_LEN 32
typedef struct
{
    u32 sig;                                                /* 'loge' */
    u32 checkSum;
    u32 entryLen;
    u32 tail;
    u64 seqNum;
    u32 dscCnt;
    u32 res_1;
    AXP_VHDX_GUID logGuid;
    u64 flushedFileOff;
    u64 lastFileOff;
} AXP_VHDX_LOG_HDR;
#define AXP_VHDX_LOG_HDR_LEN 64
typedef struct
{
    u32 sig;                                                /* 'zero' */
    u32 res_1;
    u64 len;
    u64 fileOff;
    u64 seqNum;
} AXP_VHDX_ZERO_DSC;
#define AXP_VHDX_ZERO_DSC_LEN 32
typedef struct
{
    u32 sig;                                                /* 'desc' */
    u32 trailingBytes;
    u64 leadingBytes;
    u64 fileOff;
    u64 seqNum;
} AXP_VHDX_DATA_DSC;
#define AXP_VHDX_DATA_DSC_LEN  32
#define AXP_VHDX_LOG_DATA_SIZE 4084
typedef struct
{
    u32 sig;                                                /* 'data' */
    u32 seqHi;
    u8 data[AXP_VHDX_LOG_DATA_SIZE];
    u32 seqLo;
} AXP_VHDX_LOG_DATA;
#define AXP_VHDX_LOG_DATA_LEN 4096
typedef struct
{
    u64 state :3;
    u64 res_1 :17;
    u64 fileOff :44;
} AXP_VHDX_BAT_ENT;
#define AXP_VHDX_BAT_ENT_LEN 8

/*
 * Payload BAT Entry States
 */
#define AXP_VHDX_PAYL_BLK_NOT_PRESENT   0
#define AXP_VHDX_PAYL_BLK_UNDEF         1
#define AXP_VHDX_PAYL_BLK_ZERO          2
#define AXP_VHDX_PAYL_BLK_UNMAPPED      3
#define AXP_VHDX_PAYL_BLK_FULLY_PRESENT 6
#define AXP_VHDX_PAYL_BLK_PART_PRESENT  7

/*
 * Sector Bitmap BAT Entry States
 */
#define AXP_VHDX_SB_BLK_NOT_PRESENT     0
#define AXP_VHDX_SB_BLK_PRESENT         6

#define AXP_VHDX_RES_2_LEN      5
typedef struct
{
    u64 sig;                                                /* 'metadata' */
    u16 res_1;
    u16 entryCnt;
    u32 res_2[AXP_VHDX_RES_2_LEN];
} AXP_VHDX_META_HDR;
#define AXP_VHDX_META_HDR_LEN   32

typedef struct
{
    AXP_VHDX_GUID guid;
    u32 off;
    u32 len;
    u32 isUser :1;
    u32 isVirtualDisk :1;
    u32 isRequired :1;
    u32 res_1 :29;
    u32 res_2;
} AXP_VHDX_META_ENT;
#define AXP_VHDX_META_ENT_LEN   32
typedef struct
{
    u32 blkSize;
    u32 leaveBlksAlloc :1;
    u32 hasParent :1;
    u32 res_1 :30;
} AXP_VHDX_META_FILE;
#define AXP_VHDX_META_FILE_LEN  8
typedef struct
{
    u64 virDskSize;
} AXP_VHDX_META_DISK;
#define AXP_VHDX_META_DISK_LEN  8
typedef struct
{
    AXP_VHDX_GUID pg83Data;
} AXP_VHDX_META_PAGE83;
#define AXP_VHDX_META_PAGE83_LEN 16
typedef struct
{
    u32 secSize;
} AXP_VHDX_META_SEC;
#define AXP_VHDX_META_SEC_LEN   4

typedef struct
{
    AXP_VHDX_GUID locType;
    u16 res_1;
    u16 keyValCnt;
}  __attribute__ ((packed)) AXP_VHDX_META_PAR_HDR;
#define AXP_VHDX_META_PAR_HDR_LEN 20 /* gcc sizeof says this is 24 */

typedef struct
{
    u32 keyOff;
    u32 valOff;
    u16 keyLen;
    u16 valLen;
} AXP_VHDX_META_PAR_ENT;
#define AXP_VHDX_META_PAR_ENT_LEN 12

/*
 * Lengths for various structures in the VHDX file.
 */
#define AXP_VHDX_LEN_HDR    ONE_M
#define AXP_VHDX_SUBHDR_LEN SIXTYFOUR_K
#define AXP_VHDX_LOG_LEN    ONE_M
#define AXP_VHDX_META_LEN   ONE_M
#define AXP_VHDX_BAT_LEN    ONE_M

/*
 * Offsets for various structures in the VHDX file,
 */
#define AXP_VHDX_HDR_LOC    0
#define AXP_VHDX_LOG_LOC    (AXP_VHDX_HDR_LOC + AXP_VHDX_LEN_HDR)
#define AXP_VHDX_META_LOC   (AXP_VHDX_LOG_LOC + AXP_VHDX_LOG_LEN)
#define AXP_VHDX_BAT_LOC    (AXP_VHDX_META_LOC + AXP_VHDX_META_LEN)
#define AXP_VHDX_DATA_LOC   (AXP_VHDX_BAT_LOC + AXP_VHDX_BAT_LEN)

/*
 * Header structure offsets (from AXP_VHDX_HDR_LOC)
 */
#define AXP_VHDX_FILE_ID_OFF 0
#define AXP_VHDX_HEADER1_OFF (AXP_VHDX_FILE_ID_OFF + AXP_VHDX_SUBHDR_LEN)
#define AXP_VHDX_HEADER2_OFF (AXP_VHDX_HEADER1_OFF + AXP_VHDX_SUBHDR_LEN)
#define AXP_VHDX_REG_TBL_HDR1_OFF (AXP_VHDX_HEADER2_OFF + AXP_VHDX_SUBHDR_LEN)
#define AXP_VHDX_REG_TBL_HDR2_OFF (AXP_VHDX_REG_TBL_HDR1_OFF + AXP_VHDX_SUBHDR_LEN)

/*
 * Metadata structure offsets (AXP_VHDX_META_LOC)
 */
#define AXP_VHDX_META_START_OFF SIXTYFOUR_K

/*
 * Various other constants used through out the VHDX code and within the file.
 */
#define AXP_VHDX_CURRENT_VER 1
#define AXP_VHDX_LOG_VER    0
#define AXP_VHDX_MAX_ENTRIES 2047
#define AXP_VHDX_PHYS_SEC_SIZE FOUR_K

/*
 * The below structure is used to maintain information about the virtual hard
 * disk file that is being used to simulate a hard disk.
 */
typedef struct
{

    /*
     * This is the file pointer and file name associated with the VHD.
     */
    FILE *fp;

    /*
     * These are parameters provided by the interface and stored for later
     * usage.
     */
    u32 deviceID;
    char *filePath;
    bool fixed;
    bool readOnly;

    /*
     * This is where the Block Allocation Table will be stored.  Its format is
     * determined by deviceID.
     */
    u32 batLength;
    u32 batCount;
    void *bat;

    /*
     * These are things read from (or written to) the VHD file that are used
     * while accessing the contents.
     */
    u64 logOffset;
    u64 batOffset;
    u64 metadataOffset;
    u64 diskSize;
    u32 logLength;
    u32 metadataLength;
    u32 blkSize;
    u32 sectorSize;
    u32 cylinders;
    u32 heads;
    u32 sectors;
} AXP_VHDX_Handle;

/*
 * Function Prototypes
 */
u32 _AXP_VHDX_Create(char *,
                     AXP_VHD_CREATE_FLAG,
                     char *,
                     u32,
                     u64,
                     u32,
                     u32,
                     u32,
                     AXP_VHD_HANDLE *);
u32 _AXP_VHDX_Open(char *, AXP_VHD_OPEN_FLAG, u32, AXP_VHD_HANDLE *);

#endif /* _AXP_VHDX_H_ */
