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
 *  This source file contains the functions required to support accessing
 *  either a device (disk) or CD in its raw form.  We do this for devices
 *  because we want those devices to ultimately look and be formatted just like
 *  the real thing.  We do this for CDs, because the format for these has been
 *  predetermined and is a standard, so let the operating system in the
 *  emulator handle the details for both of these disk types.
 *
 * Revision History:
 *
 *  V01.000	15-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "Devices/VirtualDisks/AXP_RAW.h"
#include "CommonUtilities/AXP_Blocks.h"

/*
 * _GetDiskInfo
 *  This function is called to get various pieces of information from the newly
 *  opened device using ioctl.
 *
 * Input Parameters:
 *  raw:
 *	A pointer to the raw device handle to be initialized according to
 *	information from the device.
 *
 * Output Parameters:
 *  raw:
 *	A pointer to the raw device handle with information from the device.
 *
 * Return Values:
 *  true:	Normal Successful Completion.
 *  false:	An error occurred.
 */
static bool _GetDiskInfo(AXP_RAW_Handle *raw)
{
    bool	retVal = true, done = false;
    int		ioctlCnt = 0;
    int		ro;

    /*
     * Let's get information about the device.
     */
    while ((retVal == true) && (done == false))
  switch(ioctlCnt++)
  {
      case 0:
    retVal = ioctl(raw->fd, BLKROGET, &ro) == 0;
    raw->readOnly = ro != 0;
    break;

      case 1:
    retVal = ioctl(raw->fd, BLKGETSIZE64, &raw->diskSize) == 0;
    break;

      case 2:
    retVal = ioctl(raw->fd, BLKBSZGET, &raw->blkSize) == 0;
    break;

      case 3:
    retVal = ioctl(raw->fd, BLKSSZGET, &raw->sectorSize) == 0;

      default:
    done = true;
    break;
  }

    /*
     * Calculate some geometry numbers.  This code is consistent with the
     * function recount_geometry in the alignment.c module of the UNIX fdisk
     * utility.  NOTE: It differs from the geometry calculation used in VHD
     * file format.
     */
    if (retVal == true)
    {
  u64	totalSectors;

  totalSectors = raw->diskSize / raw->sectorSize;
  raw->heads = 255;
  raw->sectors = 63;
  raw->cylinders = totalSectors / (raw->heads * raw->sectors);
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * _AXP_RAW_Open
 *  This function is called to open a RAW device or CD.
 *
 * Input Parameters:
 *  path:
 *	A pointer to a valid string that represents the path to the new RAW
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
 *  	RAW disk.
 *
 *  Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_FILE_NOT_FOUND:	File Not Found.
 *  AXP_VHD_READ_FAULT:		Failed to read information from the file.
 *  AXP_VHD_OUTOFMEMORY:	Insufficient memory to perform operation.
 */
u32 _AXP_RAW_Open(
    char *path,
    AXP_VHD_OPEN_FLAG flags,
    u32 deviceID,
    AXP_VHD_HANDLE *handle)
{
    AXP_RAW_Handle	*raw;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * Let's allocate the block we need to maintain access to the physical disk
     * image.
     */
    raw = (AXP_RAW_Handle *) AXP_Allocate_Block(AXP_RAW_BLK);
    if (raw != NULL)
    {

  /*
   * Allocate a buffer long enough for for the filename (plus null
   * character).
   */
  raw->filePath = AXP_Allocate_Block(-(strlen(path) + 1));
  if (raw->filePath != NULL)
  {
      int	mode = (deviceID == STORAGE_TYPE_DEV_ISO) ? O_RDONLY : O_RDWR;

      strcpy(raw->filePath, path);
      raw->deviceID = deviceID;

      /*
       * Open the device/file.  If it is an ISO file or a CDROM device,
       * then do some initialization and we are done.  If it is a
       * physical device, then we re-open the device for binary
       * read/write.
       *
       * TODO: Do we need to do something different for ISO files.
       */
      raw->fd = open(path, mode, 0);
      if (raw->fd >= 0)
      {
    raw->deviceID = deviceID;
    if (_GetDiskInfo(raw) == false)
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
     * OK, if we don't have a success at this point, and we allocated a VHD
     * handle, then deallocate the handle, since the VHD was not successfully
     * opened.
     */
    if ((retVal != AXP_VHD_SUCCESS) && (raw != NULL))
  AXP_Deallocate_Block(raw);

    /*
     * Return the outcome of this call back to the caller.
     */
    return(retVal);
}
