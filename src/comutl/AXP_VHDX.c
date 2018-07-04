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
 *  This source file contains the code to support the VHDX formatted virtual
 *  disk image.
 *
 * Revision History:
 *
 *  V01.000	03-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_virtdisk.h"
#include "AXP_VHDX.h"
#include <iconv.h>
#include <byteswap.h>
#include <arpa/inet.h>

/*
 * AXP_VHD_Create
 *  Creates a virtual hard disk (VHD) image file, either using default
 *  parameters or using an existing virtual disk or physical disk.
 *
 * Input Parameters:
 *  storageType:
 *	A pointer to a VIRTUAL_STORAGE_TYPE structure that contains the desired
 *	disk type and vendor information.
 *  path:
 *	A pointer to a valid string that represents the path to the new virtual
 *	disk image file.
 *  accessMask:
 *	The AXP_VHD_ACCESS_MASK value to use when opening the newly created
 *	virtual disk file.  If the version member of the param parameter is set
 *	to AXP_VHD_CREATE_VER_2 then only the AXP_VHD_ACCESS_NONE (0) value may
 *	be specified.
 *  securityDsc:
 *	An optional pointer to a AXP_VHD_SECURITY_DSC to apply to the virtual
 *	disk image file.  If this parameter is NULL, the parent directory's
 *	security descriptor will be used.
 *  flags:
 *	Creation flags, which must be a valid combination of the
 *	AXP_VHD_CREATE_FLAG enumeration.
 *  providerSpecFlags
 *	Flags specific to the type of virtual disk being created. May be zero
 *	if none are required.
 *  param:
 *	A pointer to a valid AXP_VHD_CREATE_PARAM structure that contains
 *	creation parameter data.
 *  async:
 *	An optional pointer to a valid AXP_VHD_ASYNC structure if asynchronous
 *	operation is desired.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly created
 *  	virtual disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_PARAM:		An invalid parameter or combination of
 *				parameters was detected.
 *  AXP_VHD_FILE_EXISTS:	File already exists.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
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
		AXP_VHD_HANDLE *handle)
{
    AXP_VHDX_Handle	*vhdx = NULL;
    char		*creator = "Digital Alpha AXP Emulator 1.0";
    u8			*outBuf = NULL;
    AXP_VHDX_REG_HDR	*reg;
    AXP_VHDX_REG_ENT	*regMeta, *regBat;
    AXP_VHDX_LOG_HDR	*logHdr;
    AXP_VHDX_BAT_ENT	*batEnt;
    AXP_VHDX_META_HDR	*metaHdr;
    AXP_VHDX_META_ENT	*metaEnt;
    AXP_VHDX_META_FILE	*metaFile;
    AXP_VHDX_META_DISK 	*metaDisk;
    AXP_VHDX_META_SEC	*metaSec;
    u32			retVal = AXP_VHD_SUCCESS;
    u32			chunkRatio, dataBlksCnt, totBATEnt; /* secBitmapBlksCnt; */
    u32			maxSize, blkSize, sectorSize;
    iconv_t		convHandle;
    size_t		convLen, outLen;
    size_t		creatorSize = strlen(creator);
    int			metaOff, batOff, ii;

    /*
     * Let's do some quick parameter checking.
     */
    if ((storageType == NULL) ||
	(param == NULL))
	retVal = AXP_VHD_INV_PARAM;
    else if (((param->ver != CREATE_VER_1) && (param->ver != CREATE_VER_2)) ||
	     ((param->ver == CREATE_VER_2) && (accessMask != ACCESS_NONE)) ||
	     (accessMask == (accessMask & ~ACCESS_ALL)) ||
	     (flags > CREATE_FULL_PHYSICAL_ALLOCATION))
	retVal = AXP_VHD_INV_PARAM;
    else if (param->ver == CREATE_VER_1)
    {
	if (((param->ver_1.blkSize != AXP_VHD_DEF_BLK) &&
	     (param->ver_1.blkSize != AXP_VHD_5KB_BLK) &&
	     (param->ver_1.blkSize != AXP_VHD_2MB_BLK) &&
	     (param->ver_1.sectorSize != AXP_VHD_DEF_SEC) &&
	     (param->ver_1.sectorSize != AXP_VHD_512_SEC) &&
	     (param->ver_1.sectorSize != AXP_VHD_4KB_SEC)) ||
	    (param->ver_1.maxSize < (3 * ONE_M)) ||
	    ((param->ver_1.maxSize % param->ver_1.sectorSize) == 0))
	    retVal = AXP_VHD_INV_PARAM;
	else
	{
	    blkSize = (param->ver_1.blkSize == AXP_VHD_DEF_BLK) ?
		AXP_VHDX_DEF_BLK_SIZE : param->ver_1.blkSize;
	    sectorSize = (param->ver_1.sectorSize == AXP_VHD_DEF_SEC) ?
		AXP_VHDX_DEF_SEC_SIZE : param->ver_1.sectorSize;
	    maxSize = param->ver_1.maxSize;
	}
    }
    else if (param->ver == CREATE_VER_2)
    {
	if (((param->ver_2.blkSize != AXP_VHD_DEF_BLK) &&
	     (param->ver_2.blkSize != AXP_VHD_5KB_BLK) &&
	     (param->ver_2.blkSize != AXP_VHD_2MB_BLK) &&
	     (param->ver_2.sectorSize != AXP_VHD_DEF_SEC) &&
	     (param->ver_2.sectorSize != AXP_VHD_512_SEC) &&
	     (param->ver_2.sectorSize != AXP_VHD_4KB_SEC)) ||
	    (param->ver_2.maxSize < (3 * ONE_M)) ||
	    ((param->ver_2.maxSize % param->ver_2.sectorSize) == 0))
	    retVal = AXP_VHD_INV_PARAM;
	else
	{
	    blkSize = (param->ver_2.blkSize == AXP_VHD_DEF_BLK) ?
		AXP_VHDX_DEF_BLK_SIZE : param->ver_2.blkSize;
	    sectorSize = (param->ver_2.sectorSize == AXP_VHD_DEF_SEC) ?
		AXP_VHDX_DEF_SEC_SIZE : param->ver_2.sectorSize;
	    maxSize = param->ver_2.maxSize;
	}
    }

    /*
     * We'll need this a bit later, but let's go get all the memory we are
     * going to need up front.
     */
    outBuf = (u8 *) calloc(SIXTYFOUR_K, sizeof(u8));

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    if (outBuf != NULL)
	vhdx = (AXP_VHDX_Handle *) AXP_Allocate_Block(AXP_VHDX_BLK);

    /*
     * If we allocated the block we need, then continue out processing.
     * Otherwise, return an error.
     */
    if (vhdx != NULL)
    {

	/*
	 * NOTE: In this implementation the interface is not UTF-16, so we
	 * don't need to do a conversion.  The on-disk information is UTF-16,
	 * and also Big Endian, so we'll need to convert from native to VHD
	 * format when writing to the disk and covert from VHD to native format
	 * when returning information through the interface.
	 */
	convHandle = iconv_open("UTF-16", "ASCII");
	if (convHandle == (iconv_t) -1)
	{
	    switch (errno)
	    {
		case ENOMEM:
		    retVal = AXP_VHD_OUTOFMEMORY;
		    break;

		case EINVAL:
		    retVal = AXP_VHD_INV_PARAM;
		    break;

		default:
		    retVal = AXP_VHD_INV_HANDLE;
		    break;
	    }
	    AXP_Deallocate_Block(&vhdx->header);
	}
	else
	{

	    /*
	     * The next call is to see if the file already exists.  If it does
	     * we'll return an error.  Otherwise, we'll try to create the file
	     * we need to create.
	     */
	    vhdx->fp = fopen(path, "rb");
	    if (vhdx->fp == NULL)
	    {
		vhdx->fp = fopen(path, "wb");
		if (vhdx->fp == NULL)
		{
		    AXP_Deallocate_Block(&vhdx->header);
		    retVal = AXP_VHD_INV_HANDLE;
		}
	    }
	    else
	    {
		AXP_Deallocate_Block(&vhdx->header);
		retVal = AXP_VHD_FILE_EXISTS;
	    }
	}
    }
    else
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we get this far, the parameters are good, the handle has been
     * created, and the file has been opened.  Now it's time to initialize it.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

	/*
	 * Figure 3: The VHDX Header Section Layout
	 *
	 * +----------+----------+----------+----------+----------+----------+
	 * |   File   |   Head   |   Head   |  Region  |  Region  | Reserved |
	 * |    ID    |     1    |     2    |     1    |     2    |          |
	 * +----------+----------+----------+----------+----------+----------+
	 * 0         64         128        192        256        320         1
	 * KB        KB          KB         KB         KB         KB        MB
	 *
	 * We can now initialize the header information within the structure
	 * and the file.
	 */
	memcpy(vhdx->fileID.sig, "vhdxfile", AXP_VHDX_ID_SIG_LEN);
	outLen = AXP_VHDX_CREATOR_LEN;
	convLen = iconv(
		    convHandle,
		    &creator,
		    &creatorSize,
		    (char **) &vhdx->fileID.creator,
		    &outLen);
	if (convLen == -1)
	{
	    switch (errno)
	    {
		case E2BIG:
		    retVal = AXP_VHD_OUTOFMEMORY;
		    break;

		case EINVAL:
		    retVal = AXP_VHD_INV_PARAM;
		    break;

		default:
		    retVal = AXP_VHD_INV_HANDLE;
		    break;
	    }
	    AXP_Deallocate_Block(&vhdx->header);
	}
	else
	{

	    /*
	     * OK, let's write the File Identifier block out at the correct
	     * offset.
	     */
	    memcpy(outBuf, &vhdx->fileID, sizeof(AXP_VHDX_ID));
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_FILE_ID_OFF);

	    /*
	     * Next we set up the header record.  This record is written to the
	     * file twice.  This will be written at offset 64KB and 128KB.
	     */
	    memcpy(vhdx->hdr.sig, "head", AXP_VHDX_SIG_LEN);
	    vhdx->hdr.checkSum = 0;
	    vhdx->hdr.seqNum = 0;
	    uuid_generate(vhdx->hdr.fileWriteGuid.uuid);
	    uuid_clear(vhdx->hdr.fileWriteGuid.uuid);
	    uuid_clear(vhdx->hdr.logGuid.uuid);
	    vhdx->hdr.logVer = htons(AXP_VHDX_LOG_VER);
	    vhdx->hdr.ver = htons(AXP_VHDX_CURRENT_VER);
	    vhdx->hdr.logLen = htonl(AXP_VHDX_LOG_LEN);
	    vhdx->hdr.logOff = bswap_64(AXP_VHDX_LOG_LOC);
	    memset(vhdx->hdr.res_1, 0,AXP_VHDX_RES_1_LEN);
	    vhdx->hdr.checkSum = AXP_Crc32(
			(u8 *) &vhdx->hdr,
			sizeof(AXP_VHDX_HDR),
			false,
			vhdx->hdr.checkSum);
	    vhdx->hdr.checkSum = htonl(vhdx->hdr.checkSum );
	    memset(outBuf, 0, SIXTYFOUR_K);
	    memcpy(outBuf, &vhdx->hdr, sizeof(AXP_VHDX_HDR));

	    /*
	     * OK, let's write the Header 1 and Header 2 blocks out at the
	     * correct offset.
	     */
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_HEADER1_OFF);
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_HEADER2_OFF);

	    /*
	     * Next we do the Region Table Header.  This record is also written
	     * to the file twice.  This will be written at offset 192KB and
	     * 256KB.
	     *
	     * We want a contiguous buffer for these three structures, so we
	     * need to put them in a single buffer, one after the other.
	     */
	    memset(outBuf, 0, SIXTYFOUR_K);
	    reg = (AXP_VHDX_REG_HDR *) outBuf;
	    metaOff = sizeof(AXP_VHDX_REG_HDR);
	    batOff = metaOff + sizeof(AXP_VHDX_REG_HDR);
	    regMeta = (AXP_VHDX_REG_ENT *) &outBuf[metaOff];
	    regBat = (AXP_VHDX_REG_ENT *) &outBuf[batOff];

	    /*
	     * Now we can initialize the region table.
	     */
	    memcpy(reg->sig, "regi", AXP_VHDX_SIG_LEN);
	    reg->entryCnt = htonl(2);

	    /*
	     * Next we need at least 2 Region Table Entries (one for BAT and
	     * one for Metadata).
	     */
	    AXP_VHDX_META_GUID(regMeta->guid);
	    regMeta->fileOff = htonl(AXP_VHDX_METADATA_LOC);
	    regMeta->len = htonl(AXP_VHDX_METADATA_LEN);
	    regMeta->req = 1;

	    AXP_VHDX_BAT_GUID(regBat->guid);
	    regBat->fileOff = bswap_64(AXP_VHDX_BAT_LOC);
	    regBat->len = htonl(AXP_VHDX_BAT_LEN);
	    regBat->req = 1;

	    reg->checkSum = AXP_Crc32(
			outBuf,
			SIXTYFOUR_K,
			false,
			reg->checkSum);
	    reg->checkSum = htonl(reg->checkSum);

	    /*
	     * OK, let's write the Header 1 and Header 2 blocks out at the
	     * correct offset.
	     */
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_REG_TBL_HDR1_OFF);
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_REG_TBL_HDR2_OFF);

	    /*
	     * Next we need to the log area, with a Log Header.
	     */
	    memset(outBuf, 0, SIXTYFOUR_K);
	    logHdr = (AXP_VHDX_LOG_HDR *) outBuf;
	    memcpy(logHdr->sig, "loge",AXP_VHDX_SIG_LEN);
	    logHdr->entryLen = htonl(FOUR_K);
	    logHdr->seqNum = bswap_64(1);
	    uuid_generate(logHdr->logGuid.uuid);
	    logHdr->flushedFileOff = bswap_64(param->ver_1.maxSize);
	    logHdr->lastFileOff = bswap_64(4 * ONE_M);
	    logHdr->checkSum = AXP_Crc32(
			outBuf,
			FOUR_K,
			false,
			logHdr->checkSum);
	    logHdr->checkSum = htonl(logHdr->checkSum);

	    /*
	     * OK, let's write the log header out to the correct offset.
	     */
	    AXP_WriteAtOffset(vhdx->fp, outBuf, FOUR_K, AXP_VHDX_REG_TBL_HDR1_OFF);

	    /*
	     * Now we need to allocate the Block Allocation Table (BAT). The
	     * BAT region must be at least large enough to contain as many
	     * entries as required to describe the possible blocks for a given
	     * virtual disk size (VirtualDiskSize - Section 3.5.2.2).
	     *
	     * The chunk ratio is the number of payload blocks in a chunk, or
	     * equivalently, the number of payload blocks per sector bitmap
	     * block.
	     * 	chunkRatio = (2^23 * logSectorSize)/blockSize
	     *
	     * The number of data blocks can be calculated as:
	     *	dataBlksCnt = ceil(virtDiskSize/blkSize)
	     *
	     * The number of sector bitmap blocks can be calculated as:
	     *	secBitmapBlksCnt = ceil(dataBlksCnt/chunkRatio)
	     *
	     * For a dynamic VHDX, the last BAT entry must locate the last
	     * payload block of the virtual disk. The total number of BAT
	     * entries can be calculated as:
	     * 	totBATEnt = dataBlksCnt + floor((dataBlksCnt-1)/chunkRatio)
	     *
	     * For a differencing VHDX, the last BAT entry must be able to
	     * locate the last sector bitmap block that contains the last
	     * payload sector. The total number of BAT entries can be
	     * calculated as:
	     *	totBATEnt = secBitmapBlksCnt * (chunkRatio + 1)
	     */
	    chunkRatio = ((2^23) * sectorSize) / blkSize;
	    dataBlksCnt = ceil((double) maxSize / (double) blkSize);

	    /*
	     * TODO: How is a differencing VHDX created?
	    secBitmapBlksCnt = ceil((double) dataBlksCnt / (double) chunkRatio);
	     */
	    totBATEnt = dataBlksCnt +
		floor((double) (dataBlksCnt - 1)/(double) chunkRatio);

	    /*
	     * Since we are creating the file, we will repeat the BAT Entry to
	     * account for all possible ones that will ever be needed for the
	     * current virtual disk size.
	     */
	    memset(outBuf, 0, SIXTYFOUR_K);
	    batEnt = (AXP_VHDX_BAT_ENT *) outBuf;
	    batEnt->state = AXP_VHDX_PAYL_BLK_NOT_PRESENT;
	    batEnt->fileOff = 0;

	    /*
	     * Go write out all the BAT entries to the virtual disk.
	     */
	    batOff = AXP_VHDX_BAT_LOC;
	    for (ii = 0; ii < totBATEnt; ii++)
	    {
		AXP_WriteAtOffset(
			vhdx->fp,
			outBuf,
			sizeof(AXP_VHDX_BAT_ENT),
			batOff);
		batOff += sizeof(AXP_VHDX_BAT_ENT);
	    }

	    /*
	     * Now for the fun.  The metadata.  When creating the file, we are
	     * dealing with the following System Metadata:
	     *
	     *	1) File Parameters
	     *	2) Virtual Disk Size
	     *	3) Logical Sector Size
	     *	4) Physical Sector Size
	     */
	    memset(outBuf, 0, SIXTYFOUR_K);

	    /*
	     * So, first things first.  We need a metadata table header that
	     * will be used to describe the total number of metadata entries.
	     * At some point, we'll create a few user metadata entries, but for
	     * now we are just going to create the system ones we care about
	     * (see the list above).
	     */
	    metaHdr = (AXP_VHDX_META_HDR *) outBuf;
	    memcpy(metaHdr->sig, "metadata", AXP_VHDX_ID_SIG_LEN);
	    metaHdr->entryCnt = 4;

	    /*
	     * The first entry is immediately after the header.
	     */
	    metaEnt = (AXP_VHDX_META_ENT *) &outBuf[sizeof(AXP_VHDX_META_HDR)];
	    metaOff = AXP_VHDX_METADATA_START_OFF;
	    for (ii = 0; ii < 4; ii++)
	    {
		metaEnt->isRequired = 1;
		metaEnt->off = metaOff;
		switch (ii)
		{
		    case 0:	/* File Parameters */
			AXP_VHDX_FILE_PARAM_GUID(metaEnt->guid);
			metaEnt->len = sizeof(AXP_VHDX_META_FILE);
			break;

		    case 1:	/* Virtual Disk Size */
			AXP_VHDX_VIRT_DSK_SIZE_GUID(metaEnt->guid);
			metaEnt->len = sizeof(AXP_VHDX_META_DISK);
			break;

		    case 2:	/* Logical Sector Size */
			AXP_VHDX_LOGI_SEC_SIZE_GUID(metaEnt->guid);
			metaEnt->len = sizeof(AXP_VHDX_META_SEC);
			break;

		    case 3:	/* Physical Sector Size */
			AXP_VHDX_PHYS_SEC_SIZE_GUID(metaEnt->guid);
			metaEnt->len = sizeof(AXP_VHDX_META_SEC);
			break;
		}

		/*
		 * Calculate the offset for the next metadata item.
		 */
		metaOff += metaEnt->len;

		/*
		 * Move to the next metadata table entry.
		 */
		metaEnt += sizeof(AXP_VHDX_META_ENT);
	    }

	    /*
	     * Write out the Metadata Table.
	     */
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_METADATA_LOC);

	    /*
	     * Now it's time to write out the Metadata Items.
	     */
	    memset(outBuf, 0, SIXTYFOUR_K);

	    /*
	     * First, the File Parameters.
	     *
	     * TODO: Do we want to have a fixed or dynamic VHDX file?
	     */
	    metaFile = (AXP_VHDX_META_FILE *) outBuf;
	    metaOff = sizeof(AXP_VHDX_META_FILE);
	    metaFile->blkSize = blkSize;

	    /*
	     * Now, Virtual Disk Size.
	     */
	    metaDisk = (AXP_VHDX_META_DISK *) &outBuf[metaOff];
	    metaOff += sizeof(AXP_VHDX_META_DISK);
	    metaDisk->virDskSize = maxSize;

	    /*
	     * Next, Logical Sector Size
	     */
	    metaSec = (AXP_VHDX_META_SEC *) &outBuf[metaOff];
	    metaOff += sizeof(AXP_VHDX_META_SEC);
	    metaSec->secSize = sectorSize;

	    /*
	     * Finally, Physical Sector Size
	     */
	    metaSec = (AXP_VHDX_META_SEC *) &outBuf[metaOff];
	    metaSec->secSize = AXP_VHDX_PHYS_SECTOR_SIZE;

	    /*
	     * Write out the Metadata Items.
	     */
	    AXP_WriteAtOffset(vhdx->fp, outBuf, SIXTYFOUR_K, AXP_VHDX_METADATA_LOC);

	    /*
	     * Before returning to the caller, set the value of the handle to
	     * the address of the VHDX handle.
	     */
	    *handle = (AXP_VHD_HANDLE) vhdx;

	}
    }

    /*
     * If we allocated a buffer, then make sure we free it before we get out of
     * here.
     */
    if (outBuf != NULL)
	free(outBuf);

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_VHD_CloseHandle
 *  Closes an open object handle.
 *
 * Input Parameters:
 *  handle:
 *	A valid handle to an open object.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
 */
u32 AXP_VHD_CloseHandle(AXP_VHD_HANDLE handle)
{
    AXP_VHDX_Handle	*vhdx = (AXP_VHDX_Handle *) handle;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * Verify that we have a proper handle.
     */
    if ((vhdx->header.type == AXP_VHDX_BLK) &&
	(vhdx->header.size == sizeof(AXP_VHDX_Handle)))
    {

	/*
	 * TODO: Verify there are no references to the handle.
	 */
	AXP_Deallocate_Block(&vhdx->header);
    }
    else
	retVal = AXP_VHD_INV_HANDLE;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

