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
 *	This header file contains common definitions needed to be able to send
 *	and receive messages between the CPU and System.  There is a companion
 *	header file defined in the system folder.  The only difference between
 *	these is the fact that these definitions have 21264 in them and the system
 *	header file, system/AXP_21274_21264_Common.h, have 21274 in them.  This
 *	file is defined so that the CPU only needs to know the minimal amount about
 *	the definitions for the System emulation and the System emulation about the
 *	CPU emulation definitions.  I want to keep these 2 things as separate as
 *	is possible.
 *
 *	NOTE:	Any change in this header file must be replicated, with the above
 *			documented differences, in the companion
 *			system/AXP_21274_21264_Common.h header file.
 *
 * Revision History:
 *
 *	V01.000		31-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_21274_COMMON_H_
#define _AXP_21264_21274_COMMON_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"

typedef enum
{
	NOP_NOP,
	NOP_Clean,
	NOP_CleanShared,
	NOP_Transition3,
	NOP_Transition1 = 0x06,
	ReadHit_NOP = 0x08,
	ReadHit_Clean,
	ReadHit_CleanShared,
	ReadHit_Transition3,
	ReadHit_Transition1 = 0x0e,
	ReadDirty_NOP = 0x10,
	ReadDirty_Clean,
	ReadDirty_CleanShared,
	ReadDirty_Transition3,
	ReadDirty_Transition1 = 0x16,
	ReadAny_NOP = 0x18,
	ReadAny_Clean,
	ReadAny_CleanShared,
	ReadAny_Transition3,
	ReadAny_Transition1 = 0x1e
} AXP_21264_PROBE_RQ;

typedef enum
{
	SysDC_Nop,
	ReadDataError,
	ChangeToDirtySuccess = 0x04,
	ChangeToDirtyFail,
	MBDone,
	ReleaseBuffer,
	WriteData = 0x08,
	ReadData = 0x10,
	ReadDataDirty = 0x14,
	ReadDataShared = 0x18,
	ReadDataSharedDirty = 0x1c
} AXP_21264_SYSDC;

/*
 * sysData size in quadwords
 */
#define AXP_21264_DATA_SIZE	8

/*
 * The following data structure will be used to send Probe Requests and sysDc
 * responses, with or without the Probe Request, with or without data from the
 * System to the target CPU.
 */
typedef struct
{
	u64					sysData[AXP_21264_DATA_SIZE];	/* Data Movement */
	u64					pa;		/* Physical Address */
	AXP_21264_PROBE_RQ	cmd;	/* System to CPU Probe Request Command */
	AXP_21264_SYSDC		sysDc;	/* Response to Command from CPU */
	bool				probe;	/* Does the message contain a Probe Request */
	bool				rvb;	/* Clear Victim or IOWB buffer if valid */
	bool				rpb;	/* Clear Probe Valid bit */
	bool				a;		/* Command acknowledge */
	bool				c;		/* Decrements uncommitted event counter */
	u8					id;		/* ID for VDB or IOWB */
	u8					wrap;	/* Read and Write wrap data */
} AXP_21264_SYSBUS_CPU;


typedef enum
{
	Sysbus_NOP,
	ProbeResponse,
	NZNOP,
	VDBFlushRequest,
	WrVictimBlk,
	CleanVictimBlk,
	Evict,
	Sysbus_MB,
	ReadBytes,
	ReadLWs,
	ReadQWs,
	WrBytes = 0x0c,
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
} AXP_21264_Commands;

typedef enum
{
	HitClean,
	HitShared,
	HitDirty,
	HitSharedDirty
} AXP_21264_ProbeStatus;

/*
 * The following data structure will be used to send Requests and Probe
 * Responses, with or without data from the CPU to the System.  The skid buffer
 * in which this message is queued is specific to a CPU.  This is how the
 * System keeps track of which response will go to which CPU.
 */
typedef struct
{
	u64					sysData[AXP_21264_DATA_SIZE];	/* Data Movement */
	u64					pa;		/* Physical Address */
	AXP_21264_Commands	cmd;	/* CPU to System Command */
	bool				probe;	/* Is the message a Probe Response */
	bool				m1;		/* Oldest Probe Miss */
	bool				m2;		/* Oldest Probe Miss or hit with no data movement */
	bool				ch;		/* Cache hit, along with m2, with no data movement */
	bool				rv;		/* validates command */
	u8					mask;	/* sysData in-use for I/O, Byte, LW, or QW */
	u8					id;		/* MAF or VDB or IOWB identifier for command */
	u8					wrap;	/* Read and Write wrap data */
} AXP_21264_SYSBUS_System;

/*
 * The following definition is used to be able to set the probe queue (PQ) item
 * in the CPU, so that CPU can process a Probe Request and SysDc response from
 * the system.
 */
typedef struct
{
	u64							pa;
	AXP_21264_SYSDC				sysDc;
	AXP_21264_ProbeStatus		probeStatus;
	int							probe;
	bool						rvb;
	bool						rpb;
	bool						a;
	bool						c;
	bool						processed;
	bool						valid;
	bool						pendingRsp;
	bool						dm;
	bool						vs;
	bool						ms;
	u8							ID;
	u8							sysData[AXP_21264_DATA_SIZE];
	u8							vdb;
	u8							maf;
	u8							wrap;
} AXP_21264_CBOX_PQ;

typedef enum
{
	phase0,
	phase1,
	phase2,
	phase3
} AXP_21264_PHASES;

/*
 * HRM 6.1.1 Memory Access Request Queues, Skid Buffers, and Dispatch Register
 *
 * Each new request that arrives from a CPU or Pchip is eventually dispatched
 * into one of four request queues. Request queues have the following
 * characteristics:
 *
 *	- Each queue corresponds to one of the memory arrays controlled by the Cchip.
 *	- Each queue has six entries.
 *
 * HRM 6.1.4 Request Queue Maintenance
 *
 * The request queue is a unified queue of all requests from the CPUs and the
 * Pchips. In an implementation-dependent manner, the relative ages of any set
 * of entries can be determined. Each queue entry contains the following
 * information:
 *
 *	- Command and other information, such as; CPU MAF/VAF id, number of QW for
 *	  DMA ops, and PIO mask
 *	- Address
 *	- Phase, Valid
 *	- Status (such as probe results)
 *	- Address match wait vector – A bit vector identifying the older requests
 *	  in this queue with (nearly) the same address, and for which this request
 *	  must wait
 *	- Page hit vector – A bit vector identifying the older requests in this
 *	  queue with the same DRAM page address, so that this request can issue
 *	  after a previous request without waiting for RAS precharge delay
 *	- Older request vector – A bit vector identifying all older requests in
 *	  this queue (used to arbitrate among otherwise equal ready requests)
 */
typedef struct
{
	u64						sysData[AXP_21264_DATA_SIZE];
	u64						mask;
	u64						pa;
	AXP_21264_Commands		cmd;
	AXP_21264_ProbeStatus	status;
	AXP_21264_PHASES		phase;
	int						entry;
	int						sysDataLen;
	u16						waitVector;
	bool					miss1;
	bool					miss2;
	bool					rqValid;
	bool					cacheHit;
	bool					valid;
} AXP_21264_RQ_ENTRY;

#define AXP_21264_CCHIP_RQ_LEN	6	/* Per CPU */

#define AXP_21264_PQ_LEN		8

#endif /* _AXP_21264_21274_COMMON_H_ */
