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
 *  This source file contains the code needed to support a solid state disk
 *  (SSD).
 *
 * Revision History:
 *
 *  V01.000 05-Aug-2018 Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001 09-Jun-2019 Jonathan D. Belanger
 *  AXP_Allocate_Block, when the block type (length) is negative, needs to
 *  either have a null value or the address of the block being allocated (so
 *  that it can be replaced) provided on the call, or the call will get a
 *  segmentation fault.
 */
#include "CommonUtilities/AXP_Blocks.h"
#include "Devices/VirtualDisks/AXP_SSD.h"

/*
 * _AXP_SSD_Create
 *  Creates a solid state disk (SSD) image file.
 *
 * Input Parameters:
 *  path:
 *      A pointer to a valid string that represents the path to the backing
 *      store file.
 *  flags:
 *      Creation flags, which must be a valid combination of the
 *      AXP_VHD_CREATE_FLAG enumeration.
 *  diskSize:
 *      An unsigned 64-bit value for the size of the disk to be created, in
 *      bytes.
 *  blkSize:
 *      An unsigned 32-bit value for the size of each block.
 *  sectorSize:
 *      An unsigned 32-bit value for the size of each sector.
 *  deviceID:
 *      An unsigned 32-bit value indicating the desired disk type.
 *
 * Output Parameters:
 *  handle:
 *      A pointer to the handle object that represents the newly created
 *      virtual disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:        Normal Successful Completion.
 *  AXP_VHD_FILE_EXISTS:    File already exists.
 *  AXP_VHD_INV_HANDLE:     Failed to create the VHDX file.
 *  AXP_VHD_WRITE_FAULT:    An error occurred writing to the VHDX file.
 *  AXP_VHD_OUTOFMEMORY:    Insufficient memory to perform operation.
 */
u32 _AXP_SSD_Create(char *path,
                    AXP_VHD_CREATE_FLAG flags,
                    u64 diskSize,
                    u32 blkSize,
                    u32 sectorSize,
                    u32 deviceID,
                    AXP_VHD_HANDLE *handle)
{
    AXP_SSD_Handle *ssd;
    AXP_SSD_Geometry header;
    u32 retVal = AXP_VHD_SUCCESS;

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    ssd = (AXP_SSD_Handle *) AXP_Allocate_Block(AXP_SSD_BLK);
    if (ssd != NULL)
    {
        ssd->memory = (u8 *) AXP_Allocate_Block(-diskSize, ssd->memory);
        if (ssd->memory != NULL)
        {

            /*
             * Allocate a buffer long enough for for the filename (plus null
             * character).
             */
            ssd->filePath = AXP_Allocate_Block(-(strlen(path) + 1),
                                               ssd->filePath);
            if (ssd->filePath != NULL)
            {
                strcpy(ssd->filePath, path);
                ssd->deviceID = deviceID;
                ssd->diskSize = diskSize;
                ssd->blkSize = blkSize;
                ssd->sectorSize = sectorSize;
            }
            else
            {
                retVal = AXP_VHD_OUTOFMEMORY;
            }
        }
        else
        {
            retVal = AXP_VHD_OUTOFMEMORY;
        }
    }
    else
    {
        retVal = AXP_VHD_OUTOFMEMORY;
    }

    /*
     * OK, if we still have a success status, go create the backing store file.
     * If it already exists, we return an error.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

        /*
         * First let's see if the backing store file already exists.  If it
         * then return an error.  Otherwise, create the file  for write-binary.
         * Since we are creating the SSD, there is nothing to be initialized.
         */
        ssd->fp = fopen(path, "rb");
        if (ssd->fp == NULL)
        {
            ssd->fp = fopen(path, "wb");
            if (ssd->fp != NULL)
            {
                u64 totalSectors = diskSize / sectorSize;
                u8 smallBuf[8];

                memset(smallBuf, 0, sizeof(smallBuf));
                memset(&header, 0, sizeof(header));
                totalSectors = diskSize / sectorSize;
                ssd->heads = header.heads = 255;
                ssd->sectors = header.sectors = 63;
                ssd->cylinders = header.cylinders = totalSectors /
                        (header.heads * header.sectors);
                header.ID1 = AXP_SSD_SIG1;
                header.diskSize = diskSize;
                header.blkSize = blkSize;
                header.sectorSize = sectorSize;
                ssd->byteZeroOffset = header.byteZeroOffset = sizeof(header);
                header.ID2 = AXP_SSD_SIG2;

                /*
                 * Write the header.
                 */
                if (AXP_WriteAtOffset(ssd->fp,
                                      &header,
                                      sizeof(header),
                                      0) == false)
                {
                    retVal = AXP_VHD_WRITE_FAULT;
                }

                /*
                 * If writing the header succeeded then, write out to the end
                 * so that we can have the entire file written (a static file).
                 */
                else if (AXP_WriteAtOffset(ssd->fp,
                                           smallBuf,
                                           sizeof(smallBuf),
                                           (header.byteZeroOffset +
                                                   diskSize -
                                                   sizeof(smallBuf))) == false)
                {
                    retVal = AXP_VHD_WRITE_FAULT;
                }
            }
            else
            {
                retVal = AXP_VHD_WRITE_FAULT;
            }
        }
        else
        {
            retVal = AXP_VHD_FILE_EXISTS;
        }
    }

    /*
     * OK, if we get this far and the return status is still successful, then
     * we need to reopen the file for binary read/write.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
        ssd->fp = freopen(path, "wb+", ssd->fp);
        if (ssd->fp == NULL)
        {
            retVal = AXP_VHD_INV_HANDLE;
        }
        else
        {
            *handle = (AXP_VHD_HANDLE) ssd;
        }
    }

    /*
     * OK, if we don't have a success at this point, and we allocated a SSD
     * handle, then deallocate the handle, since the SSD or its backing store
     * file were not successfully opened.
     */
    if ((retVal != AXP_VHD_SUCCESS) && (ssd != NULL))
    {
        AXP_Deallocate_Block(ssd);
    }

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_SSD_Open
 *  This function is called to open a solid state disk.
 *
 * Input Parameters:
 *  path:
 *      A pointer to a valid string that represents the path to the backing
 *      store file.
 *  flags:
 *      Open flags, which must be a valid combination of the AXP_VHD_OPEN_FLAG
 *      enumeration.
 *  deviceID:
 *      An unsigned 32-bit value indicating the disk type being opened.
 *
 * Output Parameters:
 *  handle:
 *      A pointer to the handle object that represents the newly opened
 *      VHD disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:        Normal Successful Completion.
 *  AXP_VHD_FILE_NOT_FOUND: File Not Found.
 *  AXP_VHD_READ_FAULT:     Failed to read information from the file.
 *  AXP_VHD_OUTOFMEMORY:    Insufficient memory to perform operation.
 *  AXP_VHD_FILE_CORRUPT:   The file appears to be corrupt.
 */
u32 _AXP_SSD_Open(char *path,
                  AXP_VHD_OPEN_FLAG flags,
                  u32 deviceID,
                  AXP_VHD_HANDLE *handle)
{
    AXP_SSD_Handle *ssd;
    AXP_SSD_Geometry header;
    size_t outLen;
    u32 retVal = AXP_VHD_SUCCESS;

    /*
     * Let's allocate the block we need to maintain access to the virtual disk
     * image.
     */
    ssd = (AXP_SSD_Handle *) AXP_Allocate_Block(AXP_SSD_BLK);
    if (ssd != NULL)
    {

        /*
         * Allocate a buffer long enough for for the filename (plus null
         * character).
         */
        ssd->filePath = AXP_Allocate_Block(-(strlen(path) + 1), ssd->filePath);
        if (ssd->filePath != NULL)
        {
            ssd->fp = fopen(path, "rb");
            if (ssd->fp != NULL)
            {
                outLen = sizeof(header);
                if (AXP_ReadFromOffset(ssd->fp, &header, &outLen, 0) == true)
                {
                    if ((header.ID1 == AXP_SSD_SIG1) &&
                        (header.ID2 == AXP_SSD_SIG2))
                    {
                        ssd->diskSize = header.diskSize;
                        ssd->blkSize = header.blkSize;
                        ssd->sectorSize = header.sectorSize;
                        ssd->byteZeroOffset = header.byteZeroOffset;
                        ssd->cylinders = header.cylinders;
                        ssd->heads = header.heads;
                        ssd->sectors = header.sectors;
                    }
                    else
                    {
                        retVal = AXP_VHD_FILE_CORRUPT;
                    }
                }
                else
                {
                    retVal = AXP_VHD_READ_FAULT;
                }
            }
            else
            {
                retVal = AXP_VHD_FILE_NOT_FOUND;
            }
        }

        /*
         * If we get here with a success status, then we have read in the
         * backing store file, now allocate the memory needed for the SSD and
         * read in the saved SSD data from the file.
         */
        if (retVal == AXP_VHD_SUCCESS)
        {
            ssd->memory = (u8 *) AXP_Allocate_Block(-ssd->diskSize,
                                                    ssd->memory);
            if (ssd->memory != NULL)
            {
                outLen = ssd->diskSize;
                if (AXP_ReadFromOffset(ssd->fp,
                                       ssd->memory,
                                       &outLen,
                                       ssd->byteZeroOffset) == false)
                {
                    retVal = AXP_VHD_READ_FAULT;
                }
            }
            else
            {
                retVal = AXP_VHD_OUTOFMEMORY;
            }
        }
    }

    /*
     * OK, if we get this far and the return status is still successful, then
     * we need to reopen the file for binary read/write.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {
        ssd->fp = freopen(path, "wb+", ssd->fp);
        if (ssd->fp == NULL)
        {
            retVal = AXP_VHD_INV_HANDLE;
        }
        else
        {
            *handle = (AXP_VHD_HANDLE) ssd;
        }
    }

    /*
     * OK, if we don't have a success at this point, and we allocated a SSD
     * handle, then deallocate the handle, since the SSD or its backing store
     * file were not successfully opened.
     */
    if ((retVal != AXP_VHD_SUCCESS) && (ssd != NULL))
    {
        AXP_Deallocate_Block(ssd);
    }

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}
