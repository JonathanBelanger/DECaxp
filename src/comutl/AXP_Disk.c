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
 *  This source file contains the code to emulate a physical disk drive.  This
 *  code does not care how the disk is controlled, that code will be layered on
 *  top of this code.  All this code does is open, close, seek, read, and write
 *  to the emulated disk.
 *
 * Revision History:
 *
 *  V01.000	30-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_Disk.h"

/*
 * AXP_DiskOpen
 *  This function is called to open an emulated disk drive based on the
 *  information provided in the input parameters.  This function returns a
 *  Disk structure that is used to call all subsequent Disk emulation
 *  functions.
 *
 * Input Parameters:
 *  type:
 *	This is a value indicating that the emulated device stores its data in
 *	one of the following locations:
 *		- File
 *		- Memory
 *		- Device
 *  fileName:
 *	This is a pointer to a string containing the file to device name where
 *	the disks contents are read and written.  It can be up to 255
 *	characters long.
 *  serialNumber:
 *	This is a pointer containing the serial number to be assigned to the
 *	emulated disk.  It can be up to 32 characters long.
 *  modelNumber:
 *	This is a pointer containing the model number to be assigned to the
 *	emulated disk.  It can be up to 32 characters long.
 *  revisionNumber:
 *	This is a pointer containing the revision number to be assigned to the
 *	emulated disk.  It can be up to 32 characters long.
 *  isCdrom:
 *	This is a boolean to indicate if the disk is a CD-ROM or magnetic disk.
 *  readOnly:
 *	This is a boolean to indicate if the disk is read-only or read-write.
 *	CD-ROMS are read-only.
 *  diskSize:
 *	This is the number of bytes associated with the disk drive.  This will
 *	be used to create the file, if needed, and calculate other
 *	configuration information for the disk drive.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  NULL:	Failed to allocate the space needed to emulate the disk drive.
 *  !NULL:	Normal Successful Completion.
 */
void *AXP_DiskOpen(
		AXP_DiskType type,
		char *fileName,
		char *serialNumber,
		char *modelNumber,
		char *revisionNumber,
		bool isCdrom,
		bool readOnly,
		u64 diskSize)
{
    void		*retVal = NULL;
    AXP_PhysicalDisk	*disk = NULL;
    off_t		calcSize;
    /*
     * First we need to allocate the block needed to hold the emulated disk
     * information.
     */
    disk = (AXP_PhysicalDisk *) AXP_Allocate_Block(AXP_DISK_BLK);
    if (disk != NULL)
    {

	/*
	 * Save how the disk is going to be emulated (File, Memory, Device).
	 */
	disk->type = type;

	/*
	 * Copy the information about the disk being emulated.
	 */
	strncpy(disk->fileName, fileName, MAX_FILENAME_LEN);
	disk->fileName[MAX_FILENAME_LEN+1] = '\0';
	strncpy(disk->serialNumber, serialNumber, MAX_FILENAME_LEN);
	disk->serialNumber[MAX_SERIALNO_LEN+1] = '\0';
	strncpy(disk->modelNumber, modelNumber, MAX_FILENAME_LEN);
	disk->modelNumber[MAX_SERIALNO_LEN+1] = '\0';
	strncpy(disk->revisionNumber, revisionNumber, MAX_FILENAME_LEN);
	disk->revisionNumber[MAX_SERIALNO_LEN+1] = '\0';

	/*
	 * Now we are getting into specifics of the disk.
	 */
	disk->isCdrom = isCdrom;
	disk->readOnly = isCdrom | readOnly;
	disk->diskSize = diskSize;

	/*
	 * The block size is determined by the kind of device being emulated.
	 */
	disk->blockSize = AXP_BLOCK_SIZE(isCdrom);

	/*
	 * We hard code the number of sectors and heads for all emulated disks.
	 * With this and the block size, we can calculate the number of
	 * cylinders.
	 *
	 * TODO: Are the number of heads and sectors always 8 and 32,
	 * respectively?
	 */
	disk->cylinders =
	    diskSize / disk->blockSize / AXP_DISK_SECTORS/ AXP_DISK_HEADS;

	/*
	 * Because we are dealing with integer math, the cylinder value may
	 * have been rounded down.  If this is the case, add one more cylinder
	 * to allow for all the space requested.
	 */
	calcSize =
	    disk->cylinders * disk->blockSize * AXP_DISK_SECTORS * AXP_DISK_HEADS;
	if (calcSize < diskSize)
	    disk->cylinders++;

	/*
	 * OK, here is where we decide what we need to do based off of type.
	 */
	switch (type)
	{
	    case Memory:
		disk->fp = calloc(disk->diskSize, sizeof(u8));

		/*
		 * If we failed to allocate the memory we need for this device,
		 * then we have a problem.  Clean-up everything.
		 */
		if (disk->fp == NULL)
		{
		    printf(
			"Failed to allocate memory of size %ld, for disk %s\n",
			disk->diskSize,
			disk->modelNumber);
		    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
		    disk = NULL;
		}
		else
		    disk->position = 0;
		break;

	    case File:
	    case Device:

		/*
		 * First try to open the file for read.  This is just to check
		 * if the file exists or not.
		 */
		disk->fp = fopen(disk->fileName, (disk->readOnly ? "rb" :"rb+"));

		/*
		 * If we could not open the file for read, then it apparently
		 * does not exist.  So, let's open one for write and then write
		 * out a full set of bytes to get the file allocated onto the
		 * disk drive.  NOTE: If we are going to a device, we don't
		 * create it, it just exists.  Also, read-only disks cannot be
		 * created, because there is no way to write data into them.
		 */
		if ((disk->fp == NULL) &&
		    (type == File) &&
		    (disk->readOnly == false))
		{
		    printf(
			"Creating file, %s, of size %ld, for disk %s\n",
			disk->fileName,
			disk->diskSize,
			disk->modelNumber);

		    /*
		     * First try and open the file.  If it succeeds, then go
		     * fill it up with all zeros.  Otherwise, report the error
		     * and clean-up everything.
		     */
		    disk->fp = fopen(disk->fileName, "wb");
		    if (disk->fp != NULL)
		    {
			u64	qw = 0;
			int ii, sizeQuad, percent = 0;

			/*
			 * The actual space on the disk has not been created by
			 * just opening the file, so now we write an entire
			 * disks worth of zeros, which will cause the host
			 * operating system to allocate a file large enough for
			 * the emulated disk.
			 */
			sizeQuad = disk->diskSize / sizeof(u64);
			printf(
			    "Writing %ld blocks to %s:   0%%\b\b\b\b",
			    disk->diskSize,
			    disk->modelNumber);
			fflush(stdout);
			for (ii = 0; ii < sizeQuad; ii++)
			{
			    fwrite(&qw, sizeof(u64), 1, disk->fp);
			    if ((ii * 100 / sizeQuad) > percent)
			    {
				percent = ii * 100 / sizeQuad;
			        printf("%3d\b\b\b", percent);
				fflush(stdout);
			    }
			}
			printf("100%%\n");
			fflush(stdout);

			/*
			 * Now, reopen the file in the correct mode.
			 */
			disk->fp = freopen(
					disk->fileName,
					(disk->readOnly ? "rb" :"rb+"),
					disk->fp);
			if (disk->fp == NULL)
			{
			    printf(
				"Failed to reopen file, %s, of size %ld, for disk %s\n",
				disk->fileName,
				disk->diskSize,
				disk->modelNumber);
			    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
			    disk = NULL;
			}
		    }
		    else
		    {
			printf(
			    "Failed to create file, %s, of size %ld, for disk %s\n",
			    disk->fileName,
			    disk->diskSize,
			    disk->modelNumber);
			    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
			disk = NULL;
		    }
		}

		/*
		 * For devices, this should always succeed.  If it did not then
		 * maybe the device does not exist, or is not accessible.
		 * Therefore, clean-up everything we allocated.
		 */
		else if ((disk->fp == NULL) && (type == Device))
		{
		    printf(
			"Failed to open device, %s, for disk %s\n",
			disk->fileName,
			disk->modelNumber);
		    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
		    disk = NULL;
		}

		/*
		 * Read-only devices need to exist prior to getting into this
		 * function.  If they don't, then we should not create one.
		 * What can be done is a read-write disk is created,
		 * initialized with data, then later opened read-only.
		 */
		else if ((disk->fp == NULL) &&
			 (type == File) &&
			 (disk->readOnly == true))
		{
		    printf(
			"Failed to open read-only disk, %s, for disk %s\n",
			disk->fileName,
			disk->modelNumber);
		    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
		    disk = NULL;
		}
		retVal = (void *) disk;
		break;
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_DiskClose
 *  This function is called to close an emulated disk drive.
 *
 * Input Parameters:
 *  disk:
 *	A pointer to the disk structure containing everything that is needed to
 *	maintain the emulated disk as either a file, device, or memory.
 *
 * Output Parameters:
 *  disk:
 *	A pointer to NULL.
 *
 * Return Values:
 *  None.
 */
void AXP_Disk_Close(AXP_PhysicalDisk *disk)
{
    switch (disk->type)
    {
	case Memory:
	    free(disk->fp);
	    break;

	case File:
	case Device:
	    fflush(disk->fp);
	    fclose(disk->fp);
	    break;
    }
    AXP_Deallocate_Block((AXP_BLOCK_DSC *) disk);
    disk = NULL;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_DiskSeek
 *  This function is called to seek to a specific location within the emulated
 *  disk.
 *
 * Input Parameters:
 *  disk:
 *	A pointer to the disk structure containing everything that is needed to
 *	maintain the emulated disk as either a file, device, or memory.
 *  whichByte:
 *	A value representing the actual byte to which to move the drive heads.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  true:	Normal Successful Completion.
 *  false:	Attempt to seek past end of disk.
 */
bool AXP_DiskSeek(AXP_PhysicalDisk *disk, off_t whichByte)
{
    bool	retVal = true;

    if (whichByte <= disk->diskSize)
    {
	switch (disk->type)
	{
	    case Memory:
		disk->position = whichByte;
		break;

	    case File:
	    case Device:
		fseek(disk->fp, whichByte, SEEK_SET);
		break;
	}
    }
    else
	retVal = false;

    /*
     * Return the success or failure of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_DiskRead
 *  This function is called to read from the emulated disk.
 *
 * Input Parameters:
 *  disk:
 *	A pointer to the disk structure containing everything that is needed to
 *	maintain the emulated disk as either a file, device, or memory.
 *  len:
 *	A value number of bytes to be read.
 *
 * Output Parameters:
 *  buf:
 *	A pointer to a location to receive the bytes read in.
 *
 * Return Value:
 *  >=0:	Actual number of bytes read.
 */
size_t AXP_DiskRead(AXP_PhysicalDisk *disk, void *buf, size_t len)
{
    size_t retVal = len;

    switch (disk->type)
    {
	case Memory:
	    if ((disk->position + len) > disk->diskSize)
		retVal = disk->diskSize - disk->position;
	    memcpy(buf, (void *) disk->fp, retVal);
	    break;

	case File:
	case Device:
	    retVal = fread(buf, sizeof(u8), len, disk->fp);
	    break;
    }

    /*
     * Return the success or failure of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_DiskWrite
 *  This function is called to write to the emulated disk.
 *
 * Input Parameters:
 *  disk:
 *	A pointer to the disk structure containing everything that is needed to
 *	maintain the emulated disk as either a file, device, or memory.
 *  buf:
 *	A pointer to a location of bytes to write out.
 *  len:
 *	A value number of bytes to be written.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  >=0:	Actual number of bytes written.
 */
size_t AXP_DiskWrite(AXP_PhysicalDisk *disk, void *buf, size_t len)
{
    size_t retVal = len;

    switch (disk->type)
    {
	case Memory:
	    if ((disk->position + len) > disk->diskSize)
		retVal = disk->diskSize - disk->position;
	    memcpy((void *) disk->fp, buf, retVal);
	    break;

	case File:
	case Device:
	    retVal = fwrite(buf, sizeof(u8), len, disk->fp);
	    break;
    }

    /*
     * Return the success or failure of this call back to the caller.
     */
    return(retVal);
}
