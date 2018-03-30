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
 *	This header file contains the code for the protocol sent from the System to
 *	the CPU over the Sysbus.
 *
 *	Revision History:
 *
 *	V01.000		30-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */

#ifndef AXP_21274_TO_CPU_H_
#define AXP_21274_TO_CPU_H_

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
} AXP_21274_PROBE_RQ;

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
} AXP_21274_SYSDC;

#define AXP_21274_DATA_SIZE	8	/* sysData size in quadwords */
typedef struct
{
	bool				probe;
	AXP_21274_PROBE_RQ	cmd;
	u64					pa;
	AXP_21274_SYSDC		sysDc;
	bool				rvb;	/* Clear Victim or IOWB buffer if valid */
	bool				rpb;	/* Clear Probe Valid bit */
	bool				a;		/* Command acknowledge */
	bool				c;		/* Decrements uncommitted event counter */
	u8					id;		/* ID for VDB or IOWB */
	u64					sysData[AXP_21274_DATA_SIZE];
} AXP_21274_SYSBUS_MSG;

#endif /* AXP_21274_TO_CPU_H_ */
