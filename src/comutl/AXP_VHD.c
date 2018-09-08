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
 *  This source file contains the code to support the VHD formatted virtual
 *  disk image.
*
 * Revision History:
 *
 *  V01.000	08-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Blocks.h"
#include "AXP_VHD.h"
#include "AXP_VHDX.h"

/*
 * AXP_VHD_Checksum
 *  This function is called to calculate a "checksum", based on the buffer
 *  supplied.  This is taken from Page 17 of the VHD Specification.
 *
 * Input Parameters:
 *  buf:
 *	An array of unsigned bytes, from which to calculate the checksum.
 *  bufLen:
 *	A value indicating the total number of bytes supplied in the buf
 *	parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  The calculated checksum (an inverse).
 */
u32 AXP_VHD_Checksum(u8 *buf, size_t bufLen)
{
    u32		retVal = 0;
    int		ii;

    /*
     * Loop through the bytes, wrapping as needed.
     */
    for (ii = 0; ii < bufLen; ii++)
	retVal += buf[ii];

    /*
     * Return the result back to the caller.
     */
    return(~retVal);
}

/*
 * AXP_VHD_CHSCalc
 *  This function is called to calculate the Cylinder, Heads, Sectors per
 *  track (CHS) value, which is the geometry of the VHD, based on a particular
 *  disk and sector size.  This is taken from Page 16 of the VHD Specification.
 *
 *  NOTE: I performed some code clean-up from the specification.  There were
 *  some missing parentheses and ambiguous coding.
 *
 * Input Parameters:
 *  diskSize:
 *	A value indicating the total number of bytes allocated to the virtual
 *	disk.  This may not be the actual size of the file, if it is dynamic.
 *
 *  sectorSize:
 *	A value indicating the size of each sector on the disk.
 *
 * Output Parameters:
 *  chs:
 *	A pointer to a structure to receive the Cylinder, Heads, and Sector
 *	information associated with the VHD.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_CHSCalc(
		u64 diskSize,
		u32 sectorSize,
		AXP_VHD_DiskGeo *chs)
{
    u32 totSectors = (u32) (diskSize / sectorSize);
    u32 cylinders, heads, sectors;

    /*
     * The maximum number of cylinders is 65,535, the maximum number of heads
     * is 16, and the maximum number of sectors/track is 255.  Therefore, if
     * the total sectors calculated above is greater than the product of these
     * maximums, then we need to reduce the total sectors to account for these
     * maximums.
     *
     *	C:	65535
     *	H:	16
     *	S:	255
     */
    if (totSectors > (65535 * 16 * 255))
	totSectors = 65535 * 16 * 255;

    /*
     * If the total number of sectors is greater than 63 and the heads and
     * cylinders are at their maximum, then set the sectors (S) and heads (H)
     * to their maximum, and determine cylinders.
     */
    if (totSectors >= (65535 * 16 * 63))
    {
	sectors = 255;
	heads = 16;
	cylinders = totSectors / sectors / heads;
    }

    /*
     * OK, we have a but more work to perform to determine the CHS.
     */
    else
    {
	u32 cylHead;	/* Number of cylinders * number of heads */

	sectors = 17;
	cylHead = totSectors / sectors;
	heads = (cylHead + 1023) / 1024;

	/*
	 * Don't let heads go lower than 4.  We'll adjust the other numbers to
	 * compensate.
	 */
	heads = (heads < 4) ? 4 : heads;

	/*
	 * If the number of cylinders/head is greater than or equal to the
	 * number of heads times 1K, or the heads is greater than the maximum,
	 * then we need to do some adjustments to the calculations above.
	 */
	if ((cylHead >= (heads * 1024)) || (heads > 16))
	{
	    sectors = 31;
	    heads = 16;
	    cylHead = totSectors / sectors;
	}

	/*
	 * If the adjustments above, still has the number of cylinders/head
	 * greater than or equal to the number of heads times 1K, then we need
	 * to make a further adjustment (we'll increase the sectors, which will
	 * reduce the number of cylinders/head).
	 */
	if (cylHead >= (heads * 1024))
	{
	    sectors = 63;
	    heads = 16;
	    cylHead = totSectors / sectors;
	}

	/*
	 * Now, we can finally calculate the total number of cylinders present
	 * on the disk.
	 */
	cylinders = cylHead / heads;
    }

    /*
     * Set the return structure accordingly.
     */
    chs->cylinders = cylinders;
    chs->heads = heads;
    chs->sectors = sectors;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * _AXP_VHD_Create
 *  Creates a virtual hard disk (VHD) image file.
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
 *	differential VHD.
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
 *  AXP_VHD_WRITE_FAULT:	An error occurred writing to the VHD file.
 *  AXP_VHD_OUTOFMEMORY:	Insufficient memory to perform operation.
 */
u32 _AXP_VHD_Create(
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
    AXP_VHDX_Handle	*vhd;
    AXP_VHD_Footer	foot;
    AXP_VHD_Dynamic	dyn;
    u64			eofOff;
    time_t		now;
    u32			retVal = AXP_VHD_SUCCESS;
    bool		writeRet = true;

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    vhd = (AXP_VHDX_Handle *) AXP_Allocate_Block(AXP_VHDX_BLK);
    if (vhd != NULL)
    {
	vhd->filePath = AXP_Allocate_Block(-(strlen(path) + 1));
	if (vhd->filePath != NULL)
	{
	    strcpy(vhd->filePath, path);
	    vhd->deviceID = deviceID;
	    vhd->logOffset = 0;
	    vhd->batOffset = sizeof(AXP_VHD_Footer) + sizeof(AXP_VHD_Dynamic);
	    vhd->metadataOffset = 0;
	    vhd->diskSize = diskSize;
	    vhd->blkSize = blkSize;
	    vhd->sectorSize = sectorSize;
	    vhd->fixed = (flags == CREATE_FULL_PHYSICAL_ALLOCATION);
	}
	else
	{
	    AXP_Deallocate_Block(vhd);
	    vhd = NULL;
	}
    }

    /*
     * If we allocated the block we need, then continue out processing.
     * Otherwise, return an error.
     */
    if (vhd != NULL)
    {

	/*
	 * The next call is to see if the file already exists.  If it does
	 * we'll return an error.  Otherwise, we'll try to create the file
	 * we need to create.
	 */
	vhd->fp = fopen(path, "rb");
	if (vhd->fp == NULL)
	{

	    /*
	     * Open the file for write only.  We'll re-open it for read-write
	     * before returning to the caller.
	     */
	    vhd->fp = fopen(path, "wb");
	    if (vhd->fp == NULL)
	    {
		AXP_Deallocate_Block(vhd);
		retVal = AXP_VHD_INV_HANDLE;
	    }
	}
	else
	{
	    AXP_Deallocate_Block(vhd);
	    retVal = AXP_VHD_FILE_EXISTS;
	}
    }
    else
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we get this far, the parameters are good, the handle has been
     * created, and the file has been opened.  Now it's time to initialize it.
     *
     * Dynamic Hard Disk Image						Page 3
     *
     * A dynamic hard disk image is a file that at any given time is as large
     * as the actual data written to it plus the size of the header and footer.
     * Allocation is done in blocks. As more data is written, the file
     * dynamically increases in size by allocating more blocks. For example,
     * the size of file backing a virtual 2-GB hard disk is initially around 2M
     * on the host file system. As data is written to this image, it grows with
     * a maximum size of 2G.
     *
     * Dynamic hard disks store metadata that is used in accessing the user
     * data stored on the hard disk. The maximum size of a dynamic hard disk is
     * 2040G. The actual size is restricted by the underlying disk hardware
     * protocol. For example, ATA hard disks have a 127G limit.
     *
     * The basic format of a dynamic hard disk is shown in the following table.
     *
     *  ---------------------------------------------
     * 	Dynamic Disk header fields	Size (bytes)
     *  ---------------------------------------------
     * 	Copy of hard disk footer	512
     * 	Dynamic Disk Header		1024
     * 	BAT (Block Allocation table)	As needed.
     * 	Data Block 1			blkSize
     * 	Data Block 2			  "
     * 	...
     * 	Data Block n			  "
     * 	Hard Disk Footer		512
     *  ---------------------------------------------
     *
     * Every time a data block is added, the hard disk footer must be moved to
     * the end of the file. Because the hard disk footer is a crucial part of
     * the hard disk image, the footer is mirrored as a header at the front of
     * the file for purposes of redundancy
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

	/*
	 * First initialize the footer.  For fixed file, this is only found at
	 * the end, but for dynamic files it can be found at both ends.
	 *
	 * Hard Disk Footer Format					Page 5
	 *
	 * All hard disk images share a basic footer format. Each hard disk
	 * type extends this format according to its needs.
	 *
	 * The format of the hard disk footer is listed in the following table.
	 *
	 * ---------------------------------------------
	 * Hard disk footer fields	Size (bytes)
	 * ---------------------------------------------
	 * Cookie			8
	 * Features			4
	 * File Format Version		4
	 * Data Offset			8
	 * Time Stamp			4
	 * Creator Application		4
	 * Creator Version		4
	 * Creator Host OS		4
	 * Original Size		8
	 * Current Size			8
	 * Disk Geometry		4
	 * Disk Type			4
	 * Checksum			4
	 * Unique Id			16
	 * Saved State			1
	 * Reserved			427
	 * ---------------------------------------------
	 * Total Size			512
	 * ---------------------------------------------
	 *
	 * NOTE: Versions previous to Microsoft Virtual PC 2004 create disk
	 * images that have a 511-byte disk footer. So the hard disk footer
	 * can exist in the last 511 or 512 bytes of the file that holds the
	 * hard disk image.
	 */
	memset(&foot, 0, sizeof(AXP_VHD_Footer));
	foot.cookie = AXP_VHDFILE_SIG;
	foot.features = AXP_FEATURES_RES;
	foot.formatVer = AXP_FORMAT_VER;
	foot.dataOffset = vhd->fixed ? AXP_FIXED_OFFSET : 0;
	time(&now);
	foot.timestamp = (u32) (now - 946684800); /* secs from 01/01/00 0:00 */
	foot.creator = AXP_VHD_CREATOR;
	foot.creatorVer = AXP_CREATOR_VER;
	foot.creatorHostOS = AXP_CREATOR_HOST;
	foot.originalSize = foot.currentSize = diskSize;
	AXP_VHD_CHSCalc(diskSize, sectorSize, &foot.chs);
	foot.checksum = AXP_VHD_Checksum((u8 *) &foot, sizeof(AXP_VHD_Footer));

	/*
	 * Save the CHS information into the handle.
	 */
	vhd->cylinders = foot.chs.cylinders;
	vhd->heads = foot.chs.heads;
	vhd->sectors = foot.chs.sectors;

	/*
	 * If this is a dynamic file, then we need to create the Dynamic Disk
	 * Header.
	 *
	 * Dynamic Disk Header Format					Page 8
	 *
	 * For dynamic and differencing disk images, the "Data Offset" field
	 * within the image footer points to a secondary structure that
	 * provides additional information about the disk image. The dynamic
	 * disk header should appear on a sector (512-byte) boundary.
	 *
	 * The format of the Dynamic Disk Header is listed in the following
	 * table.
	 *
	 * ---------------------------------------------
	 * Dynamic Disk Header fields	Size (bytes)
	 * ---------------------------------------------
	 * Cookie			8
	 * Data Offset			8
	 * Table Offset			8
	 * Header Version		4
	 * Max Table Entries		4
	 * Block Size			4
	 * Checksum			4
	 * Parent Unique ID		16
	 * Parent Time Stamp		4
	 * Reserved			4
	 * Parent Unicode Name		512
	 * Parent Locator Entry 1	24
	 * Parent Locator Entry 2	24
	 * Parent Locator Entry 3	24
	 * Parent Locator Entry 4	24
	 * Parent Locator Entry 5	24
	 * Parent Locator Entry 6	24
	 * Parent Locator Entry 7	24
	 * Parent Locator Entry 8	24
	 * Reserved			256
	 * ---------------------------------------------
	 * Total Size			1024
	 * ---------------------------------------------
	 */
	if (vhd->fixed == false)
	{
	    u32			unused, ii;
	    u64			curOffset = 0;
	    AXP_VHD_BAT_ENT	blockOff = AXP_VHD_BAT_UNUSED;

	    /*
	     * Because this is a dynamic file, the footer is replicated at the
	     * top of the file and the Dynamic Disk Header record is after
	     * that.  Therefore, the dataOffset for the record at the top of
	     * the file should be set to the location where the Dynamic Disk
	     * Header Record begins.
	     */
	    foot.dataOffset = sizeof(AXP_VHD_Footer);

	    /*
	     * Initialize the Dynamic Disk Header Record
	     */
	    memset(&dyn, 0, sizeof(AXP_VHD_Dynamic));
	    dyn.cookie = AXP_VHD_DYNAMIC_SIG;
	    dyn.dataOff = AXP_VHD_DATA_OFFSET;
	    vhd->batOffset = dyn.tableOff =
		sizeof(AXP_VHD_Footer) + sizeof(AXP_VHD_Dynamic);
	    vhd->batCount = dyn.maxTableEnt = diskSize / blkSize;
	    vhd->batLength = vhd->batCount * sizeof(AXP_VHD_BAT_ENT);
	    dyn.headerVer = AXP_VHD_HEADER_VER;
	    dyn.blockSize = blkSize;
	    dyn.checksum = AXP_VHD_Checksum((u8 *) &dyn, sizeof(AXP_VHD_Dynamic));

	    /*
	     * Now we need to figure out the Block Allocation Table (BAT).  The
	     * BAT always ends on a sector boundary, so we may have some
	     * additional unused BAT entries.
	     *
	     * The number of unused entries is the distance, in u32 values, to
	     * the end of the sector.  This is calculated as follows:
	     *
	     *	sectorSize - ((tableOffset + tableSize) % sectorSize)
	     *	tableSize = maxTableEntries * 4 [size of 32-bit offset value]
	     *
	     */
	    unused = sectorSize -
		((dyn.tableOff + (dyn.maxTableEnt * sizeof(u32))) % sectorSize);

	    vhd->bat = AXP_Allocate_Block(-vhd->batLength);
	    if (vhd->bat != NULL)
	    {
		AXP_VHD_BAT_ENT *batPtr = (AXP_VHD_BAT_ENT *) vhd->bat;

		/*
		 * So we are ready to write out the dynamic portions of the VHD
		 * file.  The initial file will be laid out as follows:
		 *
		 * 	1) Copy of hard disk footer	512
		 * 	2) Dynamic Disk Header		1024
		 * 	3) BAT (Block Allocation table)	As needed.
		 * 	4) Hard Disk Footer		512
		 */
		writeRet = AXP_WriteAtOffset(
				    vhd->fp,
				    &foot,
				    sizeof(AXP_VHD_Footer),
				    curOffset);
		dyn.tableOff = 0; /* there is nothing after the footer record */
		curOffset += sizeof(AXP_VHD_Footer);
		if (writeRet == true)
		    writeRet = AXP_WriteAtOffset(
				    vhd->fp,
				    &dyn,
				    sizeof(AXP_VHD_Dynamic),
				    curOffset);
		curOffset += sizeof(AXP_VHD_Dynamic);
		for (ii = 0;
		     ((ii < (dyn.maxTableEnt + unused)) && (writeRet == true));
		     ii++)
		{
		    batPtr[ii] = blockOff;
		    writeRet = AXP_WriteAtOffset(
				    vhd->fp,
				    &blockOff,
				    sizeof(blockOff),
				    curOffset);
		    curOffset += sizeof(blockOff);
		}
		eofOff = curOffset;
	    }
	    else
		retVal = AXP_VHD_OUTOFMEMORY;
	}
	else
	    eofOff = diskSize;

	/*
	 * Write the footer, where ever it is supposed to end up.
	 */
	if ((writeRet == true) && (retVal == AXP_VHD_SUCCESS))
	    writeRet = AXP_WriteAtOffset(
				vhd->fp,
				&foot,
				sizeof(AXP_VHD_Footer),
				eofOff);
	if ((writeRet == true) && (retVal == AXP_VHD_SUCCESS))
	{
	    vhd->fp = freopen(path, "wb+", vhd->fp);
	    if (vhd->fp == NULL)
	    {
		remove(path);		/* Delete the file */
		AXP_Deallocate_Block(vhd);
		retVal = AXP_VHD_INV_HANDLE;
	    }
	    else
		*handle = (AXP_VHD_HANDLE) vhd;
	}
	else
	{
	    if (vhd->fp != NULL)
		fclose(vhd->fp);	/* Close the file we opened */
	    vhd->fp = NULL;		/* Prevent Deallocate Blocks closing again */
	    remove(path);		/* Delete the file */
	    AXP_Deallocate_Block(vhd);
	    if (retVal == AXP_VHD_SUCCESS)
		retVal = AXP_VHD_WRITE_FAULT;
	}
    }

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_VHD_Open
 *  This function is called to open a VHD virtual disk.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the new VHD
 *	disk image file.
 *  flags:
 *	Open flags, which must be a valid combination of the AXP_VHD_OPEN_FLAG
 *	enumeration.
 *  deviceID:
 *	An unsigned 32-bit value indicating the disk type being opened.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly opened
 *  	VHD disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_FILE_NOT_FOUND:	File Not Found.
 *  AXP_VHD_READ_FAULT:		Failed to read information from the file.
 *  AXP_VHD_OUTOFMEMORY:	Insufficient memory to perform operation.
 *  AXP_VHD_FILE_CORRUPT:	The file appears to be corrupt.
 */
u32 _AXP_VHD_Open(
		char *path,
		AXP_VHD_OPEN_FLAG flags,
		u32 deviceID,
		AXP_VHD_HANDLE *handle)
{
    AXP_VHDX_Handle	*vhd;
    AXP_VHD_Footer	*footer;
    u8			footerBuf[sizeof(AXP_VHD_Footer) + 1];
    i64			fileSize;
    size_t		outLen;
    u32			retVal = AXP_VHD_SUCCESS;
    u32			oldChecksum;
    u32			newChecksum;

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    vhd = (AXP_VHDX_Handle *) AXP_Allocate_Block(AXP_VHDX_BLK);
    if (vhd != NULL)
    {

	/*
	 * Allocate a buffer long enough for for the filename (plus null
	 * character).
	 */
	vhd->filePath = AXP_Allocate_Block(-(strlen(path) + 1));
	if (vhd->filePath != NULL)
	{
	    strcpy(vhd->filePath, path);
	    vhd->deviceID = deviceID;

	    /*
	     * Try and open the file for binary read-only.  We don't know yet
	     * if this file is a valid VHD file and definitely don't want to
	     * write to it (yet).  If everything looks good, then we will
	     * reopen it for binary read/write.
	     */
	    vhd->fp = fopen(path, "rb");
	    if (vhd->fp != NULL)
	    {

		/*
		 * Whether the VHD is dynamic, differencing, or fixed, the last
		 * 512 (or 511) bytes of the file contains a footer.  We use
		 * the footer to determine the validity of the file, as well
		 * as type.  If dynamic or differencing, then we will also read
		 * the header.
		 *
		 *	NOTE:	For VHD formatted virtual hard disks, there is
		 * 		no header record.
		 */
		fileSize = AXP_GetFileSize(vhd->fp);
		if (fileSize >= sizeof(AXP_VHD_Footer))
		{

		    /*
		     * Read in the footer record.  Assume it is in the last 512
		     * bytes of the file.  We'll correct our reference if the
		     * footer is in the last 511 bytes of the file.
		     */
		    outLen = sizeof(AXP_VHD_Footer);
		    if (AXP_ReadFromOffset(
				vhd->fp,
				footerBuf,
				&outLen,
				(fileSize - sizeof(AXP_VHD_Footer))) == true)
		    {

			/*
			 * The footer has a specific structure format.  Cast
			 * the last 512 bytes to the pointer for this
			 * structure.  If the cookie field does not look
			 * correct, shift the cast pointer one byte.  If the
			 * cookie is still not correct, then we have a file
			 * that appears to be corrupt.
			 */
			footer = (AXP_VHD_Footer *) footerBuf;
			if (footer->cookie != AXP_VHDFILE_SIG)
			{
			    footer = (AXP_VHD_Footer *) &footerBuf[1];
			    if (footer->cookie != AXP_VHDFILE_SIG)
				retVal = AXP_VHD_FILE_CORRUPT;
			}

			/*
			 * All right, it appears that we have a valid footer.
			 * so, let's go perform some additional validation of
			 * the footer record.
			 */
			if (retVal == AXP_VHD_SUCCESS)
			{

			    /*
			     * Recalculate the footer checksum.  Note, that the
			     * original checksum was calculated with the
			     * checksum itself, being equal to zero.
			     */
			    oldChecksum = footer->checksum;
			    footer->checksum = 0;
			    newChecksum = AXP_VHD_Checksum(
						(u8 *) footer,
						sizeof(AXP_VHD_Footer));

			    /*
			     * Type to do some validation of the footer.  If
			     * the features and format have the expected
			     * values, and if the checksum matches or if it
			     * does not, then we will check the header (only
			     * for Dynamic and Differencing VHDs) checksum
			     * before deciding that the file is corrupt.
			     * Otherwise, we'll extract some information from
			     * the footer.
			     */
			    if ((footer->features == AXP_FEATURES_RES) &&
				(footer->formatVer == AXP_FORMAT_VER) &&
				(AXP_VHD_TypeValid(footer->diskType)) &&
				(((footer->diskType == DiskFixed) &&
				  (footer->dataOffset == AXP_FIXED_OFFSET)) ||
				 ((footer->diskType != DiskFixed) &&
				  (footer->dataOffset != AXP_FIXED_OFFSET))) &&
				((oldChecksum == newChecksum) ||
				  ((footer->diskType == DiskDynamic) ||
				   (footer->diskType == DiskDifferencing))))
			    {
				vhd->logOffset = 0;
				vhd->logLength = 0;
				vhd->batOffset = 0;
				vhd->batLength = 0;
				vhd->batCount = 0;
				vhd->metadataOffset = 0;
				vhd->metadataLength = 0;
				vhd->diskSize = footer->currentSize;
				vhd->blkSize = 0;
				vhd->cylinders = footer->chs.cylinders;
				vhd->heads = footer->chs.heads;
				vhd->sectors = footer->chs.sectors;
				vhd->sectorSize =
					vhd->diskSize /
					    ((u64) vhd->cylinders *
					     (u64) vhd->heads *
					     (u64) vhd->sectors);
				vhd->fixed = footer->diskType == DiskFixed;
			    }
			    else
				retVal = AXP_VHD_FILE_CORRUPT;
			}

			/*
			 * If we still have what appears to be a valid VHD
			 * file and it is a dynamic or differencing VHD, then
			 * go get the header.
			 */
			if ((retVal == AXP_VHD_SUCCESS) &&
			    (vhd->fixed == false))
			{
			    AXP_VHD_Dynamic	dyn;

			    /*
			     * The footer record is 512 bytes and the dynamic
			     * record is 1024 bytes.  The header record has a
			     * copy of the footer, so the file has to be at
			     * least 2048 bytes in size.  If it is not, then
			     * something is wrong with the file.
			     */
			    if (fileSize > TWO_K)
			    {
				outLen = sizeof(AXP_VHD_Dynamic);
				if (AXP_ReadFromOffset(
						vhd->fp,
						(u8 *) &dyn,
						&outLen,
						sizeof(AXP_VHD_Footer)) == true)
				{

				    /*
				     * Recalculate the dynamic checksum.  Note,
				     * that the original checksum was
				     * calculated with the checksum itself,
				     * being equal to zero.
				     */
				    oldChecksum = dyn.checksum;
				    dyn.checksum = 0;
				    newChecksum = AXP_VHD_Checksum(
							(u8 *) &dyn,
							sizeof(AXP_VHD_Dynamic));
				    if ((dyn.cookie == AXP_VHD_DYNAMIC_SIG) &&
					(dyn.dataOff == AXP_VHD_DATA_OFFSET) &&
					(dyn.headerVer == AXP_VHD_HEADER_VER) &&
					(oldChecksum == newChecksum))
				    {
					vhd->batOffset = dyn.tableOff;
					vhd->batCount = dyn.maxTableEnt;
					vhd->batLength =
					    dyn.maxTableEnt * sizeof(u32);
					vhd->blkSize = dyn.blockSize;
				    }
				    else
					retVal = AXP_VHD_FILE_CORRUPT;
				}
				else
				    retVal = AXP_VHD_READ_FAULT;

				/*
				 * OK, if we still have a success, go read the
				 * BAT information, but first allocate an array
				 * of sufficient size.
				 */
				vhd->bat = AXP_Allocate_Block(-vhd->batLength);
				if (vhd->bat != NULL)
				{
				    outLen = vhd->batLength;
				    if (AXP_ReadFromOffset(
							vhd->fp,
							(u8 *) vhd->bat,
							&outLen,
							vhd->batOffset) == false)
					retVal = AXP_VHD_READ_FAULT;
				}
				else
				    retVal = AXP_VHD_OUTOFMEMORY;
			    }
			    else
				retVal = AXP_VHD_FILE_CORRUPT;
			}
		    }
		    else
			retVal = AXP_VHD_READ_FAULT;
		}
		else
		    retVal = AXP_VHD_READ_FAULT;
	    }
	    else
		retVal = AXP_VHD_FILE_NOT_FOUND;
	}
	else
	    retVal = AXP_VHD_OUTOFMEMORY;
    }
    else
	retVal = AXP_VHD_OUTOFMEMORY;

    /*
     * OK, if we get this far and the return status is still successful, then
     * we need to reopen the file for binary read/write.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
	vhd->fp = freopen(path, "wb+", vhd->fp);
	if (vhd->fp == NULL)
	    retVal = AXP_VHD_INV_HANDLE;
	else
	    *handle = (AXP_VHD_HANDLE) vhd;
    }

    /*
     * OK, if we don't have a success at this point, and we allocated a VHD
     * handle, then deallocate the handle, since the VHD was not successfully
     * opened.
     */
    if ((retVal != AXP_VHD_SUCCESS) && (vhd != NULL))
	AXP_Deallocate_Block(vhd);

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_VHD_ReadSectors
 *  Reads one or more sectors from a virtual hard disk (VHD) image file.
 *
 * Input Parameters:
 *  handle:
 *	A pointer to the handle object that represents the virtual disk from
 *	which to read..
 *  lba:
 *	A value representing the Logical Block Address from where the read is
 *	to be started.
 *  sectorsRead: TODO
 *	A pointer to a value representing the number of sectors to be read from
 *	the VHD.
 *
 * Output Parameters:
 *  sectorsRead: TODO
 *	A pointer to an unsigned 32-bit value to receive the actual number of
 *	bytes read.
 *  outBuf:
 *  	A pointer to an unsigned 8-bit array in which to receive the read in
 *  	data.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_READ_FAULT:		An error occurred reading from the VHD file.
 */
u32 _AXP_VHD_ReadSectors(
		AXP_VHD_HANDLE handle,
		u64 lba,
		u32 *sectorsRead,
		u8 *outBuf)
{
    AXP_VHDX_Handle	*vhd = (AXP_VHDX_Handle *) handle;
    u64			offset;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * If this is a fixed sized VHD, then all the blocks for the disk have been
     * preallocated.  Go ahead and read from the file.
     */
    if (vhd->fixed == true)
    {
	offset = lba * (u64) vhd->sectorSize;
	*sectorsRead *= vhd->sectorSize;
	if (AXP_ReadFromOffset(
			vhd->fp,
			outBuf,
			sectorsRead,
			offset) == true)
	    *sectorsRead /= vhd->sectorSize;
	else
	    retVal = AXP_VHD_READ_FAULT;
    }

    /*
     * OK, we have a dynamic VHD.  We need to determine a few things.  First,
     * is the block we are looking to read in the file.  And second, if it is,
     * then find out where it is located and read it in.  NOTE: There is
     * nothing to say that sectors are continuous, so we are going to have read
     * them in one at a time.
     */
    else
    {
	AXP_VHD_BAT_ENT *bat = (AXP_VHD_BAT_ENT *) vhd->bat;
	u8		*bufPtr = outBuf;
	u32		sectorsPerBlk = vhd->blkSize / vhd->sectorSize;
	u64		blkNum = lba / (u64) sectorsPerBlk;
	u32		bitMapBytes = (7 + sectorsPerBlk) / 8;
	u32		bitMapSects = (bitMapBytes + vhd->sectorSize - 1) /
				vhd->sectorSize;
	u32		sectorsInRead = sectorsPerBlk - lba % sectorsPerBlk;;
	u32		sectorsRem = *sectorsRead;
	u64		bytesInRead = sectorsInRead * vhd->sectorSize;

	*sectorsRead = 0;
	while ((sectorsRem > 0) && (retVal == AXP_VHD_SUCCESS))
	{
	    if (bat[blkNum] == AXP_VHD_BAT_UNUSED)
		memset(bufPtr, 0, bytesInRead);
	    else
	    {
		offset = ((u64) bat[blkNum] + lba % (u64) sectorsPerBlk +
			    (u64) bitMapSects) * (u64) vhd->sectorSize;
		if (AXP_ReadFromOffset(
				vhd->fp,
				bufPtr,
				&bytesInRead,
				offset) == true)
		{
		    /* TODO: finish */
		    sectorsRem -= sectorsInRead;
		    bufPtr = &bufPtr[bytesInRead];
		}
		else
		    retVal = AXP_VHD_READ_FAULT;
	    }
	}
    }

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_VHD_WriteSectors
 *  Writes one or more sectors to a virtual hard disk (VHD) image file.
 *
 * Input Parameters:
 *  handle:
 *	A pointer to the handle object that represents the virtual disk from
 *	which to read..
 *  lba:
 *	A value representing the Logical Block Address from where the read is
 *	to be started.
 *  sectorsWritten: TODO
 *	A pointer to a value representing the number of sectors to be written
 *	to the VHD.
 *  inBuf:
 *  	A pointer to an unsigned 8-bit array to be written to the file.
 *
 * Output Parameters:
 *  sectorsWritten: TODO
 *	A pointer to an unsigned 32-bit value to receive the actual number of
 *	bytes written.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_WRITE_FAULT:	An error occurred writing to the VHD file.
 */
u32 _AXP_VHD_WriteSectors(
		AXP_VHD_HANDLE handle,
		u64 lba,
		u32 *sectorsWritten,
		u8 *inBuf)
{
    AXP_VHDX_Handle	*vhd = (AXP_VHDX_Handle *) handle;
    u64			offset;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * If this is a fixed sized VHD, then all the blocks for the disk have been
     * preallocated.  Go ahead and read from the file.
     */
    if (vhd->fixed == true)
    {
	offset = lba * (u64) vhd->sectorSize;
	*sectorsWritten *= vhd->sectorSize;
	if (AXP_WriteToOffset(
			vhd->fp,
			inBuf,
			sectorsWritten,
			offset) == true)
	    *sectorsWritten /= vhd->sectorSize;
	else
	    retVal = AXP_VHD_WRITE_FAULT;
    }

    /*
     * OK, we have a dynamic VHD.  We need to determine a few things.  First,
     * is the block we are looking to write in the file.  And second, if it is,
     * then find out where it is located and write it.  NOTE: There is
     * nothing to say that sectors are continuous, so we are going to have to
     * write them one at a time.
     */
    else
    {
    }

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}
