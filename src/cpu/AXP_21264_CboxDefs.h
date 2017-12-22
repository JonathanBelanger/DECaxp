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
 *
 *	V01.005		16-Nov-2017	Jonathan D. Belanger
 *	Added code to manage the Bcache.  We still need to figure out how to do
 *	the write-though to memory when updating a location in the Bcache.
 *
 *	V01.006		22-Dec-2017	Jonathan D. Belanger
 *	We need a merge length a store length in each IOWB.  The store length is
 *	used to determine how big the data was for the store that allocated the
 *	IOWB.  This is used to make sure we merge like sizes only.
 */
#ifndef _AXP_21264_CBOX_DEFS_
#define _AXP_21264_CBOX_DEFS_

#include "AXP_Utility.h"
#include "AXP_Configure.h"

/*
 * HRM Tables 5-23 and 5-24
 * CBox CSRs
 */
typedef struct
{
	u64						BcBankEnable : 1;
	u64						BcBurstModeEnable : 1;
	u64						BcCleanVictim : 1;
	u64						BcClkfwdEnable : 1;
	u64						BcClockOut : 1;
	u64						BcDdmFallEn : 1;
	u64						BcDdmfEnable : 1;
	u64						BcDdmrEnable : 1;
	u64						BcDdmRiseEn : 1;
	u64						BcEnable : 1;
	u64						BcFrmClk : 1;
	u64						BcLateWriteUpper : 1;
	u64						BcPentiumMode : 1;
	u64						BcRdRdBubble : 1;
	u64						BcRdvictim : 1;
	u64						BcSjBankEnable : 1;
	u64						BcTagDdmFallEn : 1;
	u64						BcTagDdmRiseEn : 1;
	u64						BcWrWrBubble : 1;
	u64						ThirtyTwoByteIo : 1;
	u64						DupTagEnable : 1;
	u64						EnableEvict : 1;
	u64						EnableProbeCheck : 1;
	u64						EnableStcCommand : 1;
	u64						FastModeDisable : 1;
	u64						InitMode : 1;
	u64						JitterCmd : 1;
	u64						MboxBcPrbStall : 1;
	u64						PrbTagOnly : 1;
	u64						RdvicAckInhibit : 1;
	u64						SkewedFillMode : 1;
	u64						SpecReadEnable : 1;
	u64						StcEnable : 1;
	u64						SysbusFormat : 1;
	u64						SysbusMbEnable : 1;
	u64						SysClkfwdEnable : 1;
	u64						SysDdmFallEn : 1;
	u64						SysDdmfEnable : 1;
	u64						SysDdmrEnable : 1;
	u64						SysDdmRdFallEn : 1;
	u64						SysDdmRdRiseEn : 1;
	u64						SysDdmRiseEn : 1;
	u64						BcClkDelay : 2;
	u64						BcCpuClkDelay : 2;
	u64						BcCpuLateWriteNum : 2;
	u64						BcRcvMuxCntPreset : 2;
	u64						CfrFrmclkDelay : 2;
	u64						DataValidDly : 2;
	u64						InvalToDirty : 2;
	u64						InvalToDirtyEnable : 2;
	u64						SysBusSize : 2;
	u64						SysClkDelay : 2;
	u64						res_1 : 2;				/* Quadword align */
	u64						SysCpuClkDelay : 2;
	u64						SysRcvMuxCntPreset : 2;
	u64						SysRcvMuxPreset : 2;
	u64						BcLateWriteNum : 3;
	u64						CfrEv6clkDelay : 3;
	u64						SetDirtyEnable : 3;
	u64						SysbusVicLimit : 3;
	u64						BcBphaseLdVector : 4;
	u64						BcSize : 4;
	u64						BcWrRdBubbles : 4;
	u64						BcWrtSts : 4;
	u64						CfrGclkDelay : 4;
	u64						MbCnt : 4;
	u64						SysBphaseLdVector : 4;
	u64						SysdcDelay : 4;
	u64						SysbusAckLimit : 5;
	u64						SysClkRatio : 5;
	u64						res_2 : 4;				/* Quadword align */
	u32						SysFrameLdVector : 5;
	u32						BcRdWrBubbles : 6;
	u32						res_3 : 21;				/* Longword Align */
	u32						BcLatTagPattern : 24;
	u32						res_4 : 8;				/* Quadword align */
	u8						BcFdbkEn;
	u8						DcvicThreshold ;
	u8						SysFdbkEn;
	u8						res_5;					/* Longword align */
	u16						BcClkLdVector;
	u16						SysClkLdVector;
	u32						BcLatDataPattern;
	u32						res_6;					/* Quadword align */
} AXP_21264_CBOX_CSRS;

/*
 * MB definitions.
 */
#define AXP_21264_1MB			(1 * ONE_M)
#define AXP_21264_2MB			(2 * ONE_M)
#define AXP_21264_4MB			(4 * ONE_M)
#define AXP_21264_8MB			(8 * ONE_M)
#define AXP_21264_16MB			(16 * ONE_M)

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
} AXP_21264_CBOX_CSR_VAL;

typedef struct
{
	const char 						*name;
	const AXP_21264_CBOX_CSR_VAL	values;
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
#define	AXP_BCACHE_1MB				0					/* b000000 */
#define	AXP_BCACHE_2MB				1					/* b000001 */
#define	AXP_BCACHE_4MB				3					/* b000011 */
#define	AXP_BCACHE_8MB				7					/* b000111 */
#define	AXP_BCACHE_16MB				15					/* b001111 */

/*
 * Mask bits for the offset, index, and tag information for the Bcache.  The
 * size of the index bits is dependent upon the size of the cache.  The offset
 * and tag bits are always the same.
 */
#define AXP_BCACHE_OFF_BITS		0x000000000000003fll	/* bits  5 -  0 */
#define AXP_BCACHE_TAG_BITS		0x00000fffffffffffll	/* bits 63 - 20 */
#define AXP_BCACHE_IDX_FILL		0x0000000000003fffll
#define AXP_BCACHE_IDX_SHIFT	6
#define AXP_BCACHE_TAG_SHIFT	20

/*
 * Macros to extract the index and tag bits from a physical address.
 */
#define AXP_BCACHE_INDEX(cpu, pa)	(((pa) >> AXP_BCACHE_IDX_SHIFT) &		\
									 (((cpu)->csr.BcSize << 14) || AXP_BCACHE_IDX_FILL))
#define AXP_BCACHE_TAG(cpu, pa)		(((pa) >> AXP_BCACHE_TAG_SHIFT) &		\
									 AXP_BCACHE_TAG_BITS)

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
#define AXP_21264_DM_NOP			0	/* b'00' */
#define AXP_21264_DM_RDHIT			1	/* b'01' */
#define AXP_21264_DM_RDDIRTY		2	/* b'10' */
#define AXP_21264_DM_RDANY			3	/* b'00' */

#define AXP_21264_NS_NOP			0	/* b'000' */
#define AXP_21264_NS_CLEAN			1	/* b'001' */
#define AXP_21264_NS_CLEAN_SHARED	2	/* b'010' */
#define AXP_21264_NS_TRANS3			3	/* b'011' */
#define AXP_21264_NS_DIRTY_SHARED	4	/* b'100' */
#define AXP_21264_NS_INVALID		5	/* b'101' */
#define AXP_21264_NS_TRANS1			6	/* b'110' */
#define AXP_21264_NS_RES			7	/* b'111' */

#define AXP_21264_GET_PROBE_DM(probe)	(((probe) & 0x18) >> 3)
#define AXP_21264_GET_PROBE_NS(probe)	((probe) & 0x07)
#define AXP_21264_SET_PROBE(dm, ns)		((((dm) & 0x03) << 3) || ((ns) & 0x07))

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
 * Mask field to indicate which bytes in the data returned from/supplied to the
 * system are relevant.
 */
#define AXP_21264_IO_INV		0x0000
#define AXP_21264_IO_BYTE		0x0001
#define AXP_21264_IO_WORD		0x0003
#define AXP_21264_IO_LONG		0x000f
#define AXP_21264_IO_QUAD		0x00ff
#define AXP_21264_ICACHE_FILL	0x0100
#define AXP_21264_DCACHE_FILL	0x0200

#if 0
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
	AXP_21264_SYSADD_OUT_CMD		command;
	AXP_21264_SYSADD_OUT_PROBE_STAT	status;
	AXP_21264_SYSADD_MASK			mask;
	bool							miss1;				/* M1 */
	bool							miss2;				/* M2 */
	bool							cacheHit;			/* CH */
	bool							readValidate;		/* RV */
	bool							dataMovement;		/* DM */
	bool							victimSent;			/* VS */
	bool							mafAddressSent;		/* MS */
	u8								maf;
	u8								vdb;
	u8								sysData[sizeof(u64)]; /* when DM=true */
	u64								pa;
} AXP_21264_SYSADD_OUT;

/*
 * HRM 4.7.7.1 and 4.7.7.2
 * This is for probes and Data Transfer Commands from the system to the Alpha
 * AXP 21264 CPU.
 */
typedef struct
{
	AXP_21264_SYSADD_IN_PROBE_CMD	command;
	AXP_21264_SYSADD_IN_SYSDC		sysDc;
	bool							probeData;	/* probe=true; data=false; */
	bool							resetVictimBuffer;	/* RVB */
	bool							resetProbeValidBit;	/* RPB */
	bool							commandAck;			/* A */
	bool							commit;				/* C */
	u8								ID;
	u64								pa;
	u8								sysData[sizeof(u64)]; /* when ProbeData=false */
} AXP_21264_SYSADD_IN;
#endif

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
 */
typedef enum
{
	toBcache,
	toMemory,
	probeResponse
} AXP_21264_VDB_TYPE;

typedef struct
{
	AXP_21264_VDB_TYPE			type;
	u64							pa;
#if 0
	union
	{
		AXP_21264_SYSADD_IN			rsp;
		AXP_21264_SYSADD_OUT		rq;
	};
#endif
	bool						validVictim;
	bool						validProbe;
	bool						processed;
	bool						valid;
	bool						marked;
	u8							data[sizeof(u64)];
	u8							dataLen;
} AXP_21264_CBOX_VIC_BUF;

/*
 * HRM 2.1.4.3
 * This structure is the definition for a single entry in the probe queue (PQ)
 */
typedef struct
{
	int							probe;
	AXP_21264_SYSDC_RSP			sysDc;
	u8							ID;
	bool						notJustSysDc;
	bool						rvb;
	bool						rpb;
	bool						a;
	bool						c;
	bool						processed;
	bool						valid;
	bool						marked;
	u64							pa;
} AXP_21264_CBOX_PQ;

/*
 * In HRM Section 2, the MAF is documented as being in the Mbox/Memory
 * Reference Unit.  In HRM Section 4.1.1.1, it states the following:
 *
 *		The Cbox contains an 8-entry miss buffer (MAF) and an 8-entry victim
 *		buffer (VAF).
 */
typedef enum
{
	MAFNotInUse,
	LDx,
	STx,
	STx_C,
	STxChangeToDirty,
	STxCChangeToDirty,
	WH64,
	ECB,
	Istream
} AXP_CBOX_MAF_TYPE;

#define AXP_21264_MBOX_MAX		8
typedef struct
{
	AXP_CBOX_MAF_TYPE	type;
	u64					pa;
	bool				valid;
	i8					lqSqEntry[AXP_21264_MBOX_MAX];
	bool				complete;	/* cleared by Mbox, set by Cbox */
} AXP_21264_CBOX_MAF;

/*
 * HRM 2.12
 * Each IOWB entry has the following:
 * 		64 bytes of data
 * 		physical address
 * 		control logic	(TBD)
 */
#define AXP_21264_SIZE_LONG		32
#define AXP_21264_SIZE_QUAD		64
typedef struct
{
	bool				processed;
	bool				valid;
	bool				aligned;
	u64					pa;
	i8					lqSqEntry[AXP_21264_MBOX_MAX];
	u8					storeLen;
	u8					data[AXP_21264_SIZE_QUAD];
	int					dataLen;
} AXP_21264_CBOX_IOWB;


/*
 * This structure is the definition for one Cbox copy of Dcache Tag Array.  A
 * block contains the following:
 *
 *		Physical tag bits
 *		Index and set into corresponding DTAG array (which is the same as the
 *			Dcache)
 *		Valid bit - the CTAG array entry is in use.
 */
typedef struct
{
	u64					physTag;
	u32					dtagIndex;
	bool				valid;
	bool				dirty;
	bool				shared;
} AXP_21264_CBOX_CTAG;

/*
 * Bcache definitions
 */
#define AXP_BCACHE_BLOCK_SIZE	64
typedef u8 AXP_21264_BCACHE_BLK[AXP_BCACHE_BLOCK_SIZE];
typedef struct
{
	bool				valid;
	bool				shared;
	bool				dirty;
	u64					tag;
} AXP_21264_BCACHE_TAG;

#endif /* _AXP_21264_CBOX_DEFS_ */
