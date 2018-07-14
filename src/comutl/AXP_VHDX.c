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
#include <AXP_VirtualDisk.h>
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_VHDX.h"

/*
 * Local Prototypes
 */
static void _AXP_VHD_CreateCleanup(AXP_VHDX_Handle *, char *);


/*
 * _AXP_VHD_CreateCleanup
 *  This function is called when an error occurs after having allocated the
 *  VHDX handle and potentially created and opened the file.  This function
 *  will close the file, clear the file pointer, and delete the file off of
 *  the system.
 *
 * Input Parameters:
 *  vhdx:
 *	A pointer to the VHDX Handle we used to manage the virtual hard disk.
 *  path:
 *	A pointer to a string for the file we potentially opened.  If we did so
 *	successfully, then we need to delete it.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void _AXP_VHD_CreateCleanup(AXP_VHDX_Handle *vhdx, char *path)
{

    /*
     * Clean-up after ourselves
     */
    if (vhdx->fp != NULL)
	fclose(vhdx->fp);	/* Close the file we opened */
    vhdx->fp = NULL;		/* Prevent Deallocate Blocks closing again */
    remove(path);		/* Delete the file */

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _AXP_VHDX_Create
 *  Creates a virtual hard disk (VHDX) image file.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the new virtual
 *	disk image file.
 *  flags:
 *	Creation flags, which must be a valid combination of the
 *	AXP_VHD_CREATE_FLAG enumeration.
 *  parentPath:
 *	A pointer to a valid string that represents the path to the parent
 *	virtual disk image file.  This means that we are creating a
 *	differential VHDX.
 *  parentDevID:
 *	An unsigned 32-bit value indicating the disk type of the parent.
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
u32 _AXP_VHDX_Create(
		char *path,
		AXP_VHD_CREATE_FLAG flags,
		char *parentPath,
		u32 parentDevID,
		u64 diskSize,
		u32 blkSize,
		u32 sectorSize,
		u32 deviceID,
		AXP_VHD_HANDLE *handle)
{
    AXP_VHDX_Handle	*vhdx = NULL;
    char		*creator = "Digital Alpha AXP Emulator 1.0";
    u8			*outBuf;
    AXP_VHDX_ID		*ID;
    AXP_VHDX_HDR	*hdr;
    AXP_VHDX_REG_HDR	*reg;
    AXP_VHDX_REG_ENT	*regMeta, *regBat;
    AXP_VHDX_LOG_HDR	*logHdr;
    AXP_VHDX_BAT_ENT	*batEnt;
    AXP_VHDX_META_HDR	*metaHdr;
    AXP_VHDX_META_ENT	*metaEnt;
    AXP_VHDX_META_FILE	*metaFile;
    AXP_VHDX_META_DISK 	*metaDisk;
    AXP_VHDX_META_SEC	*metaSec;
    AXP_VHDX_META_PAGE83 *meta83;
    AXP_VHDX_META_PAR_HDR *metaParHdr;
    AXP_VHDX_META_PAR_ENT *metaParEnt;
    u64			chunkRatio;
    u32			retVal = AXP_VHD_SUCCESS;
    u32			dataBlksCnt, totBATEnt, secBitmapBlksCnt;
    size_t		outLen;
    size_t		creatorSize = strlen(creator);
    int			metaOff, batOff, ii;
    bool		writeRet = false;

    /*
     * We'll need this a bit later, but let's go get all the memory we are
     * going to need up front.
     */
    outBuf = (u8 *) calloc(SIXTYFOUR_K, 1);

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    if (outBuf != NULL)
	vhdx = (AXP_VHDX_Handle *) AXP_Allocate_Block(AXP_VHDX_BLK);
    if (vhdx != NULL)
    {
	vhdx->filePath = calloc(1, strlen(path) + 1);
	if (vhdx->filePath != NULL)
	{
	    strcpy(vhdx->filePath, path);
	    vhdx->deviceID = deviceID;
	    vhdx->logOffset = AXP_VHDX_LOG_LOC;
	    vhdx->batOffset = AXP_VHDX_BAT_LOC;
	    vhdx->metadataOffset = AXP_VHDX_META_LOC;
	    vhdx->diskSize = diskSize;
	    vhdx->blkSize = blkSize;
	    vhdx->sectorSize = sectorSize;
	    vhdx->fixed = (flags == CREATE_FULL_PHYSICAL_ALLOCATION);
	}
	else
	{
	    AXP_Deallocate_Block(&vhdx->header);
	    vhdx = NULL;
	}
    }

    /*
     * If we allocated the block we need, then continue out processing.
     * Otherwise, return an error.
     */
    if (vhdx != NULL)
    {

	/*
	 * The next call is to see if the file already exists.  If it does
	 * we'll return an error.  Otherwise, we'll try to create the file
	 * we need to create.
	 */
	vhdx->fp = fopen(path, "rb");
	if (vhdx->fp == NULL)
	{

	    /*
	     * Open the file for write only.  We'll re-open it for read-write
	     * before returning to the caller.
	     */
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
    else if (retVal == AXP_VHD_SUCCESS)
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we get this far, the parameters are good, the handle has been
     * created, and the file has been opened.  Now it's time to initialize it.
     *
     * We will create the following minimal regions in the VHDX file.  These
     * regions are:
     *	1) Header Region
     *	2) Log Region
     *	3) BAT Region
     *	4) Metadata Region
     *
     * 3.1 - Header Section						Page 14
     * The header section contains four items: the file type identifier, two
     * headers, and the region table.
     *
     * Figure 3: The VHDX Header Section Layout
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
    if (retVal == AXP_VHD_SUCCESS)
    {
	i32 convRet;

	ID = (AXP_VHDX_ID *) outBuf;
	ID->sig= AXP_VHDXFILE_SIG;
	outLen = AXP_VHDX_CREATOR_LEN * sizeof(uint16_t);
	convRet = AXP_Ascii2UTF_16(creator, creatorSize, ID->creator, &outLen);
	switch (convRet)
	{
	    case E2BIG:
	    case EMFILE:
	    case ENFILE:
	    case ENOMEM:
		retVal = AXP_VHD_OUTOFMEMORY;
		break;

	    case EINVAL:
	    case EILSEQ:
	    case EBADF:
		retVal = AXP_VHD_INV_PARAM;
		break;

	    default:
		break;
	}
	if (retVal != AXP_VHD_SUCCESS)
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	}
	else
	{

	    /*
	     * OK, let's write the File Identifier block out at the correct
	     * offset.
	     */
	    writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				SIXTYFOUR_K,
				AXP_VHDX_FILE_ID_OFF);
	    if (writeRet == false)
	    {
		_AXP_VHD_CreateCleanup(vhdx, path);
		AXP_Deallocate_Block(&vhdx->header);
		retVal = AXP_VHD_WRITE_FAULT;
	    }
	}
    }

    /*
     * Next we set up the header record.  This record is written to the file
     * twice.  This will be written at offset 64KB and 128KB.
     *
     * 3.1.2 - Headers							Page 14
     * Since the header is used to locate the log, updates to the headers
     * cannot be made through the log. To provide power failure consistency
     * guarantees, there are two headers in every VHDX file.
     *
     * Each of the two headers is a 4 KB structure that is aligned to 64KB
     * boundary.  One header is stored at offset 64 KB and the other at 128KB.
     * Only one header is considered current and in use at any point in time.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	memset(outBuf, 0, SIXTYFOUR_K);
	hdr = (AXP_VHDX_HDR *) outBuf;
	hdr->sig = AXP_HEAD_SIG;
	AXP_VHD_SetGUIDDisk(&hdr->fileWriteGuid);
	AXP_VHD_SetGUIDDisk(&hdr->dataWriteGuid);
	hdr->logVer = AXP_VHDX_LOG_VER;
	hdr->ver = AXP_VHDX_CURRENT_VER;
	hdr->logLen = AXP_VHDX_LOG_LEN;
	hdr->logOff = AXP_VHDX_LOG_LOC;
	hdr->checkSum = AXP_Crc32(
				outBuf,
				AXP_VHDX_HDR_LEN,
				false,
				hdr->checkSum);

	/*
	 * OK, let's write the Header 1 and Header 2 blocks out at the
	 * correct offset.
	 */
	writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				SIXTYFOUR_K,
				AXP_VHDX_HEADER1_OFF);
	if (writeRet == true)
	    writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				SIXTYFOUR_K,
				AXP_VHDX_HEADER2_OFF);
	if (writeRet == false)
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	    retVal = AXP_VHD_WRITE_FAULT;
	}
    }

    /*
     * Next we do the Region Table Header.  This record is also written
     * to the file twice.
     *
     * 3.1.3 - Region Table						Page 17
     * The region table consists of a header followed by a variable number of
     * entries, which specify the identity and location of regions within the
     * file. There are two copies of the region table stored at file offset
     * 192KB and 256 KB. Updates to the region table structures must be made
     * through the log.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	memset(outBuf, 0, SIXTYFOUR_K);
	reg = (AXP_VHDX_REG_HDR *) outBuf;
	batOff = AXP_VHDX_REG_HDR_LEN;
	metaOff = batOff + AXP_VHDX_REG_ENT_LEN;
	regBat = (AXP_VHDX_REG_ENT *) &outBuf[batOff];
	regMeta = (AXP_VHDX_REG_ENT *) &outBuf[metaOff];

	/*
	 * Now we can initialize the region table.
	 */
	reg->sig = AXP_REGI_SIG;
	reg->entryCnt = 2;

	/*
	 * Next we need at least 2 Region Table Entries (one for BAT and
	 * one for Metadata).
	 */
	AXP_VHD_KnownGUIDDisk(AXP_Block_Allocation_Table, &regBat->guid);
	regBat->fileOff = AXP_VHDX_BAT_LOC;
	regBat->len = AXP_VHDX_BAT_LEN;
	regBat->req = 1;

	AXP_VHD_KnownGUIDDisk(AXP_Metadata_Region, &regMeta->guid);
	regMeta->fileOff = AXP_VHDX_META_LOC;
	regMeta->len = AXP_VHDX_META_LEN;
	regMeta->req = 1;

	reg->checkSum = AXP_Crc32(
			outBuf,
			SIXTYFOUR_K,
			false,
			reg->checkSum);

	/*
	 * OK, let's write the Header 1 and Header 2 blocks out at the
	 * correct offset.
	 */
	writeRet = AXP_WriteAtOffset(
			vhdx->fp,
			outBuf,
			SIXTYFOUR_K,
			AXP_VHDX_REG_TBL_HDR1_OFF);
	if (writeRet == true)
	    AXP_WriteAtOffset(
			vhdx->fp,
			outBuf,
			SIXTYFOUR_K,
			AXP_VHDX_REG_TBL_HDR2_OFF);
	if (writeRet == false)
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	    retVal = AXP_VHD_WRITE_FAULT;
	}
    }

    /*
     * Now we need to allocate the Block Allocation Table (BAT).
     *
     * 3.4 - BAT							Page 24
     * BAT is a region consisting of a single array of 64-bit values, with an
     * entry for each block that determines the state and file offset of that
     * block. The entries for the payload block and sector bitmap block are
     * interleaved in a way that the sector bitmap block entry associated with
     * a chunk follows the entries for the payload blocks in that chunk. For
     * example, if the chunk ratio is 4, the table’s interleaving would look
     * like by the figure below.
     *
     *	Figure 6: BAT Layout Example
     *	+---+---+---+---+---+---+---+---+---+---+----------------------+
     *	|PB0|PB1|PB2|PB3|SB0|PB4|PB5|PB6|PB7|SB1|                      |
     *	+---+---+---+---+---+---+---+---+---+---+----------------------+
     *	 PB = Payload Block
     *	 SB = Sector Bitmap Block
     *
     * The BAT region must be at least large enough to contain as many entries
     * as required to describe the possible blocks for a given virtual disk
     * size (VirtualDiskSize - Section 3.5.2.2).
     *
     * The chunk ratio is the number of payload blocks in a chunk, or
     * equivalently, the number of payload blocks per sector bitmap block.
     *	chunkRatio = (2^23 * logSectorSize)/blockSize
     *
     * The number of data blocks can be calculated as:
     *	dataBlksCnt = ceil(virtDiskSize/blkSize)
     *
     * The number of sector bitmap blocks can be calculated as:
     *	secBitmapBlksCnt = ceil(dataBlksCnt/chunkRatio)
     *
     * For a dynamic VHDX, the last BAT entry must locate the last payload
     * block of the virtual disk. The total number of BAT entries can be
     * calculated as:
     *	totBATEnt = dataBlksCnt + floor((dataBlksCnt-1)/chunkRatio)
     *
     * For a differencing VHDX, the last BAT entry must be able to locate the
     * last sector bitmap block that contains the last payload sector. The
     * total number of BAT entries can be calculated as:
     *	totBATEnt = secBitmapBlksCnt * (chunkRatio + 1)
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	u64	blkOffset = (vhdx->fixed ? AXP_VHDX_DATA_LOC : 0);
	u64	batState = (vhdx->fixed ?
			AXP_VHDX_PAYL_BLK_FULLY_PRESENT :
			AXP_VHDX_PAYL_BLK_NOT_PRESENT);;

	chunkRatio = (8 * ONE_M * (u64) sectorSize) / (u64) blkSize;
	dataBlksCnt = ceil((double) diskSize / (double) blkSize);

	/*
	 * If we don't have a parent disk, then we are not creating a
	 * differencing VHDX.  Otherwise, we are.  The calculation for the
	 * total number of BAT entries is different depending upon this
	 * distinction.
	 */
	if (parentPath == NULL)
	    totBATEnt = dataBlksCnt +
		    floor((double) (dataBlksCnt - 1)/(double) chunkRatio);
	else
	{
	    secBitmapBlksCnt = ceil((double) dataBlksCnt / (double) chunkRatio);
	    totBATEnt = secBitmapBlksCnt * (chunkRatio + 1);
	}

	/*
	 * Since we are creating the file, we will repeat the BAT Entry to
	 * account for all possible ones that will ever be needed for the
	 * current virtual disk size.
	 */
	memset(outBuf, 0, SIXTYFOUR_K);
	batEnt = (AXP_VHDX_BAT_ENT *) outBuf;
	batEnt->state = batState;
	batEnt->fileOff = blkOffset / ONE_M;

	/*
	 * Go write out all the BAT entries to the virtual disk.
	 */
	batOff = AXP_VHDX_BAT_LOC;
	for (ii = 0; ((ii < totBATEnt) && (writeRet == true)); ii++)
	{

	    /*
	     * If we are writing a Sector Bitmap Block, then we need to make
	     * sure the state and offset are correct.
	     */
	    if ((ii != 0) && ((ii % chunkRatio) == 0))
	    {
		batEnt->state = AXP_VHDX_SB_BLK_NOT_PRESENT;
		batEnt->fileOff = 0;
	    }
	    writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				AXP_VHDX_BAT_ENT_LEN,
				batOff);

	    /*
	     * If we just wrote a Sector Bitmap Block, then we need to set the
	     * state and offset back to what it should be for a Payload Block.
	     */
	    if ((ii != 0) && ((ii % chunkRatio) == 0))
	    {
		batEnt->state = batState;
		batEnt->fileOff = blkOffset / ONE_M;
	    }
	    batOff += AXP_VHDX_BAT_ENT_LEN;

	    if (vhdx->fixed == true)
		blkOffset += blkSize;
	}
	if (writeRet == false)
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	    retVal = AXP_VHD_WRITE_FAULT;
	}
    }

    /*
     * 3.5 - Metadata Region						Page 29
     * The metadata region consists of a fixed-size, 64 KB, unsorted metadata
     * table, followed by unordered, variable-sized, unaligned metadata items
     * and free space. The metadata items represent both user and system
     * metadata, which are distinguished by a bit in the table.
     *
     * Figure 7: Metadata Region Layout Example
     * +-----+---+---+---+---+--+------+-+---+---+----+------+----+---+
     * | MH  |SE1|UE2|SE3|SE4|  | SM 1 | |SM2|   |UM 3|      |SM 4|   |
     * +-----+---+---+---+---+--+------+-+---+---+----+------+----+---+
     *         v   v   v   v       ^       ^       ^           ^
     *         |   |   |   |       |       |       |           |
     *         |   |   |   +-------|-------+       |           |
     *         |   |   +-----------|---------------|-----------+
     *         |   +---------------|---------------+
     *         +-------------------+
     *	MH = Metadata Header		      SM = System Metadata Item
     *	SE = System Metadata Entry	      UM = User Metadata Item
     *	UE = User Metadata Entry
     *
     * Now for the fun.  The metadata.  When creating the file, we are dealing
     * with the following System Metadata:
     *
     *	1) File Parameters
     *	2) Virtual Disk Size
     *	3) Logical Sector Size
     *	4) Physical Sector Size
     *	5)
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	memset(outBuf, 0, SIXTYFOUR_K);

	/*
	 * So, first things first.  We need a metadata table header that will
	 * be used to describe the total number of metadata entries.  At some
	 * point, we'll create a few user metadata entries, but for now we are
	 * just going to create the system ones we care about (see the list
	 * above).
	 */
	metaHdr = (AXP_VHDX_META_HDR *) outBuf;
	metaHdr->sig= AXP_METADATA_SIG;
	metaHdr->entryCnt = (parentPath != NULL) ? 6: 5;

	/*
	 * The first entry is immediately after the header.
	 */
	metaEnt = (AXP_VHDX_META_ENT *) &outBuf[AXP_VHDX_META_HDR_LEN];
	metaOff = AXP_VHDX_META_START_OFF;
	for (ii = 0; ii < metaHdr->entryCnt; ii++)
	{
	    metaEnt->isRequired = 1;
	    metaEnt->off = metaOff;
	    switch (ii)
	    {
		case 0:	/* File Parameters */
		    AXP_VHD_KnownGUIDDisk(AXP_File_Parameter, &metaEnt->guid);
		    metaEnt->len = AXP_VHDX_META_FILE_LEN;
		    break;

		case 1:	/* Virtual Disk Size */
		    AXP_VHD_KnownGUIDDisk(AXP_Disk_Size, &metaEnt->guid);
		    metaEnt->len = AXP_VHDX_META_DISK_LEN;
		    metaEnt->isVirtualDisk = 1;
		    break;

		case 2:	/* Logical Sector Size */
		    AXP_VHD_KnownGUIDDisk(AXP_Logical_Sector, &metaEnt->guid);
		    metaEnt->len = AXP_VHDX_META_SEC_LEN;
		    metaEnt->isVirtualDisk = 1;
		    break;

		case 3:	/* Physical Sector Size */
		    AXP_VHD_KnownGUIDDisk(AXP_Physical_Sector, &metaEnt->guid);
		    metaEnt->len = AXP_VHDX_META_SEC_LEN;
		    metaEnt->isVirtualDisk = 1;
		    break;

		case 4: /* Paga 83 */
		    AXP_VHD_KnownGUIDDisk(AXP_Page_83, &metaEnt->guid);
		    metaEnt->len = AXP_VHDX_META_PAGE83_LEN;
		    metaEnt->isVirtualDisk = 1;
		    break;

		case 5: /* Parent Locator */
		    AXP_VHD_KnownGUIDDisk(AXP_Parent_Locator, &metaEnt->guid);
		    metaEnt->len =
			AXP_VHDX_META_PAR_HDR_LEN + AXP_VHDX_META_PAR_ENT_LEN;
		    metaEnt->isVirtualDisk = 1;
		    break;
	    }

	    /*
	     * Calculate the offset for the next metadata item.
	     */
	    metaOff += metaEnt->len;

	    /*
	     * Move to the next metadata table entry.
	     */
	    metaEnt++;
	}

	/*
	 * Write out the Metadata Table.
	 */
	 writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				SIXTYFOUR_K,
				AXP_VHDX_META_LOC);
	 if (writeRet == false)
	 {
	     _AXP_VHD_CreateCleanup(vhdx, path);
	     AXP_Deallocate_Block(&vhdx->header);
	     retVal = AXP_VHD_WRITE_FAULT;
	 }
    }

    /*
     * 3.5.2 - Known Metadata Items					Page 31
     * There are certain Metadata items that are defined in this specification,
     * some of which are optional and some of which are required. The table
     * below summarizes the known metadata items and their optional or required
     * state.
     *
     *	Table 7: Known Metadata Item Properties
     *	--------------------------------------------------------------------------------
     *	Known							IsUser	Is	Is
     *	Items		GUID						Virtual	Required
     *	--------------------------------------------------------------------------------
     *	File Parameters	CAA16737-FA36-4D43-B3B6-33F0AA44E76B	False	False	True
     *	Virtual Disk	2FA54224-CD1B-4876-B211-5DBED83BF4B8	False	True	True
     *	Size
     *	Page 83 Data	BECA12AB-B2E6-4523-93EF-C309E000C746	False	True	True
     *	Logical Sector	8141BF1D-A96F-4709-BA47-F233A8FAAB5F	False	True	True
     *	Size
     *	Physical Sector	CDA348C7-445D-4471-9CC9-E9885251C556	False	True	True
     *	Size
     *	Parent Locator	A8D35F2D-B30B-454D-ABF7-D3D84834AB0C	False	False	True
     *	--------------------------------------------------------------------------------
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

	/*
	 * Now it's time to write out the Metadata Items.
	 */
	memset(outBuf, 0, SIXTYFOUR_K);

	/*
	 * First, the File Parameters.
	 */
	metaFile = (AXP_VHDX_META_FILE *) outBuf;
	metaOff = AXP_VHDX_META_FILE_LEN;
	metaFile->leaveBlksAlloc = (vhdx->fixed) ? 1 : 0;
	metaFile->hasParent = (parentPath != NULL) ? 1 : 0;
	metaFile->blkSize = blkSize;

	/*
	 * Now, Virtual Disk Size.
	 */
	metaDisk = (AXP_VHDX_META_DISK *) &outBuf[metaOff];
	metaOff += AXP_VHDX_META_DISK_LEN;
	metaDisk->virDskSize = diskSize;

	/*
	 * Next, Logical Sector Size
	 */
	metaSec = (AXP_VHDX_META_SEC *) &outBuf[metaOff];
	metaOff += AXP_VHDX_META_SEC_LEN;
	metaSec->secSize = sectorSize;

	/*
	 * Second to last, Physical Sector Size
	 */
	metaSec = (AXP_VHDX_META_SEC *) &outBuf[metaOff];
	metaOff += AXP_VHDX_META_SEC_LEN;
	metaSec->secSize = AXP_VHDX_PHYS_SEC_SIZE;

	/*
	 * Last required item, Page 83 Data
	 */
	meta83 = (AXP_VHDX_META_PAGE83 *) &outBuf[metaOff];
	metaOff += AXP_VHDX_META_PAGE83_LEN;
	AXP_VHD_SetGUIDDisk(&meta83->pg83Data);

	/*
	 * Optionally, the Parent Locator.
	 */
	if (parentPath != NULL)
	{
	    char *key = "absolute_win32_path";

	    /*
	     * Parent Locator Header
	     */
	    metaParHdr = (AXP_VHDX_META_PAR_HDR *) &outBuf[metaOff];
	    metaOff += AXP_VHDX_META_PAR_HDR_LEN;
	    AXP_VHD_KnownGUIDDisk(AXP_ParentLocator_Type, &metaEnt->guid);
	    metaParHdr->keyValCnt = 1;

	    /*
	     * Parent Locator Entry
	     */
	    metaParEnt = (AXP_VHDX_META_PAR_ENT *) &outBuf[metaOff];
	    metaOff += AXP_VHDX_META_PAR_ENT_LEN;
	    metaParEnt->keyLen = strlen(key);
	    metaParEnt->valLen = strlen(parentPath);
	    metaParEnt->keyOff = metaOff;
	    metaOff += metaParEnt->keyLen;
	    metaParEnt->valOff = metaOff;
	    metaOff += metaParEnt->valLen;
	    strncpy(&outBuf[metaParEnt->keyOff], key, metaParEnt->keyLen);
	    strncpy(&outBuf[metaParEnt->valOff], parentPath, metaParEnt->valLen);
	}

	/*
	 * Write out the Metadata Items.
	 */
	writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				SIXTYFOUR_K,
				(AXP_VHDX_META_LOC + AXP_VHDX_META_START_OFF));

	if ((writeRet == true) && (vhdx->fixed == true))
	{
	    writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				"\0",
				1,
				AXP_VHDX_DATA_LOC + vhdx->diskSize - 1);
	}

	/*
	 * Before returning to the caller, set the value of the handle to
	 * the address of the VHDX handle.
	 */
	if (writeRet == true)
	    *handle = (AXP_VHD_HANDLE) vhdx;
	else
	 {
	     _AXP_VHD_CreateCleanup(vhdx, path);
	     AXP_Deallocate_Block(&vhdx->header);
	     retVal = AXP_VHD_WRITE_FAULT;
	 }
    }

    /*
     * We need to the write the log, with a Log Header, area last.  This is
     * because the Flushed File Offset and Last File Offset need to consider
     * the entire file size at the time the log entry was written.  In order to
     * be able to do this, we need to have written everything above and now we
     * can get the current file size and use this to calculate these values.
     *
     * 3.2 - Log							Page 18
     * The log is a single circular buffer stored contiguously at a
     * location that is specified in the VHDX header. It consists of an
     * ordered sequence of variable-sized entries, each of which represents
     * a set of 4 KB sector updates that need to be performed to the VHDX
     * structures.
     *
     * Figure 4: Log Layout Example
     *
     * |Older Sequence|         Active Sequence         |Older Sequence|
     * +--------------+-------+-----+---+-----+---------+--------------+
     * |              |   N   | N+1 |N+2| N+3 |   N+4   |              |
     * +--------------+-------+-----+---+-----+---------+--------------+
     *                  Tail                      Head
     *                   ^      v    v    v        v
     *                   |      |    |    |        |
     *                   +------+    |    |        |
     *                   +-----------+    |        |
     *                   +----------------+        |
     *                   +-------------------------+
     *
     * 3.2.1.1 Entry Header						Page 20
     * The FlushedFileOffset field stores the VHDX file size in bytes that must
     * be at least as large as the size of the VHDX file at the time the log
     * entry was written. The file size specified in the log entry must have
     * been stable on the host disk such that, even in the case of a system
     * power failure, a non-corrupted VHDX file will be at least as large as
     * the size specified by the log entry. Before shrinking a file while the
     * log is in use, a parser must write the target size to a log entry and
     * flush the entry so that the update is stable on the log on the host disk
     * storage media; this will ensure that the VHDX file is not treated as
     * truncated during log replay. A parser should write the largest possible
     * value that satisfies these requirements. The value must be a multiple
     * of 1MB.
     *
     * The LastFileOffset field stores a file size in bytes that all allocated
     * file structures fit into, at the time the log entry was written. A
     * parser should write the smallest possible value that satisfies these
     * requirements. The value must be a multiple of 1MB.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	u64	fileSize = AXP_VHD_PerformFileSize(vhdx->fp);

	memset(outBuf, 0, SIXTYFOUR_K);
	logHdr = (AXP_VHDX_LOG_HDR *) outBuf;
	logHdr->sig = AXP_LOGE_SIG;
	logHdr->entryLen = FOUR_K;
	logHdr->seqNum = 1;
	AXP_VHD_SetGUIDDisk(&logHdr->logGuid);
	logHdr->flushedFileOff = logHdr->lastFileOff = fileSize;
	logHdr->checkSum = AXP_Crc32(
				outBuf,
				FOUR_K,
				false,
				logHdr->checkSum);

	/*
	 * OK, let's write the log header out to the correct offset.
	 */
	writeRet = AXP_WriteAtOffset(
				vhdx->fp,
				outBuf,
				FOUR_K,
				AXP_VHDX_LOG_LOC);
	if ((writeRet == false) || (fileSize == 0))
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	    retVal = AXP_VHD_WRITE_FAULT;
	}
    }

    if (retVal == AXP_VHD_SUCCESS)
    {
	vhdx->fp = freopen(path, "wb+", vhdx->fp);
	if (vhdx->fp == NULL)
	{
	    _AXP_VHD_CreateCleanup(vhdx, path);
	    AXP_Deallocate_Block(&vhdx->header);
	    retVal = AXP_VHD_INV_HANDLE;
	}
    }

    /*
     * Free what we allocated before we get out of here.
     */
    if (outBuf != NULL)
	free(outBuf);

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}
