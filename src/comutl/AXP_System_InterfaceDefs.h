/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This header file contains the definitions needed for the Digital AXP 21264
 *	CPU to be able to interface with the System.
 *
 *	Revision History:
 *
 *	V01.000		31-Dec-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_SYSTEM_INTERFACE_DEFS_
#define _AXP_SYSTEM_INTERFACE_DEFS_	1

/*
 * HRM Table 4-5
 * The enumeration contains the System Data Control (SysDc) Responses to 21264
 * Commands.
 */
typedef enum
{
	NOPsysdc = 0x00,
	ReadDataError,
	ChangeToDirtySuccess = 0x04,
	ChangeToDirtyFail,
	MBDone,
	ReleaseBuffer,
	WriteData0,
	WriteData1,
	WriteData2,
	WriteData3,
	WriteData = WriteData0,
	ReadData0 = 0x10,
	ReadData = ReadData0,
	ReadData1,
	ReadData2,
	ReadData3,
	ReadDataDirty0,
	ReadDataDirty = ReadDataDirty0,
	ReadDataDirty1,
	ReadDataDirty2,
	ReadDataDirty3,
	ReadDataShared0,
	ReadDataShared = ReadDataShared0,
	ReadDataShared1,
	ReadDataShared2,
	ReadDataShared3,
	ReadDataSharedDirty0,
	ReadDataSharedDirty = ReadDataSharedDirty0,
	ReadDataSharedDirty1,
	ReadDataSharedDirty2,
	ReadDataSharedDirty3
} AXP_21264_SYSDC_RSP;

/*
 * HRM Table 4-14
 * The enumeration contains the 21264-to-System Commands.
 */
typedef enum
{
	NOPcmd = 0x00,
	ProbeResponse,
	NZNOP,
	VDBFlushRequest,
	WrVictimBlk,
	CleanVictimBlk,
	Evict,
	MB,
	ReadBytes,
	ReadLWs,
	ReadQWs,
	ReadWs,
	WrBytes = 0x0C,
	WrLWs,
	WrQWs,
	ReadBlk = 0x10,
	ReadBlkMod,
	ReadBlkI,
	FetchBlk,
	ReadBlkSpec,
	ReadBlkModSpec,
	ReadBlkSpecI,
	FetchBlkSpec,
	ReadBlkVic,
	ReadBlkModVic,
	ReadBlkVicI,
	InvalToDirtyVic,
	CleanToDirty,
	SharedToDirty,
	STCChangeToDirty,
	InvalToDirty
} AXP_21264_TO_SYS_CMD;

/*
 * HRM Table 4-18
 * Result of Probe
 */
typedef enum
{
	HitClean = 0,
	HitShared,
	HitDirty,
	HitSharedDirty
} AXP_21264_PROBE_STAT;

#endif	/* AXP_SYSTEM_INTERFACE_DEFS_ */
