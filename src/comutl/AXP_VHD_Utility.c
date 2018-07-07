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
 *  This source file contains utility functions utilized by the rest of the VHD
 *  code.
 *
 * Revision History:
 *
 *  V01.000	07-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Blocks.h"
#include "AXP_Trace.h"
#include "AXP_VHD_Utility.h"
#include "AXP_virtdisk.h"
#include "AXP_VHDX.h"

/*
 * Let's define some local items.
 */
static AXP_VHDX_GUID AXP_WellKnownGUIDs[] =
{

    /*
     * Vendor = Microsoft (from Microsoft VHD Interface web site)
     */
    {
	.data1 = 0xec984aec,
	.data2 = 0xa0f9,
	.data3 = 0x47e9,
	.data4 = 0x901f71415a66345b
    },

    /*
     * Vendor = Unknown (from Microsoft VHD Interface web site)
     */
    {
	.data1 = 0,
	.data2 = 0,
	.data3 = 0,
	.data4 = 0
    },

    /*
     * Metadata = File Parameters (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0xcaa16737,
	.data2 = 0xfa36,
	.data3 = 0x4d43,
	.data4 = 0xb3b633f0aa44e76b
    },

    /*
     * Metadata = Virtual DiskSize (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0x2fa54224,
	.data2 = 0xcd1b,
	.data3 = 0x4876,
	.data4 = 0xb2115dbed83bf4b8
    },

    /*
     * Metadata = Page 83 Data (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0xbeca12ab,
	.data2 = 0xb2e6,
	.data3 = 0x4523,
	.data4 = 0x93efc309e000c746
    },

    /*
     * Metadata = Logical Sector Size (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0x8141bf1d,
	.data2 = 0xa96f,
	.data3 = 0x4709,
	.data4 = 0xba47f233a8faab5f
    },

    /*
     * Metadata = Physical Sector Size (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0xcda348c7,
	.data2 = 0x445d,
	.data3 = 0x4471,
	.data4 = 0x9cc9e9885251c556
    },

    /*
     * Metadata = Parent Locator (from VHDX Functional Spec - Page 31)
     */
    {
	.data1 = 0xa8d35f2d,
	.data2 = 0xb30b,
	.data3 = 0x454d,
	.data4 = 0xabf7d3d84834ab0c
    },

    /*
     * Region = BAT (from VHDX Functional Spec - Page 18)
     */
    {
	.data1 = 0x2dc27766,
	.data2 = 0xf623,
	.data3 = 0x4200,
	.data4 = 0x9d64115e9Bfd4a08
    },

    /*
     * Region = Metadata (from VHDX Functional Spec - Page 18)
     */
    {
	.data1 = 0x8b7ca206,
	.data2 = 0x4790,
	.data3 = 0x4b9a,
	.data4 = 0xb8fe575f050f886e
    }
};

/*
 * AXP_VHD_CopyGUID
 *  This function is called to copy a GUID from a source to a destination.
 *
 * Input Parameters:
 *  src:
 *	A pointer to the source GUID.
 *
 * Output Parameters:
 *  dest:
 *	A pointer to the destination GUID.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_CopyGUID(AXP_VHDX_GUID *dest, AXP_VHDX_GUID *src)
{
    dest->data1 = src->data1;
    dest->data2 = src->data2;
    dest->data3 = src->data3;
    dest->data4 = src->data4;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_VHD_CompareGUID
 *  This function is called to compare 2 GUIDs for being equal.  This function
 *  does not care about greater than or less than.
 *
 * Input Parameters:
 *  guid1:
 *  	A pointer to the first GUID in the comparison.  NOTE: Order does not
 *  	matter.
 *  guid2:
 *  	A pointer to the second GUID in the comparison.  NOTE: Order does not
 *  	matter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The two GUIDs are equal.
 *  false:	The two GUIDs are not equal.
 */
bool AXP_VHD_CompareGUID(AXP_VHDX_GUID *guid1, AXP_VHDX_GUID *guid2)
{

    /*
     * Return the results of the comparison back to the caller.
     */
    return(((guid1->data1 == guid2->data1) &&
	    (guid1->data2 == guid2->data2) &&
	    (guid1->data3 == guid2->data3) &&
	    (guid1->data4 == guid2->data4)));
}

/*
 * AXP_VHD_KnownGUID
 *  This function is called to determine if the supplied GUID is one of the
 *  known ones and which one it is.  If it is not a known one, then
 *  AXP_Known_MAX will be returned.
 *
 * Input Parameters:
 *  guid:
 *	A pointer to the GUID to be compared to the known ones.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  AXP_Vendor_Microsoft:	The GUID associated with Microsoft as a vendor.
 *  AXP_Vendor_Unknown:		The GUID associated with an Unknown vendor.
 *				(This is the same as AXP_Zero_GUID).
 *  AXP_File_Parameter:		The GUID associated with File Parameters
 *				Metadata.
 *  AXP_Disk_Size:		The GUID associated with Virtual Disk Size
 *				Metadata.
 *  AXP_Page_83:		The GUID associated with Page 83 Data Metadata.
 *  AXP_Logical_Sector:		The GUID associated with Logical Sector Size
 *				Metadata.
 *  AXP_Physical_Sector:	The GUID associated with Physical Sector Size
 *				Metadata.
 *  AXP_Parent_Locator:		The GUID associated with Parent Locator
 *				Metadata.
 *  AXP_Block_Allocation_Table:	The GUID associated with BAT Region
 *  AXP_Metadata_Region:	The GUID associated with Metadata Region
 *  AXP_Known_MAX:		Not a well known GUID.
 */
AXP_VHD_KnownGUIDs AXP_VHD_KnownGUID(AXP_VHDX_GUID *guid)
{
    AXP_VHD_KnownGUIDs	retVal;

    /*
     * Loop through all the possibilities and if we find a match, return it.
     * The loop exits when either a match was found or we reach the end of the
     * Well Known GUIDs.
     */
    for (retVal = AXP_Vendor_Microsoft; retVal < AXP_Known_MAX; retVal++)
	if (AXP_VHD_CompareGUID(guid, &AXP_WellKnownGUIDs[retVal]))
	    break;

    /*
     * Return what we determined back to the caller.
     */
    return(retVal);
}

/*
 * AXP_VHD_SetGUIDMemory
 *  This function is called to set a GUID to a generated value for use in
 *  memory.  This may or may not be the same as what is written out to disk.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  guid:
 *	A pointer to a GUID to receive the generated value.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_SetGUIDMemory(AXP_VHDX_GUID *guid)
{
    uuid_generate(guid->uuid);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_VHD_SetGUIDDisk
 *  This function is called to set a  GUID to a generated value for being
 *  written out to disk.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  guid:
 *	A pointer to a GUID to receive the generated value.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_SetGUIDDisk(AXP_VHDX_GUID *guid)
{

    /*
     * First generate the GUID in Memory format.
     */
    AXP_VHD_SetGUIDMemory(guid);

    /*
     * Now, convert it to Disk format.
     */
    AXP_Convert_To(GUID, guid, guid);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_VHD_KnownGUIDMemory
 *  This function is called to set a GUID to a known value for use in
 *  memory.  This may or may not be the same as what is written out to disk.
 *
 * Input Parameters:
 *  known:
 *	A value indicating which of the known GUIDs should be used.
 *
 * Output Parameters:
 *  guid:
 *	A pointer to a GUID to receive the generated value.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_KnownGUIDMemory(AXP_VHD_KnownGUIDs known, AXP_VHDX_GUID *guid)
{

    /*
     * As long as known is one of the actual known ones, then copy the known
     * GUID to the caller's return parameter.
     */
    if (known < AXP_Known_MAX)
	AXP_VHD_CopyGUID(guid, &AXP_WellKnownGUIDs[known]);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_VHD_KnownGUIDDisk
 *  This function is called to set a GUID to a known value for use in
 *  memory.  This may or may not be the same as what is written out to disk.
 *
 * Input Parameters:
 *  known:
 *	A value indicating which of the known GUIDs should be used.
 *
 * Output Parameters:
 *  guid:
 *	A pointer to a GUID to receive the generated value.
 *
 * Return Values:
 *  None.
 */
void AXP_VHD_KnownGUIDDisk(AXP_VHD_KnownGUIDs known, AXP_VHDX_GUID *guid)
{

    /*
     * First generate the GUID in Memory format.
     */
    AXP_VHD_KnownGUIDMemory(known, guid);

    /*
     * Now, convert it to Disk format.
     */
    AXP_Convert_To(GUID, guid, guid);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_VHD_PerformFileSize
 *  For VHDX files, they need to be sized in multiples of 1M.  This function is
 *  called to get the current size of the file, then, if necessary, extend the
 *  file size so that it is a multiple of 1M.
 *
 * Input Parameters:
 *  fp:
 *	The pointer to the file.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  0:		An error occurred either getting the file size, or extending
 *		it.
 *  >0:		A value that must be a multiple of 1M.
 */
u64 AXP_VHD_PerformFileSize(FILE *fp)
{
    u64	retVal = (u64) AXP_GetFileSize(fp);
    u64	reqBytes = retVal & 0x00000000000fffffll;

    /*
     * If the file size is not a multiple of 1M, then write a byte at the
     * offset that will extend the file out appropriately.
     */
    if (retVal != (u64) -1)
    {
	if (reqBytes != 0)
	{
	    retVal = (retVal + ONE_M) & 0xfffffffffff00000ll;
	    if (AXP_WriteAtOffset(fp, " ", 1, retVal - 1) == false)
		retVal = 0;
	}
    }
    else
	retVal = 0;

    /*
     * Return the file size back to the caller.
     */
    return(retVal);
}

/*
 * AXP_VHD_ValidateCreate
 *  This function is called to validate the parameters for the AXP_VHD_Create.
 *  The output parameters are local, to the AXP_VHD_Create function, so that it
 *  can only deal with a known set of values.
 *
 * Input Parameters:
 *  storageType:
 *	A pointer to a structure containing the storage type information.
 *  path:
 *	A pointer to the string containing the full path to the file.
 *  accessMask:
 *	A value indicating the type of access being requested.  This is
 *	validated, but not necessarily utilized.
 *  flags:
 *	A value indicating the type of create being requested.
 *  param:
 *	A pointer to a structure containing the information supplied by the
 *	caller to specifics about the Virtual Hard Disk (VHD) being created.
 *  handle:
 *	A pointer to the VHD handle
 *
 * Output Parameters:
 *  diskSize:
 *	A pointer to a 64-bit unsigned integer to receive the total number of
 *	bytes that this VHD should represent.  This is as big as the VHD can
 *	grow (unless resized).
 *  blkSize:
 *	A pointer to a 32-bit unsigned integer to receive the block size being
 *	requested.  This may be defaulted.
 *  sectorSize:
 *	A pointer to a 32-bit unsigned integer to receive the sector size being
 *	requested.  This may be defaulted.
 *  deviceID:
 *	A pointer to a 32-bit unsigned integer to receive the type of device
 *	being created.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_PARAM:		An invalid parameter or combination of
 *				parameters was detected.
 */
u32 AXP_VHD_ValidateCreate(
		AXP_VHD_STORAGE_TYPE *storageType,
		char *path,
		AXP_VHD_ACCESS_MASK accessMask,
		AXP_VHD_CREATE_FLAG flags,
		AXP_VHD_CREATE_PARAM *param,
		AXP_VHD_HANDLE *handle,
		u64 *diskSize,
		u32 *blkSize,
		u32 *sectorSize,
		u32 *deviceID)
{
    u64			minDisk, maxDisk;
    u32			defBlk, minBlk, maxBlk;
    u32			defSector, minSector, maxSector;
    u32			retVal = AXP_VHD_SUCCESS;
    AXP_VHD_KnownGUIDs	vendorGuid;

    /*
     * We really should have all the following parameters supplied on the call.
     * We assume the return parameters are present, since this function is only
     * ever called from within the tool.
     */
    if ((storageType != NULL) &&
	(param != NULL) &&
	(handle != NULL) &&
	(path != NULL))
    {
	vendorGuid = AXP_VHD_KnownGUID(&storageType->vendorID);
	if ((vendorGuid == AXP_Vendor_Microsoft) ||
	    (vendorGuid == AXP_Vendor_Unknown))
	{
	    *deviceID = storageType->deviceID;
	    switch(storageType->deviceID)
	    {
		case STORAGE_TYPE_DEV_ISO:
		    minDisk = 0;
		    maxDisk = 0;
		    minBlk = 0;
		    defBlk = AXP_ISO_BLK_DEF;
		    maxBlk = 0;
		    minSector = 0;
		    defSector = AXP_ISO_SEC_DEF;
		    maxSector = 0;
		    break;

		case STORAGE_TYPE_DEV_VHD:
		    minDisk = 3 * ONE_M;
		    maxDisk = 2040 * ONE_M;
		    minBlk = AXP_VHD_BLK_MIN;
		    defBlk = AXP_VHD_BLK_DEF;
		    maxBlk = AXP_VHD_BLK_MAX;
		    minSector = AXP_VHD_SEC_MIN;
		    defSector = AXP_VHD_SEC_DEF;
		    maxSector = AXP_VHD_SEC_MAX;
		    break;

		case STORAGE_TYPE_DEV_VHDX:
		    minDisk = 3 * ONE_M;
		    maxDisk = 6 * ONE_T;
		    minBlk = AXP_VHDX_BLK_MIN;
		    defBlk = AXP_VHDX_BLK_DEF;
		    maxBlk = AXP_VHDX_BLK_MAX;
		    minSector = AXP_VHDX_SEC_MIN;
		    defSector = AXP_VHDX_SEC_DEF;
		    maxSector = AXP_VHDX_SEC_MAX;
		    break;

		default:
		    break;
	    }

	    switch (param->ver)
	    {
		case CREATE_VER_UNSPEC:
		    break;

		case CREATE_VER_1:
		    *blkSize = (param->ver_1.blkSize == AXP_VHD_DEF_BLK) ?
				defBlk :
				param->ver_1.blkSize;
		    *sectorSize = (param->ver_1.sectorSize == AXP_VHD_DEF_SEC) ?
				defSector : param->ver_1.sectorSize;
		    *diskSize = param->ver_1.maxSize;
		    break;

		case CREATE_VER_2:
		    *blkSize = (param->ver_2.blkSize == AXP_VHD_DEF_BLK) ?
				defBlk :
				param->ver_2.blkSize;
		    *sectorSize = (param->ver_2.sectorSize == AXP_VHD_DEF_SEC) ?
				defSector : param->ver_2.sectorSize;
		    *diskSize = param->ver_2.maxSize;
		    break;

		case CREATE_VER_3:
		    *blkSize = (param->ver_3.blkSize == AXP_VHD_DEF_BLK) ?
				defBlk :
				param->ver_3.blkSize;
		    *sectorSize = (param->ver_3.sectorSize == AXP_VHD_DEF_SEC) ?
				defSector : param->ver_3.sectorSize;
		    *diskSize = param->ver_3.maxSize;
		    break;

		case CREATE_VER_4:
		    *blkSize = (param->ver_4.blkSize == AXP_VHD_DEF_BLK) ?
				defBlk :
				param->ver_4.blkSize;
		    *sectorSize = (param->ver_4.sectorSize == AXP_VHD_DEF_SEC) ?
				defSector : param->ver_4.sectorSize;
		    *diskSize = param->ver_4.maxSize;
		    break;
	    }

	    /*
	     * Finally, let's check the values supplied in various parameters.
	     *
	     *	1) Only Version 1 and Version 2 are supported at this time.
	     *	2) If Version 2, then the Access Mask must be NONE.
	     *	3) Block Size needs to be between the minimum and maximum, and
	     *	   be a power of 2.
	     *	4) Sector Size must be either the minimum or maximum (but not
	     *	   in between).
	     *	5) Disk Size needs to be between the minimum and maximum
	     *	   allowable sized and  be a multiple of Sector Size.
	     */
	    if (((param->ver != CREATE_VER_1) && (param->ver != CREATE_VER_2)) ||
		((param->ver == CREATE_VER_2) && (accessMask != ACCESS_NONE)) ||
		(flags > CREATE_FULL_PHYSICAL_ALLOCATION) ||
		(((*blkSize <= minBlk) || (*blkSize >= maxBlk)) ||
		 (IS_POWER_OF_2(*blkSize) == false)) ||
		((*sectorSize != minSector) && (*sectorSize != maxSector)) ||
		((*diskSize < minDisk) || (*diskSize > maxDisk) ||
		 ((*diskSize % *sectorSize) != 0)))
		retVal = AXP_VHD_INV_PARAM;
	}
	else
	    retVal = AXP_VHD_INV_PARAM;
    }
    else
	retVal = AXP_VHD_INV_PARAM;

    /*
     * Return the parameter checking results back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Dump_VHD_Info
 *  This function is called to read in various aspects of a VHD file and log
 *  information about it to the tracing code.
 *
 * Input Parameters:
 *  handle:
 *	A valid handle to an open object.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void AXP_Dump_VHD_Info(AXP_VHD_HANDLE handle)
{
    AXP_VHDX_Handle	*vhdx = (AXP_VHDX_Handle *) handle;
    u8			buffer[SIXTYFOUR_K];
    u64			offset;
    u64			logOffset, batOffset = 0, metadataOffset = 0;
    size_t		retLen = SIXTYFOUR_K;
    int			ii = 0;
    u16			*data4a;
    bool		readRet;

    /*
     * We only trace if logging is turned on.
     */
    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();

	/*
	 * Before we go too far, let's make sure that we have a valid handle
	 * with data we need from within.
	 */
	if ((handle != NULL) &&
	    (vhdx->header.type == AXP_VHDX_BLK) &&
	    (vhdx->header.size == sizeof(AXP_VHDX_Handle)))
	{
	    if (vhdx->fp != NULL)
	    {

		/*
		 * OK, we have a valid handle and file pointer.  Let's go do
		 * some file reading and processing.  NOTE: We are not going to
		 * use the VHD routines for this.  This is so we can
		 * independently verify a VHD file.  By the way, this code can
		 * potentially dump any file.
		 */
		AXP_TraceWrite("Dumping VHD File Information:");

		/*
		 * First the header section, which is comprised of 5 64K
		 * blocks.
		 *
		 * This is for the File Identifier.
		 */
		readRet = AXP_ReadFromOffset(
					vhdx->fp,
					buffer,
					&retLen,
					AXP_VHDX_HDR_LOC);
		if (readRet == true)
		{
		    AXP_VHDX_ID *id = (AXP_VHDX_ID *) buffer;
		    char creator[sizeof(id->creator)/sizeof(uint16_t)];
		    char *ptr = (char *) id->creator;

		    AXP_TraceWrite("\t3.1.1 File Type Identifier: @ 0x%016llx", AXP_VHDX_HDR_LOC);
		    AXP_TraceWrite("\t\tSignature: %.8s", (char *) &id->sig);
		    for (ii = 0;
			 ((ptr[ii] != '\0') && (ii < sizeof(creator)));
			 ii += 2)
			creator[ii/2] = ptr[ii];
		    creator[ii/2] = '\0';
		    AXP_TraceWrite("\t\tCreator: %s", creator);
		}
		if (readRet == true)
		{
		    for (ii = 0; ((ii < 2) && (readRet == true)); ii++)
		    {
			offset = (ii == 0) ?
				AXP_VHDX_HEADER1_OFF :
				AXP_VHDX_HEADER2_OFF;

			/*
			 * This is for the the File Header (actually stored twice).
			 */
			retLen = SIXTYFOUR_K;
			readRet = AXP_ReadFromOffset(
					    vhdx->fp,
					    buffer,
					    &retLen,
					    offset);
			if (readRet == true)
			{
			    AXP_VHDX_HDR *hdr = (AXP_VHDX_HDR *) buffer;

			    AXP_TraceWrite("\t3.1.2 Headers %d: @ 0x%016llx", ii+1, offset);
			    AXP_TraceWrite(
				"\t\tSignature: %.4s",
				(char *) &hdr->sig);
			    AXP_TraceWrite("\t\tChecksum: 0x%08x", hdr->checkSum);
			    AXP_TraceWrite(
				"\t\tSequenceNumber: 0x%016x (%llu)",
				hdr->seqNum,
				hdr->seqNum);
			    AXP_Convert_From(
				GUID,
				&hdr->fileWriteGuid,
				&hdr->fileWriteGuid);
			    data4a = (u16 *) &hdr->fileWriteGuid.data4;
			    AXP_TraceWrite(
				"\t\tFileWriteGuid: %08x-%04x-%04x-%04x-%04x%04x%04x",
				hdr->fileWriteGuid.data1,
				hdr->fileWriteGuid.data2,
				hdr->fileWriteGuid.data3,
				data4a[3],
				data4a[2],
				data4a[1],
				data4a[0]);
			    AXP_Convert_From(
				GUID,
				&hdr->dataWriteGuid,
				&hdr->dataWriteGuid);
			    data4a = (u16 *) &hdr->dataWriteGuid.data4;
			    AXP_TraceWrite(
				"\t\tDataWriteGuid: %08x-%04x-%04x-%04x-%04x%04x%04x",
				hdr->dataWriteGuid.data1,
				hdr->dataWriteGuid.data2,
				hdr->dataWriteGuid.data3,
				data4a[3],
				data4a[2],
				data4a[1],
				data4a[0]);
			    AXP_Convert_From(
				GUID,
				&hdr->logGuid,
				&hdr->logGuid);
			    data4a = (u16 *) &hdr->logGuid.data4;
			    AXP_TraceWrite(
				"\t\tLogGuid: %08x-%04x-%04x-%04x-%04x%04x%04x",
				hdr->logGuid.data1,
				hdr->logGuid.data2,
				hdr->logGuid.data3,
				data4a[3],
				data4a[2],
				data4a[1],
				data4a[0]);
			    AXP_TraceWrite("\t\tLogVersion %u", hdr->logVer);
			    AXP_TraceWrite("\t\tVersion: %u", hdr->ver);
			    AXP_TraceWrite("\t\tLogLength: %u", hdr->logLen);
			    logOffset = hdr->logOff;
			    AXP_TraceWrite(
				"\t\tLogOffset: 0x%016llx (%llu)",
				hdr->logOff,
				logOffset);
			}
		    }
		}
		if (readRet == true)
		{
		    int jj, nextEntry;

		    for (ii = 0; ((ii < 2) && (readRet == true)); ii++)
		    {
			nextEntry = AXP_VHDX_REG_HDR_LEN;
			offset = (ii == 0) ?
				AXP_VHDX_REG_TBL_HDR1_OFF :
				AXP_VHDX_REG_TBL_HDR2_OFF;

			/*
			 * This is for the the File Header (actually stored twice).
			 */
			retLen = SIXTYFOUR_K;
			readRet = AXP_ReadFromOffset(
					    vhdx->fp,
					    buffer,
					    &retLen,
					    offset);
			if (readRet == true)
			{
			    AXP_VHDX_REG_HDR *reg = (AXP_VHDX_REG_HDR *) buffer;

			    AXP_TraceWrite("\t3.1.3 Region Table %d: @ 0x%016llx", ii+1, offset);
			    AXP_TraceWrite(
				"\t\tSignature: %.4s",
				(char *) &reg->sig);
			    AXP_TraceWrite("\t\tChecksum: 0x%08x", reg->checkSum);
			    AXP_TraceWrite(
				"\t\tEntryCount: %u",
				reg->entryCnt);
			    for (jj = 0;
				 ((jj < reg->entryCnt) && (readRet == true));
				 jj++)
			    {
				AXP_VHDX_REG_ENT *ent =
				    (AXP_VHDX_REG_ENT *) &buffer[nextEntry];
				u64 localOffset;

				AXP_TraceWrite("\t\tRegion Entry %d: @ 0x%016llx", jj+1, offset+nextEntry);
				nextEntry += AXP_VHDX_REG_ENT_LEN;
				AXP_Convert_From(
					GUID,
					&ent->guid,
					&ent->guid);
				switch (AXP_VHD_KnownGUID(&ent->guid))
				{
				    case AXP_Block_Allocation_Table:
					batOffset = ent->fileOff;
					break;

				    case AXP_Metadata_Region:
					metadataOffset = ent->fileOff;
					break;

				    default:
					break;
				}
				data4a = (u16 *) &ent->guid.data4;
				AXP_TraceWrite(
					"\t\t\tGuid: %08x-%04x-%04x-%04x-%04x%04x%04x",
					ent->guid.data1,
					ent->guid.data2,
					ent->guid.data3,
					data4a[3],
					data4a[2],
					data4a[1],
					data4a[0]);
				localOffset = (batOffset == 0) ?
					metadataOffset :
					batOffset;
				AXP_TraceWrite(
					"\t\t\tOffset: %016llx (%llu)",
					ent->fileOff,
					localOffset);
				AXP_TraceWrite("\t\t\tLength: %u", ent->len);
				AXP_TraceWrite("\t\t\tRequired: %u", ent->req);
			    }
			}
		    }
		}
		if (readRet == false)
		    AXP_TraceWrite(">>>>> AXP_ReadFromOffset failed. <<<<<");
	    }
	    else
		AXP_TraceWrite(
			">>>>> No file pointer found in VHD Handle used"
			" to trace VHD File. <<<<<");
	}
	else
	    AXP_TraceWrite(">>>>> Invalid VHD Handle used to trace VHD File. <<<<<");
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}
