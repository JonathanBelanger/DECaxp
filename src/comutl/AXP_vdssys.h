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
 *  This header file contains the definitions in vdssys.h, required to
 *  implement the virtual disk functions defined by Microsoft.
 *
 * Revision History:
 *
 *  V01.000	01-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VDSSYS_H_
#define _AXP_VDSSYS_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_VHDX_GUID.h"

/*
 * Enumerations
 */
typedef enum
{
    APPLY_SNAP_NONE,
    APPLY_SNAP_WRITE
} AXP_VHD_APPLY_SNAP_FLAG;
typedef enum
{
    APPLY_SNAP_UNSPEC,
    APPLY_SNAP_VER_1
} AXP_VHD_APPLY_SNAP_VER;
typedef enum
{
    ATTACH_NONE,
    ATTACH_READ_ONLY,
    ATTACH_NO_DRIVE_LETTER,
    ATTACH_PERM_LIFETIME,
    ATTACH_NO_LOCAL_HOST,
    ATTACH_NO_SECURITY_DESCRIPTOR,
    ATTACH_BYPASS_DEFAULT_ENCRYPTION_POLICY
} AXP_VHD_ATTACH_FLAG;
typedef enum
{
    ATTACH_VER_UNSPEC,
    ATTACH_VER_1
} AXP_VHD_ATTACH_VER;
typedef enum
{
    COMPACT_NONE,
    COMPACT_NO_ZERO_SCAN,
    COMPACT_NO_BLOCK_MOVES
} AXP_VHD_COMPACT_FLAG;
typedef enum
{
    COMPACT_VER_UNSPEC,
    COMPACT_VER_1
} AXP_VHD_COMPACT_VER;
typedef enum
{
    CREATE_NONE,
    CREATE_FULL_PHYSICAL_ALLOCATION,
    CREATE_PREVENT_WRITES_TO_SOURCE_DISK,
    CREATE_DO_NOT_COPY_METADATA_FROM_PARENT,
    CREATE_CREATE_BACKING_STORAGE,
    CREATE_USE_CHANGE_TRACKING_SOURCE_LIMIT,
    CREATE_PRESERVE_PARENT_CHANGE_TRACKING_STATE,
    CREATE_VHD_SET_USE_ORIGINAL_BACKING_STORAGE,
    CREATE_SPARSE_FILE,
    CREATE_PMEM_COMPATIBLE
} AXP_VHD_CREATE_FLAG;
typedef enum
{
    CREATE_VER_UNSPEC,
    CREATE_VER_1,
    CREATE_VER_2,
    CREATE_VER_3,
    CREATE_VER_4
} AXP_VHD_CREATE_VERSION;
typedef enum
{
    DELETE_SNAP_NONE,
    DELETE_SNAP_PERSIST_RCT
} AXP_VHD_DELETE_SNAP_FLAG;
typedef enum
{
    DELETE_SNAP_VER_UNSPEC,
    DELETE_SNAP_VER_1
} AXP_VHD_DELETE_SNAP_VER;
typedef enum
{
    DEP_NONE,
    DEP_MULT_BACKING_FILES,
    DEP_FULLY_ALLOCATED,
    DEP_READ_ONLY,
    DEP_REMOTE,
    DEP_SYS_VOL,
    DEP_SYS_VOL_PARENT,
    DEP_REMOVABLE,
    DEP_NO_DRIVE_LETTER,
    DEP_PARENT,
    DEP_NO_HOST_DISK,
    DEP_PERM_LIFETIME
} AXP_VHD_DEP_FLAG;
typedef enum
{
    DETACH_NONE
} AXP_VHD_DETACH_FLAG;
typedef enum
{
    EXPAND_NONE
} AXP_VHD_EXPAND_FLAG;
typedef enum
{
    EXPAND_VER_UNSPEC,
    EXPAND_VER_1
} AXP_VHD_EXPAND_VER;
typedef enum
{
    GET_STORAGE_DEP_NONE,
    GET_STORAGE_DEP_HOST_VOL,
    GET_STORAGE_DEP_DISK_HANDLE
} AXP_VHD_GET_STORAGE_DEP_FLAG;
typedef enum
{
    GET_INFO_UNSPEC,
    GET_INFO_SIZE,
    GET_INFO_ID,
    GET_INFO_PARENT_LOC,
    GET_INFO_PARENT_ID,
    GET_INFO_PARENT_TIMESTAMP,
    GET_INFO_STORAGE_TYPE,
    GET_INFO_PROVIDER_SUBTYPE,
    GET_INFO_IS_4K_ALIGNED,
    GET_INFO_PHYS_DISK,
    GET_INFO_PHYS_SECTOR_SIZE,
    GET_INFO_SMALL_SAFE_VIRT_SIZE,
    GET_INFO_FRAG,
    GET_INFO_IS_LOADED,
    GET_INFO_DISK_ID,
    GET_INFO_CHANGE_TRACK_STATE
} AXP_VHD_GET_INFO_VER;
typedef enum
{
    MERGE_NONE
} AXP_VHD_MERGE_FLAG;
typedef enum
{
    MERGE_VER_UNSPEC,
    MERGE_VER_1,
    MERGE_VER_2
} AXP_VHD_MERGE_VER;
typedef enum
{
    MIRROR_NONE,
    MIRROR_EXISTING_FILE,
    MIRROR_SKIP_MIRROR_ACT
} AXP_VHD_MIRROR_FLAG;
typedef enum
{
    MIRROR_VER_UNSPEC,
    MIRROR_VER_1
} AXP_VHD_MIRROR_VER;
typedef enum
{
    MODIFY_NONE,
    MODIFY_WRITEABLE_SNAP
} AXP_VHD_MOD_FLAG;
typedef enum
{
    MODIFY_UNSPEC,
    MODIFY_SNAP_PATH,
    MODIFY_REMOVE_SNAP,
    MODIFY_DEF_SNAP_PATH
} AXP_VHD_MOD_VER;
typedef enum
{
    OPEN_NONE,
    OPEN_NO_PARENTS,
    OPEN_BLANK_FILE,
    OPEN_BOOT_DRIVE,
    OPEN_CACHED_IO,
    OPEN_CUSTOM_DIFF_CHAIN,
    OPEN_PARENT_CACHED_IO,
    OPEN_VHDSET_FILE_ONLY,
    OPEN_IGNORE_REL_PARENT_LOC,
    OPEN_NO_WRITE_HARDENING
} AXP_VHD_OPEN_FLAG;
typedef enum
{
    OPEN_VER_UNSPEC,
    OPEN_VER_1,
    OPEN_VER_2,
    OPEN_VER_3
} AXP_VHD_OPEN_VER;
typedef enum
{
    QUERY_CHANGES_NONE
} AXP_VHD_QUERY_CHANGES_FLAG;
typedef enum
{
    RAW_SCSI_NONE
} AXP_VHD_RAW_SCSI_FLAG;
typedef enum
{
    RAW_SCSI_VER_UNSPEC,
    RAW_SCSI_VER_1
} AXP_VHD_RAW_SCSI_VER;
typedef enum
{
    SET_INFO_UNSPEC,
    SET_INFO_PARENT_PATH,
    SET_INFO_ID,
    SET_INFO_PARENT_PATH_WITH_DEPTH,
    SET_INFO_PHYS_SECTOR_SIZE,
    SET_INFO_VIRT_DISK_ID,
    SET_INFO_CHANGE_TRACKING_STATE,
    SET_INFO_PARENT_LOC
} AXP_VHD_SET_INFO_VER;
typedef enum
{
    STORAGE_DEP_VER_UNSPEC,
    STORAGE_DEP_VER_1,
    STORAGE_DEP_VER_2
} AXP_VHD_STORAGE_DEP_INFO_VER;
typedef enum
{
    TAKE_SNAP_NONE,
    TAKE_SNAP_WRITEABLE
} AXP_VHD_TAKE_SNAP_FLAG;
typedef enum
{
    TAKE_SNAP_VER_UNSPEC,
    TAKE_SNAP_VER_1
} AXP_VHD_TAKE_SNAP_VER;
typedef enum
{
    ACCESS_NONE,
    ACCESS_ATTACH_RO,
    ACCESS_ATTACH_RW,
    ACCESS_DETACH,
    ACCESS_GET_INFO,
    ACCESS_CREATE,
    ACCESS_METAOPS,
    ACCESS_READ,
    ACCESS_ALL,
    ACCESS_WRITABLE
} AXP_VHD_ACCESS_MASK;

/*
 * Typedefs and Data Structures.
 */

/*
 * Device time and Vendor information.
 */
typedef struct
{
    u32 deviceID;
    AXP_VHDX_GUID vendorID;
} AXP_VHD_STORAGE_TYPE;

/*
 * Device type (DeviceID)
 */
#define STORAGE_TYPE_DEV_UNKNOWN	0
#define STORAGE_TYPE_DEV_ISO		1
#define STORAGE_TYPE_DEV_VHD		2
#define STORAGE_TYPE_DEV_VHDX		3

typedef void	*AXP_VHD_HANDLE;
typedef struct
{
    AXP_VHD_APPLY_SNAP_VER ver;
    union
    {
	struct
	{
	    AXP_VHDX_GUID snapId;
	    AXP_VHDX_GUID leafSnapId;
	} ver_1;
    };
} AXP_VHD_APPLY_SNAP_PARAM;
typedef struct
{
    AXP_VHD_ATTACH_VER ver;
    union
    {
	struct
	{
	    u32 res_1;
	} ver_1;
    };
} AXP_VHD_ATTACH_PARAM;
typedef struct
{
    AXP_VHD_COMPACT_VER ver;
    union
    {
	struct
	{
	    u32 res_1;
	} ver_1;
    };
} AXP_VHD_COMPACT_PARAM;
typedef struct
{
    AXP_VHD_CREATE_VERSION ver;
    union
    {
	struct
	{
	    AXP_VHDX_GUID GUID;
	    u64 maxSize;
	    u32 blkSize;
	    u32 sectorSize;
	    char *parentPath;
	    char *srcPath;
	} ver_1;
	struct
	{
	    AXP_VHDX_GUID GUID;
	    u64 maxSize;
	    u32 blkSize;
	    u32 sectorSize;
	    u32 physSectorSize;
	    char *parentPath;
	    char *srcPath;
	    AXP_VHD_OPEN_FLAG flags;
	    AXP_VHD_STORAGE_TYPE parentStorageType;
	    AXP_VHD_STORAGE_TYPE srcStorageType;
	    AXP_VHDX_GUID resiliencyGUID;
	} ver_2;
	struct
	{
	    AXP_VHDX_GUID GUID;
	    u64 maxSize;
	    u32 blkSize;
	    u32 sectorSize;
	    u32 physSectorSize;
	    char *parentPath;
	    char *srcPath;
	    AXP_VHD_OPEN_FLAG flags;
	    AXP_VHD_STORAGE_TYPE parentStorageType;
	    AXP_VHD_STORAGE_TYPE srcStorageType;
	    AXP_VHDX_GUID resiliencyGUID;
	    char *srcLimitPath;
	    AXP_VHD_STORAGE_TYPE backingStorageType;
	} ver_3;
	struct
	{
	    AXP_VHDX_GUID GUID;
	    u64 maxSize;
	    u32 blkSize;
	    u32 sectorSize;
	    u32 phySectorSize;
	    char *parentPath;
	    char *srcPath;
	    AXP_VHD_OPEN_FLAG flags;
	    AXP_VHD_STORAGE_TYPE parentStorageType;
	    AXP_VHD_STORAGE_TYPE srcStorageType;
	    AXP_VHDX_GUID resiliencyGUID;
	    char *srcLimitPath;
	    AXP_VHD_STORAGE_TYPE backingStorageType;
	    AXP_VHDX_GUID pMemAddrAbstractionType;
	    u64 dataAlignment;
	} ver_4;
    };
} AXP_VHD_CREATE_PARAM;
typedef struct
{
    AXP_VHD_DELETE_SNAP_VER ver;
    union
    {
	struct
	{
	    AXP_VHDX_GUID snapsId;
	} ver_1;
    };
} AXP_VHD_DELETE_SNAP_PARAM;
typedef struct
{
    AXP_VHD_EXPAND_VER ver;
    union
    {
	struct
	{
	    u64 newSize;
	} ver_1;
    };
} AXP_VHD_EXPAND_PARAM;
typedef struct
{
    AXP_VHD_GET_INFO_VER ver;
    union
    {
	struct
	{
	    u64 virtSize;
	    u64 physSize;
	    u32 blkSize;
	    u32 sectorSize;
	} size;
	AXP_VHDX_GUID ID;
	struct
	{
	    bool parentResolved;
	    char parentLocBuffer[1];
	} parentLoc;
	AXP_VHDX_GUID parentID;
	u32 parentTimestamp;
	AXP_VHD_STORAGE_TYPE storageType;
	u32 providerSubtype;
	bool is4kAligned;
	bool isLoaded;
	struct
	{
	    u32 logSectorSize;
	    u32 physSectorSize;
	    bool isRemote;
	} physDisk;
	u32 vhdPhysSectorSize;
	u64 smallestSafeVirtSize;
	u32 fragPercentage;
	AXP_VHDX_GUID virtDiskID;
	struct
	{
	    bool enabled;
	    bool newerChanges;
	    char mostRecentID[1];
	} changeTrackingState;
    };
} AXP_VHD_GET_INFO;
typedef struct
{
    AXP_VHD_MERGE_VER ver;
    union
    {
	struct
	{
	    u32 mergeDepth;
	} ver_1;
	struct
	{
	    u32 mergeSrcDepth;
	    u32 mergeTargetDepth;
	} ver_2;
    };
} AXP_VHD_MERGE_PARAM;
typedef struct
{
    AXP_VHD_MIRROR_VER ver;
    union
    {
	struct
	{
	    char *mirrorDiskPath;
	} ver_1;
    };
} AXP_VHD_MIRROR_PARAM;
typedef struct
{
    AXP_VHD_MOD_VER ver;
    union
    {
	struct
	{
	    AXP_VHDX_GUID snapID;
	    char *snapFilePath;
	} snapPath;
	AXP_VHDX_GUID snapID;
	char *defFilePath;
    };
} AXP_VHD_MOD_PARAM;
typedef struct
{
    AXP_VHD_OPEN_VER ver;
    union
    {
	struct
	{
	    u32 rWDepth;
	} ver_1;
	struct
	{
	    bool getInfoOnly;
	    bool readOnly;
	    AXP_VHDX_GUID resiliencyGUID;
	} ver_2;
	struct
	{
	    bool getInfoOnly;
	    bool readOnly;
	    AXP_VHDX_GUID resiliencyGUID;
	    AXP_VHDX_GUID snapID;
	} ver_3;
    };
} AXP_VHD_OPEN_PARAM;
typedef struct
{
    u64 offset;
    u64 len;
    u64 res_1;
} AXP_VHD_QUERY_CHANGES_RANGE;
typedef struct
{
    AXP_VHD_RAW_SCSI_VER ver;
    union
    {
	struct
	{
	    bool rSVDHandle;
	    u8 dataIn;
	    u8 cdbLen;
	    u8 senseInfoLen;
	    u32 srbFlags;
	    u32 dataXferLen;
	    void *dataBuf;
	    u8 *senseInfo;
	    u8 *cdb;
	} ver_1;
    };
} AXP_VHD_RAW_SCSI_PARAM;
typedef struct
{
    AXP_VHD_RAW_SCSI_VER ver;
    union
    {
	struct
	{
	    u8 ccsiStatus;
	    u8 senseInfoLen;
	    u32 dataXferLen;
	} ver_1;
    };
} AXP_VHD_RAW_SCSI_RSP;
typedef struct
{
    AXP_VHD_SET_INFO_VER ver;
    union
    {
	char *parentFilePath;
	AXP_VHDX_GUID GUID;
	struct
	{
	    u32 childDepth;
	    char *parentFilePath;
	} parentPathWithDepthInfo;
	u32 vhdPhysSectorSize;
	AXP_VHDX_GUID virtDiskId;
	bool changeTrackingEnabled;
	struct
	{
	    AXP_VHDX_GUID linkageID;
	    char *parentFilePath;
	} parentLoc;
    };
} AXP_VHD_SET_INFO_PARAM;
typedef struct
{
    AXP_VHD_DEP_FLAG depTypeFlags;
    u32 providerSpecFlags;
    AXP_VHD_STORAGE_TYPE virtStorageType;
} AXP_VHD_STORAGE_DEP_INFO_1;
typedef struct
{
    AXP_VHD_DEP_FLAG depTypeFlags;
    u32 providerSpecFlags;
    AXP_VHD_STORAGE_TYPE virtStorageType;
    u32 ancestorLevel;
    char *depDevName;
    char *hostVolName;
    char *depVolName;
    char *depVolRelPath;
} AXP_VHD_STORAGE_DEP_INFO_2;
typedef struct
{
    AXP_VHD_STORAGE_DEP_INFO_VER ver;
    u32 numEntries;
    union
    {
	AXP_VHD_STORAGE_DEP_INFO_1 entriesV1[1];
	AXP_VHD_STORAGE_DEP_INFO_2 entriesV2[1];
    };
} AXP_VHD_STORAGE_DEP_INFO;
typedef struct
{
    AXP_VHD_TAKE_SNAP_VER ver;
    union
    {
	struct
	{
	    AXP_VHDX_GUID snapsID;
	} ver_1;
    };
} AXP_VHD_TAKE_SNAP_PARAM;
typedef struct
{
    u32 operStatus;
    u64 curVal;
    u64 complVal;
} AXP_VHD_PROGRESS;

/*
 * We put these here because they limit values through the interface.
 */
#define AXP_VHD_DEF_BLK		0
#define AXP_VHD_BLK_MIN		(512 * ONE_K)
#define AXP_VHD_BLK_DEF		(2 * ONE_M)
#define AXP_VHD_BLK_MAX		(2 * ONE_M)
#define AXP_VHDX_BLK_MIN	(512 * ONE_K)
#define AXP_VHDX_BLK_DEF	(32 * ONE_M)
#define AXP_VHDX_BLK_MAX	(256 * ONE_M)
#define AXP_ISO_BLK_DEF		0
#define AXP_ISO_BLK_MIN		0
#define AXP_ISO_BLK_MAX		0
#define AXP_VHD_DEF_SEC		0
#define AXP_VHD_SEC_DEF		512
#define AXP_VHD_SEC_MIN		512
#define AXP_VHD_SEC_MAX		512
#define AXP_VHDX_SEC_DEF	512
#define AXP_VHDX_SEC_MIN	512
#define AXP_VHDX_SEC_MAX	FOUR_K
#define AXP_ISO_SEC_DEF		TWO_K
#define AXP_ISO_SEC_MIN		TWO_K
#define AXP_ISO_SEC_MAX		TWO_K

typedef struct
{
    u32 *internal;
    u32 *internalHi;
    union
    {
	struct
	{
	    u32 offset;
	    u32 offsetHi;
	};
	void *ptr;
    };
    void *hEvent;
} AXP_VHD_ASYNC;
typedef void AXP_VHD_SEC_DSC;

/*
 * Function Prototypes
 *
 *
 * Applies a snapshot of the current virtual disk for VHD Set files.
 */
u32 AXP_VHD_ApplySnap(
		AXP_VHD_HANDLE handle,
		const AXP_VHD_APPLY_SNAP_VER *param,
		AXP_VHD_APPLY_SNAP_FLAG flags);

/*
 * Attaches a virtual hard disk (VHD) or CD or DVD image file (ISO) by locating
 * an appropriate VHD provider to accomplish the attachment.
 */
u32 AXP_VHD_Attach(
		AXP_VHD_HANDLE handle,
		AXP_VHD_SEC_DSC *securityDsc,
		AXP_VHD_ATTACH_FLAG flags,
		u32 providerSpecFlags,
		AXP_VHD_ATTACH_PARAM *param,
		AXP_VHD_ASYNC *async);

/*
 * Breaks a previously initiated mirror operation and sets the mirror to be the
 * active virtual disk.
 */
u32 AXP_VHD_BreakMirror(AXP_VHD_HANDLE handle);

/*
 * Reduces the size of a virtual hard disk (VHD) backing store file.
 */
u32 AXP_VHD_Compact(
		AXP_VHD_HANDLE handle,
		AXP_VHD_COMPACT_FLAG flags,
		AXP_VHD_COMPACT_PARAM *param,
		AXP_VHD_ASYNC *async);

/*
 * Creates a virtual hard disk (VHD) image file, either using default
 * parameters or using an existing virtual disk or physical disk.
 */
u32 AXP_VHD_Create(
		AXP_VHD_STORAGE_TYPE *storageType,
		char *path,
		AXP_VHD_ACCESS_MASK accessMask,
		AXP_VHD_SEC_DSC *securityDsc,
		AXP_VHD_CREATE_FLAG flags,
		u32 providerSpecFlags,
		AXP_VHD_CREATE_PARAM *param,
		AXP_VHD_ASYNC *async,
		AXP_VHD_HANDLE *handle);

/*
 * Deletes a snapshot from a VHD Set file.
 */
u32 AXP_VHD_DeleteSnap(
		AXP_VHD_HANDLE handle,
		const AXP_VHD_DELETE_SNAP_PARAM *param,
		AXP_VHD_DELETE_SNAP_FLAG flags);

/*
 * Detaches a virtual hard disk (VHD) or CD or DVD image file (ISO) by locating
 * an appropriate virtual disk provider to accomplish the operation.
 */
u32 AXP_VHD_Detach(
		AXP_VHD_HANDLE handle,
		AXP_VHD_DETACH_FLAG flags,
		u32 ProviderSpecFlags);

/*
 * Increases the size of a fixed or dynamically expandable virtual hard disk
 * (VHD).
 */
u32 AXP_VHD_Expand(
		AXP_VHD_HANDLE handle,
		AXP_VHD_EXPAND_FLAG flags,
		AXP_VHD_EXPAND_PARAM *param,
		AXP_VHD_ASYNC *async);

/*
 * Returns the relationships between virtual hard disks (VHDs) or CD or DVD
 * image file (ISO) or the volumes contained within those disks and their
 * parent disk or volume.
 */
u32 AXP_VHD_GetStorageDepInfo(
		AXP_VHD_HANDLE handle,
		AXP_VHD_GET_STORAGE_DEP_FLAG flags,
		u32 storageDepInfoSize,
		AXP_VHD_STORAGE_DEP_INFO *storageDepInfo,
		u32 *sizeUsed);

/*
 * Retrieves information about a VHD.
 */
u32 AXP_VHD_GetInfo(
		AXP_VHD_HANDLE handle,
		u32 *infoSize,
		AXP_VHD_GET_INFO *info,
		u32 *sizeUsed);

/*
 * Checks the progress of an asynchronous virtual hard disk (VHD) operation.
 */
u32 AXP_VHD_GetProgress(
		AXP_VHD_HANDLE handle,
		AXP_VHD_ASYNC *async,
		AXP_VHD_PROGRESS *prog);

/*
 * Retrieves the path to the physical device object that contains a virtual
 * hard disk (VHD) or CD or DVD image file (ISO).
 */
u32 AXP_VHD_GetPhysPath(
		AXP_VHD_HANDLE handle,
		u32 *diskPathSize,
		char *diskPath);

/*
 * Merges a child virtual hard disk (VHD) in a differencing chain with one or
 * more parent virtual disks in the chain.
 */
u32 AXP_VHD_Merge(
		AXP_VHD_HANDLE handle,
		AXP_VHD_MERGE_FLAG flags,
		AXP_VHD_MERGE_PARAM *parameters,
		AXP_VHD_ASYNC *async);

/*
 * Initiates a mirror operation for a virtual disk.
 */
u32 AXP_VHD_Mirror(
		AXP_VHD_HANDLE handle,
		AXP_VHD_MIRROR_FLAG flags,
		AXP_VHD_MIRROR_PARAM *param,
		AXP_VHD_ASYNC *async);

/*
 * Modifies the internal contents of a virtual disk file. Can be used to set
 * the active leaf, or to fix up snapshot entries.
 */
u32 AXP_VHD_Modify(
		AXP_VHD_HANDLE handle,
		const AXP_VHD_MOD_PARAM *param,
		AXP_VHD_MOD_FLAG flags);

/*
 * Opens a virtual hard disk (VHD) or CD or DVD image file (ISO) for use.
 */
u32 AXP_VHD_Open(
		AXP_VHD_STORAGE_TYPE *type,
		char *path,
		AXP_VHD_ACCESS_MASK accessMask,
		AXP_VHD_OPEN_FLAG flags,
		AXP_VHD_OPEN_PARAM *param,
		AXP_VHD_HANDLE *handle);

/*
 * Retrieves information about changes to the specified areas of a virtual hard
 * disk (VHD) that are tracked by resilient change tracking (RCT).
 */
u32 AXP_VHD_QueryChanges(
		AXP_VHD_HANDLE handle,
		char *chgTrackID,
		u64 offset,
		u64 len,
		AXP_VHD_QUERY_CHANGES_FLAG flags,
		AXP_VHD_QUERY_CHANGES_RANGE *ranges,
		u32 *rangeCnt,
		u64 *procLen);

/*
 * Issues an embedded SCSI request directly to a virtual hard disk.
 */
u32 AXP_VHD_RawSCSI(
		AXP_VHD_HANDLE handle,
		const AXP_VHD_RAW_SCSI_PARAM *param,
		AXP_VHD_RAW_SCSI_FLAG flags,
		AXP_VHD_RAW_SCSI_RSP *rsp);

/*
 * Sets information about a virtual hard disk (VHD).
 */
u32 AXP_VHD_SetInfo(
		AXP_VHD_HANDLE handle,
		AXP_VHD_SET_INFO_PARAM *info);

/*
 * Creates a snapshot of the current virtual disk for VHD Set files.
 */
u32 AXP_VHD_TakeSnap(
		AXP_VHD_HANDLE handle,
		const AXP_VHD_TAKE_SNAP_PARAM *param,
		AXP_VHD_TAKE_SNAP_FLAG flags);

/*
 * This is not part of the VHD interface but is how a Handle is closed, and
 * thus the VHD closed.  Note, to make sure there are not any other file
 * handles to the VHD, a call to AXP_VHD_Detach should have been called first.
 */
u32 AXP_VHD_CloseHandle(AXP_VHD_HANDLE handle);

/*
 * These are error codes consistent with the Microsoft definitions.
 */
typedef enum
{
    AXP_VHD_SUCCESS = 0,
    AXP_VHD_FILE_NOT_FOUND = 2,
    AXP_VHD_PATH_NOT_FOUND = 3,
    AXP_VHD_ACCESS_DENIED = 5,
    AXP_VHD_INV_HANDLE = 6,
    AXP_VHD_NOT_ENOUGH_MEMORY = 8,
    AXP_VHD_OUTOFMEMORY = 14,
    AXP_VHD_CRC = 23,
    AXP_VHD_SEEK = 25,
    AXP_VHD_SECTOR_NOT_FOUND = 27,
    AXP_VHD_WRITE_FAULT = 29,
    AXP_VHD_READ_FAULT = 30,
    AXP_VHD_HANDLE_EOF = 38,
    AXP_VHD_HANDLE_DISK_FULL = 39,
    AXP_VHD_NOT_SUPPORTED = 50,
    AXP_VHD_FILE_EXISTS = 80,
    AXP_VHD_INV_PARAM = 87,
    AXP_VHD_OPEN_FAILED = 110,
    AXP_VHD_BUFFER_OVERFLOW = 111,
    AXP_VHD_DISK_FULL = 112,
    AXP_VHD_CALL_NOT_IMPL = 120,
    AXP_VHD_INV_NAME = 123,
    AXP_VHD_NEGATIVE_SEEK = 131,
    AXP_VHD_DEVICE_ALREADY_ATTACHED = 548,
    AXP_VHD_ILLEGAL_CHAR = 582,
    AXP_VHD_UNDEFINED_CHAR = 583,
    AXP_VHD_UNSUP_COMPRESSION = 618,
    AXP_VHD_RANGE_NOT_FOUND = 644,
    AXP_VHD_IO_INCOMPLETE = 996,
    AXP_VHD_IO_PENDING = 997,
    AXP_VHD_INV_FLAGS = 1004,
    AXP_VHD_FILE_CORRUPT = 1392,
    AXP_VHD_INV_HANDLE_STATE = 1609,
    AXP_VHD_FILE_ENCRYPTED = 6002,
    AXP_VHD_FILE_READ_ONLY = 6009
} AXP_VHDX_SYS_ERR;

#endif /* _AXP_VDSSYS_H_ */
