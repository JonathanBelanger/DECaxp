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
 *	This header file contains the function definitions implemented in the
 *	AXP_21264_Cbox.c module.
 *
 * Revision History:
 *
 *	V01.000		14-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		12-Oct-2017	Jonathan D. Belanger
 *	Change the CTAG structure to contain the DTAG index for the corresponding
 *	entry.  This should make keeping these 2 arrays, along with the Dcache
 *	itself, in synch.  NOTE: The set information is the same for all.
 *
 *	V01.002		14-Oct-2017	Jonathan D. Belanger
 *	Started defining the CPU to System and System to CPU messages.
 *
 *	V01.003		22-Oct-2017	Jonathan D. Belanger
 *	Differentiated between messages that come over the SysAddIn_L[14:0] pins
 *	and SysAddOut_L[14:0] pins.  Also defined a number of the remaining
 *	structures needed by the Cbox.
 *
 *	V01.004		06-Nov-2017	Jonathan D. Belanger
 *	Split this fine into 2 separate files.  One to include the definitions for
 *	the Cbox CSRs and one to be included by AXP_21264_Cbox.c so that it will
 *	compile cleanly.  This file is the one with the CSRs define within it.
 */
#ifndef _AXP_21264_CBOX_DEFS_
#define _AXP_21264_CBOX_DEFS_

#include "AXP_Utility.h"
#include "AXP_Configure.h"

/*
 * This structure is the definition for one Cbox copy of Dcache Tag Array.  A
 * block contains the following:
 *
 *		Virtual tag bits
 *		Index into corresponding DTAG array (which is the same as the Dcache)
 *		Valid bit
 *			The CTAG array entry is in use.
 */
typedef struct
{
	u64					virtTag;
	u32					dtagIndex;
	bool				valid;
} AXP_21264_CBOX_CTAG;

/*
 * HRM Tables 5-23 and 5-24
 * CBox CSRs
 */
typedef struct
{
		u64 BcBankEnable : 1;
		u64 BcBurstModeEnable : 1;
		u64 BcCleanVictim : 1;
		u64 BcClkfwdEnable : 1;
		u64 BcClockOut : 1;
		u64 BcDdmFallEn : 1;
		u64 BcDdmfEnable : 1;
		u64 BcDdmrEnable : 1;
		u64 BcDdmRiseEn : 1;
		u64 BcEnable : 1;
		u64 BcFrmClk : 1;
		u64 BcLateWriteUpper : 1;
		u64 BcPentiumMode : 1;
		u64 BcRdRdBubble : 1;
		u64 BcRdvictim : 1;
		u64 BcSjBankEnable : 1;
		u64 BcTagDdmFallEn : 1;
		u64 BcTagDdmRiseEn : 1;
		u64 BcWrWrBubble : 1;
		u64 ThirtyTwoByteIo : 1;
		u64 DupTagEnable : 1;
		u64 EnableEvict : 1;
		u64 EnableProbeCheck : 1;
		u64 EnableStcCommand : 1;
		u64 FastModeDisable : 1;
		u64 InitMode : 1;
		u64 JitterCmd : 1;
		u64 MboxBcPrbStall : 1;
		u64 PrbTagOnly : 1;
		u64 RdvicAckInhibit : 1;
		u64 SkewedFillMode : 1;
		u64 SpecReadEnable : 1;
		u64 StcEnable : 1;
		u64 SysbusFormat : 1;
		u64 SysbusMbEnable : 1;
		u64 SysClkfwdEnable : 1;
		u64 SysDdmFallEn : 1;
		u64 SysDdmfEnable : 1;
		u64 SysDdmrEnable : 1;
		u64 SysDdmRdFallEn : 1;
		u64 SysDdmRdRiseEn : 1;
		u64 SysDdmRiseEn : 1;
		u64 BcClkDelay : 2;
		u64 BcCpuClkDelay : 2;
		u64 BcCpuLateWriteNum : 2;
		u64 BcRcvMuxCntPreset : 2;
		u64 CfrFrmclkDelay : 2;
		u64 DataValidDly : 2;
		u64 InvalToDirty : 2;
		u64 InvalToDirtyEnable : 2;
		u64 SysBusSize : 2;
		u64 SysClkDelay : 2;
		u64 res_1 : 2;				/* Quadword align */
		u64 SysCpuClkDelay : 2;
		u64 SysRcvMuxCntPreset : 2;
		u64 SysRcvMuxPreset : 2;
		u64 BcLateWriteNum : 3;
		u64 CfrEv6clkDelay : 3;
		u64 SetDirtyEnable : 3;
		u64 SysbusVicLimit : 3;
		u64 BcBphaseLdVector : 4;
		u64 BcSize : 4;
		u64 BcWrRdBubbles : 4;
		u64 BcWrtSts : 4;
		u64 CfrGclkDelay : 4;
		u64 MbCnt : 4;
		u64 SysBphaseLdVector : 4;
		u64 SysdcDelay : 4;
		u64 SysbusAckLimit : 5;
		u64 SysClkRatio : 5;
		u64 res_2 : 4;				/* Quadword align */
		u32 SysFrameLdVector : 5;
		u32 BcRdWrBubbles : 6;
		u32 res_3 : 21;				/* Longword Align */
		u32 BcLatTagPattern : 24;
		u32 res_4 : 8;				/* Quadword align */
		u8 BcFdbkEn;
		u8 DcvicThreshold ;
		u8 SysFdbkEn;
		u8 res_5;					/* Longword align */
		u16 BcClkLdVector;
		u16 SysClkLdVector;
		u32 BcLatDataPattern;
		u32 res_6;					/* Quadword align */
} AXP_21264_CBOX_CSRS;

typedef enum
{
	BcBankEnable,
	BcBurstModeEnable,
	BcCleanVictim,
	BcClkfwdEnable,
	BcClockOut,
	BcDdmFallEn,
	BcDdmfEnable,
	BcDdmrEnable,
	BcDdmRiseEn,
	BcEnable,
	BcFrmClk,
	BcLateWriteUpper,
	BcPentiumMode,
	BcRdRdBubble,
	BcRdvictim,
	BcSjBankEnable,
	BcTagDdmFallEn,
	BcTagDdmRiseEn,
	BcWrWrBubble,
	ThirtyTwoByteIo,
	DupTagEnable,
	EnableEvict,
	EnableProbeCheck,
	EnableStcCommand,
	FastModeDisable,
	InitMode,
	JitterCmd,
	MboxBcPrbStall,
	PrbTagOnly,
	RdvicAckInhibit,
	SkewedFillMode,
	SpecReadEnable,
	StcEnable,
	SysbusFormat,
	SysbusMbEnable,
	SysClkfwdEnable,
	SysDdmFallEn,
	SysDdmfEnable,
	SysDdmrEnable,
	SysDdmRdFallEn,
	SysDdmRdRiseEn,
	SysDdmRiseEn,
	BcClkDelay,
	BcCpuClkDelay,
	BcCpuLateWriteNum,
	BcRcvMuxCntPreset,
	CfrFrmclkDelay,
	DataValidDly,
	InvalToDirty1,
	InvalToDirtyEnable,
	SysBusSize,
	SysClkDelay,
	SysCpuClkDelay,
	SysRcvMuxCntPreset,
	SysRcvMuxPreset,
	BcLateWriteNum,
	CfrEv6clkDelay,
	SetDirtyEnable,
	SysbusVicLimit,
	BcBphaseLdVector,
	BcSize,
	BcWrRdBubbles,
	BcWrtSts,
	CfrGclkDelay,
	MbCnt,
	SysBphaseLdVector,
	SysdcDelay,
	SysbusAckLimit,
	SysClkRatio,
	SysFrameLdVector,
	BcRdWrBubbles,
	BcLatTagPattern,
	BcFdbkEn,
	DcvicThreshold,
	SysFdbkEn,
	BcClkLdVector,
	SysClkLdVector,
	BcLatDataPattern,
	LastCSR
} AXP_21264_CBOX_CSR_VALUES;

typedef struct
{
	const char 						*name;
	const AXP_21264_CBOX_CSR_VALUES	values;
} AXP_21264_CBOX_CSR_NAMES;
/*
 * HRM Table 4-15
 * INVALID_TO_DIRTY_ENABLE values.
 */
#define AXP_WH64_CVT_RDMODX00		0	/* b'x0' */
#define AXP_WH64_CVT_RDMODX10		2	/* b'x0' */
#define AXP_WH64_ENA_INT_ACK		1	/* b'01' */
#define AXP_WH64_ENA_INT_INV2DIR	3	/* b'11' */

/*
 * HRM Table 4-16
 * SET_DIRTY_ENABLE values.
 *
 * This is a mask with the following meaning (when set):
 * 		bit 0:	Only Clean Blocks generate external ACK - CleanToDirty
 * 		bit 1:	Only Clean/Shared Blocks generate external ACK - SharedToDirty
 * 		bit 2:	Only Dirty/Shared Blocks generate external ACK - SharedToDirty
 */
#define AXP_ACK_INTERNALLY			0	/* b'000' Uniprocessor */
#define AXP_CLEAN_ACK_ONLY			1	/* b'001' CleanToDirty */
#define AXP_CLEAN_SHARED_ACK_ONLY	2	/* b'010' SharedToDirty */
#define AXP_DIRTY_SHARED_ACK_ONLY	4	/* b'100' SharedToDirty */

/*
 * HRM Table 4-42
 * BC_SIZE value when BC_ENABLE is set.
 *
 * An easy calculation is the value of BC_SIZE + 1 in MB.
 */
#define	AXP_BCACHE_1MB				0
#define	AXP_BCACHE_2MB				1
#define	AXP_BCACHE_4MB				3
#define	AXP_BCACHE_8MB				7
#define	AXP_BCACHE_16MB				15

/*
 * HRM 2.12
 * Each IOWB entry has the following:
 * 		64 bytes of data
 * 		physical address
 * 		control logic	(TBD)
 */
#define AXP_21264_IOWB_SIZE		64
typedef struct
{
	u64					pa;
	u8					data[AXP_21264_IOWB_SIZE];
} AXP_21264_CBOX_IOWB;

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
	ReadData0 = 0x10,
	ReadData1,
	ReadData2,
	ReadData3,
	ReadDataDirty0,
	ReadDataDirty1,
	ReadDataDirty2,
	ReadDataDirty3,
	ReadDataShared0,
	ReadDataShared1,
	ReadDataShared2,
	ReadDataShared3,
	ReadDataSharedDirty0,
	ReadDataSharedDirty1,
	ReadDataSharedDirty2,
	ReadDataSharedDirty3
} AXP_21264_SYSADD_IN_SYSDC;

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
} AXP_21264_SYSADD_OUT_CMD;

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
} AXP_21264_SYSADD_OUT_PROBE_STAT;

/*
 * HRM Section 4.7.7
 * There are 2 commands to be sent from the System to the CPU.  These are
 * they.
 */
#define AXP_SYSTEM_DATA_TRANSFER	false
#define AXP_SYSTEM_PROBE			true

/*
 * HRM Tables 4-21 and 4-22
 * Probe Command
 */
typedef enum
{
	NopNop = 0x00,
	NopClean,
	NopCleanShared,
	NopTransistion3,
	NopDirtyShared,
	NopInvalid,
	NopTransistion1,
	NopReserved,
	ReadHitNop,
	ReadHitClean,
	ReadHitCleanShared,
	ReadHitTransistion3,
	ReadHitDirtyShared,
	ReadHitInvalid,
	ReadHitTransistion1,
	ReadHitReserved,
	ReadDirtyNop,
	ReadDirtyClean,
	ReadDirtyCleanShared,
	ReadDirtyTransistion3,
	ReadDirtyDirtyShared,
	ReadDirtyInvalid,
	ReadDirtyTransistion1,
	ReadDirtyReserved,
	ReadAnywayNop,
	ReadAnywayClean,
	ReadAnywayCleanShared,
	ReadAnywayTransistion3,
	ReadAnywayDirtyShared,
	ReadAnywayInvalid,
	ReadAnywayTransistion1,
	ReadAnywayReserved
} AXP_21264_SYSADD_IN_PROBE_CMD;

/*
 * HRM 4.7.3 and 4.7.5
 * This structure is used for commands and ProbeResponses to be sent to the
 * System from the 21264.
 *
 * NOTE: Because we are emulating these communications and not sending a series
 * 		 of 15 bits in 4 cycles, we do not need to differentiate between "Bank
 * 		 Interleave" and "Page Hit" modes.
 *
 * This is for commands from the Alpha AXP 21264 CPU to the System.
 */
typedef struct
{
	AXP_21264_SYSADD_OUT_CMD		Command;
	AXP_21264_SYSADD_OUT_PROBE_STAT	Status;
	bool							Miss1;				/* M1 */
	bool							Miss2;				/* M2 */
	bool							CacheHit;			/* CH */
	bool							ReadValidate;		/* RV */
	bool							DataMovement;		/* DM */
	bool							VictimSent;			/* VS */
	bool							MAFAddressSent;		/* MS */
	u8								MAF;
	u8								Mask;
	u8								ID;
	u8								SysData[sizeof(u64)]; /* when DM=true */
	u64								pa;
} AXP_21264_SYSADD_OUT;

/*
 * HRM 4.7.7.1 and 4.7.7.2
 * This is for probes and Data Transfer Commands from the system to the Alpha
 * AXP 21264 CPU.
 */
typedef struct
{
	AXP_21264_SYSADD_IN_PROBE_CMD	Command;
	AXP_21264_SYSADD_IN_SYSDC		SysDc;
	bool							ProbeData;	/* probe=true; data=false; */
	bool							ResetVictimBuffer;	/* RVB */
	bool							ResetProbeValidBit;	/* RPB */
	bool							CommandAck;			/* A */
	bool							Commit;				/* C */
	u8								ID;
	u64								pa;
	u8								SysData[sizeof(u64)]; /* when ProbeData=false */
} AXP_21264_SYSADD_IN;

/*
 * HRM 2.1.4.1
 * This structure is the definition for one of the Victim Address File (VAF)
 * and Victim Data File (VDF).  Together these structures form an 8-entry
 * victim buffer used for holding:
 *
 * 		Dcache blocks to be written to the Bcache
 * 		Istream cache blocks from memory to the Bcache
 * 		Bcache blocks to be written to memory
 * 		Cache blocks sent to the system in response to probe commands
 *
 * NOTE: This current implementation does not have a Bcache.  Therefore, the
 * 		 writes to the Bcache are actually writes to memory.  What this means
 * 		 is that the two middle items above are not implemented, as they are
 * 		 redundant in with the lack of Bcache.
 */
typedef struct
{
	bool							validVictim;
	bool							validProbe;
	bool							inOut;		/* true = out; false = in */
	union
	{
		AXP_21264_SYSADD_IN			SysAddIn;
		AXP_21264_SYSADD_OUT		SysAddOut;
	};
} AXP_21264_CBOX_VIC_BUF;

/*
 * HRM 2.1.4.3
 * This structure is the definition for a single entry in the probe queue (PQ)
 */
typedef struct
{
	AXP_21264_SYSADD_IN_PROBE_CMD	Command;
	bool							marked;
	u64								pa;
} AXP_21264_CBOX_PQ;

#endif /* _AXP_21264_CBOX_DEFS_ */

