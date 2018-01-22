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
 *	This header file contains the definitions to implemented the ITB, DTB,
 *	Icache and Dcache.
 *
 * Revision History:
 *
 *	V01.000		29-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_CACHE_DEFS_DEFS_
#define _AXP_21264_CACHE_DEFS_DEFS_

#include "AXP_Utility.h"
#include "AXP_21264_CPU.h"

/*
 * The following definitions utilize the granularity hint information in the
 * TLB initialization to determine the bits to keep for the physical address,
 * and the match and keep masks.  The GH_KEEP formula only works for 8KB pages.
 */
#define GH_KEEP(gh)		((AXP_21264_PAGE_SIZE * (0x1ll << (3 * (gh)))) - 1)
#define GH_MATCH(gh)	(((0x1ll << (AXP_21264_MEM_BITS - 1)) - 1) & ~(GH_KEEP(gh)))
#define GH_PHYS(gh)		(((0x1ll << AXP_21264_MEM_BITS) - 1) & ~(GH_KEEP(gh)))

/*
 * Define the Translation Look-aside Buffer (TLB) for both the Instruction and
 * Data Streams.
 */
typedef struct
{
	u64		virtAddr;
	u64		physAddr;
	u64		matchMask;
	u64		keepMask;
	u32		kre : 1;
	u32		ere : 1;
	u32		sre : 1;
	u32		ure : 1;
	u32		kwe : 1;
	u32		ewe : 1;
	u32		swe : 1;
	u32		uwe : 1;
	u32		faultOnRead : 1;
	u32		faultOnWrite : 1;
	u32		faultOnExecute : 1;
	u32		res_1 : 21;
	u8		asn;
	bool	_asm;
	bool	valid;
} AXP_21264_TLB;

#define AXP_CM_KERNEL		0
#define AXP_CM_EXEC			1
#define AXP_CM_SUPER		2
#define AXP_CM_USER			3
#define AXP_SPE2_VA_MASK	0x00000fffffffe000ll
#define AXP_SPE2_VA_VAL		0x2
#define AXP_SPE2_BIT		0x4
#define AXP_SPE1_VA_MASK	0x000001ffffffe000ll
#define AXP_SPE1_PA_43_41	0x00000e0000000000ll
#define AXP_SPE1_BIT		0x2
#define AXP_SPE1_VA_40		0x0000010000000000ll
#define AXP_SPE1_VA_VAL		0x7e
#define AXP_SPE0_VA_MASK	0x000000003fffe000ll
#define AXP_SPE0_VA_VAL		0x3fffe
#define AXP_SPE0_BIT		0x1

/*
 * For SuperPages, certain locations within the virtual address have to have
 * specific values.  The following structures and union are utilized to be able
 * to locate that value within a virtual address.
 *
 * This structure is used when the SPE[2] bit is set.
 */
typedef struct
{
	u64			res_1 : 46;
	u64			spe2 : 2;
	u64			res_2 : 16;
} AXP_VA_SPE2;

/*
 * This structure is used when the SPE[1] bit is set.
 */
typedef struct
{
	u64			res_1 : 41;
	u64			spe1 : 7;
	u64			res_2 : 16;
} AXP_VA_SPE1;

/*
 * This structure is used when the SPE[0] bit is set.
 */
typedef struct
{
	u64			res_1 : 30;
	u64			spe0 : 18;
	u64			res_2 : 16;
} AXP_VA_SPE0;

/*
 * This union is used to overlay the above structures over a 64-bit virtual
 * address.
 */
typedef union
{
	u64			va;
	AXP_VA_SPE2	spe2;
	AXP_VA_SPE1	spe1;
	AXP_VA_SPE0	spe0;
} AXP_VA_SPE;

/*
 * This enumeration is used to indicate the type of memory access being
 * requested.
 */
typedef enum
{
	None,		/* No access */
	Read,		/* Read */
	Write,		/* Write */
	Execute,	/* Read */
	Modify		/* Read & Write */
} AXP_21264_ACCESS;

#define AXP_DCACHE_DATA_LEN	64

typedef enum
{
	Invalid,				/* Initial State */
	Pending,				/* Waiting to be filled */
	Ready					/* Block is ready */
} AXP_21264_CACHE_ST;

/*
 * This structure is the definition for one data cache block.  A block contains
 * the following:
 *
 *		64 data bytes
 */
typedef struct
{
	u8					data[AXP_DCACHE_DATA_LEN];
} AXP_DCACHE_BLK;

/*
 * The following enumerations are used to determine the action to be performed
 * for LDx, STx, STx_C, WH64, ECV, MB/WMB instructions (HRM Table: 4-1).
 *
 * Prefetches (LDL, LDF, LDG, LDT, LDBU, LDWU) to R31 use the LDx flow, and
 * prefetch with modify intent (LDS) uses the STx flow. If the prefetch target
 * is addressed to I/O space, the upper address bit is cleared, converting the
 * address to memory space
 *
 *	Instruction		DcHit	DcW		BcHit	BcW		Status and Action
 *	-----------		-----	---		-----	---		---------------------------
 *	LDx Memory		1		X		X		X		Dcache hit, done.
 *	LDx Memory		0		X		1		X		Bcache hit, done.
 *	LDx Memory		0		X		0		X		Miss, generate RdBlk
 *													command.
 *	LDx I/O			X		X		X		X		RdBytes, RdLWs, or RdQWs
 *													based on size.
 *	Istream Memory	1		X		X		X		Dcache hit, Istream
 *													serviced from Dcache.
 *	Istream Memory	0		X		1		X		Bcache hit, Istream
 *													serviced from Bcache.
 *	Istream Memory	0		X		0		X		Miss, generate RdBlkI
 *													command.
 *	STx Memory		1		1		X		X		Store Dcache hit and
 *													writable, done.
 *	STx Memory		1		0		X		X		Store hit and not writable,
 *													set dirty flow (note 1).
 *	STx Memory		0		X		1		1		Store Bcache hit and
 *													writable, done.
 *	STx Memory		0		X		1		0		Store hit and not writable,
 *													set-dirty flow (note 1).
 *	STx Memory		0		X		0		X		Miss, generate RdBlkMod
 *													command.
 *	STx I/O			X		X		X		X		WrBytes, WrLWs, or WrQWs
 *													based on size.
 *	STx_C Memory	0		X		X		X		Fail STx_C.
 *	STx_C Memory	1		0		X		X		STx_C hit and not writable,
 *													set dirty flow (note 1).
 *	STx_C I/O		X		X		X		X		Always succeed and WrQws
 *													or WrLws are generated,
 *													based on the size.
 *	WH64 Memory		1		1		X		X		Hit, done.
 *	WH64 Memory		1		0		X		X		WH64 hit not writable, set
 *													dirty flow (note 1).
 *	WH64 Memory		0		X		1		1		WH64 hit dirty, done.
 *	WH64 Memory		0		X		1		0		WH64 hit not writable, set
 *													dirty flow (note 1).
 *	WH64 Memory		0		X		0		X		Miss, generate InvalToDirty
 *													command (note 2).
 *	WH64 I/O		X		X		X		X		NOP the instruction. WH64
 *													is UNDEFINED for I/O space.
 *	ECB Memory		X		X		X		X		Generate evict command
 *													(note 3).
 *	ECB I/O			X		X		X		X		NOP the instruction. ECB
 *													instruction is UNDEFINED
 *													for I/O space.
 *	MB/WMB			X		X		X		X		Generate MB command (note
 *													4).
 *	-----------		-----	---		-----	---		---------------------------
 *	Table 4-1 Notes:
 *	1.	Set Dirty Flow: Based on the Cbox CSR SET_DIRTY_ENABLE[2:0], SetDirty
 *		requests can be either internally acknowledged (called a SetModify) or
 *		sent to the system environment for processing. When externally
 *		acknowledged, the shared status information for the cache block is also
 *		broadcast. The commands sent externally are SharedToDirty or
 *		CleanToDirty. Based on the Cbox CSR ENABLE_STC_COMMAND[0], the external
 *		system can be informed of a STx_C generating a SetDirty using the
 *		STCChangeToDirty command. See Table 4–16 in AXP_21264_CboxDefs.h for
 *		more information.
 *	2.	InvalToDirty: Based on the Cbox CSR INVAL_TO_DIRTY_ENABLE[1:0],
 *		InvalToDirty requests can be either internally acknowledged or sent to
 *		the system environment as InvalToDirty commands. This Cbox CSR provides
 *		the ability to convert WH64 instructions to RdModx operations. See
 *		Table 4–15 in AXP_21264_CboxDefs.h for more information.
 *	3.	Evict: There are two aspects to the commands that are generated by an
 *		ECB instruction: first, those commands that are generated to notify the
 *		system of an evict being performed; second, those commands that are
 *		generated by any victim that is created by servicing the ECB.
 *		If Cbox CSR ENABLE_EVICT[0] is clear, no command is issued by the 21264
 *		on the external interface to notify the system of an evict being
 *		performed. If Cbox CSR ENABLE_EVICT[0] is set, the 21264 issues an
 *		Evict command on the system interface only if a Bcache index match to
 *		the ECB address is found in the 21264 cache system.
 *		The 21264 can issue the commands CleanVictimBlk and WrVictimBlk for a
 *		victim that is created by an ECB. CleanVictimBlk is issued only if Cbox
 *		CSR BC_CLEAN_VICTIM is set and there is a Bcache index match valid but
 *		not dirty in the 21264 cache system. WrVictimBlk is issued for any
 *		Bcache match of the ECB address that is dirty in the 21264 cache
 *		system.
 *	4.	MB: Based on the Cbox CSR SYSBUS_MB_ENABLE, the MB command can be sent
 *		to the pins.
 */
#define AXP_21264_CACHE_MISS			0x00
#define AXP_21264_CACHE_HIT				0x01
#define AXP_21264_CACHE_CLEAN			0x01
#define AXP_21264_CACHE_DIRTY			0x02
#define AXP_21264_CACHE_DIRTY_			0x03
#define AXP_21264_CACHE_SHARED			0x04
#define AXP_21264_CACHE_CLEAN_SHARED	0x05
#define AXP_21264_CACHE_DIRTY_SHARED	0x07

#define AXP_CACHE_MISS(_status)			((_status) == AXP_21264_CACHE_MISS)
#define AXP_CACHE_HIT(_status)			\
	(((_status) & AXP_21264_CACHE_HIT) == AXP_21264_CACHE_HIT)
#define AXP_CACHE_CLEAN(_status)		((_status) == AXP_21264_CACHE_CLEAN)
#define AXP_CACHE_CLEAN_SHARED(_status)	((_status) == AXP_21264_CACHE_CLEAN_SHARED)
#define AXP_CACHE_DIRTY(_status)		((_status) == AXP_21264_CACHE_DIRTY_)
#define AXP_CACHE_DIRTY_SHARED(_status)	((_status) == AXP_21264_CACHE_DIRTY_SHARED)

/*
 * This structure is the definition for one Duplicate Dcache Tag Entry.  A
 * block contains the following:
 *
 *		Physical tag bits
 *		Index into corresponding CTAG array
 *		Valid, dirty, shared, and modified bits
 *		One bit to control round-robin set allocation (one bit per two cache
 *		blocks)
 *
 *	NOTE:	This is what the individual bits represent:
 *
 *				valid:		The cache block is in use and contains valid cache
 *							data.
 *				dirty:		The cache block has been written to/updated and
 *							needs to be written back to memory upon eviction.
 *							A valid block without the 'dirty' bit set is 'clean'.
 *				shared:		The cache block can be found in more than one
 *							system component (such as another CPU)
 *				modified:	The cache block has just been retrieved from memory
 *							and needs to be written out to the Bcache upon
 *							eviction.
 *				set_0_1:	Is used to determine if the cache block should be
 *							stored in set 0 or set 1 in a round-robin fashion.
 *				locked:		Is used when a store operation that is writable is
 *							pending until the store instruction that issued it
 *							retires.  This restriction assists in STx_C
 *							instruction.
 *
 *	NOTE:	To simplify the cache functionality, the status and bits above have
 *			been moved from the DCACHE structure above.
 */
typedef struct
{
	u64					physTag;
	u32					ctagIndex;
	u32					ctagSet;
	u32					lockers;
	AXP_21264_CACHE_ST	state;
	bool				valid;
	bool				dirty;
	bool				shared;
	bool				modified;
	bool				set_0_1;
	bool				evict;
} AXP_DTAG_BLK;

#define AXP_ICACHE_LINE_INS		16
#define AXP_ICACHE_BUF_LEN		64

/*
 * This structure is the definition of one instruction cache block.  A block
 * contains the following:
 *
 *		16 Alpha instructions (64 bytes)
 *		Virtual tag bits [47:15]
 *		8-bit address space number (ASN) field
 *		1-bit address space match (ASM) bit
 *		1-bit PALcode bit to indicate physical addressing
 *		Valid bit
 *		Data and tag parity bits
 *		Four access-check bits for the following modes (KESU):
 *			kernel
 *			executive
 *			supervisor
 *			user
 *		Additional predecoded information to assist with instruction
 *		processing and fetch control
 */
typedef struct
{
	u64 				kre : 1;			/* Kernel read/execute */
	u64 				ere : 1;			/* Executive read/execute */
	u64 				sre : 1;			/* Supervisor read/execute */
	u64					ure : 1;			/* User read/execute */
	u64					_asm : 1;			/* Address Space Match */
	u64					asn : 8;			/* Address Space Number */
	u64					pal : 1;			/* PALcode */
	u64					vb : 1;				/* Valid Bit */
	u64					tag : 33;			/* Tag */
	u64					set_0_1 : 1;		/* When set 0 was last used */
	u64					res_1 : 15;			/* align to the 64-bit boundary */
	AXP_INS_FMT			instructions[AXP_ICACHE_LINE_INS];
} AXP_ICACHE_BLK;

/*
 * 2.1.5.2 Data Cache
 *
 *	 6       5         4         4         3         2         1
 *	 3       6         8         0         2         4         6         8         0
 *	+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *	|    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
 *	+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *	|                                                            |           |<68 B>|
 *	|                                                            | <512 rows>|
 *	|                                                               | <- 8KB page ->|
 *	| <---------------- Virtual Page Number (VPN) ----------------> |
 *
 * The Dcache contains 512 rows, with 2 sets each.  Each block within the data
 * cache contains 64 bytes.  The offset within these 64 bytes requires 6 bits.
 * The index for 512 rows requires 9 bits.  Therefore, the first 15 bits of a
 * virtual address are utilizing in indexing into the Dcache.  The offset
 * within an 8KB page requires 13 bits, leaving the 14th up to the 64th bits
 * for the  Virtual Page Number (VPN).  The overlapping for these 2 needs means
 * that any particular VPN may be in 4 unique locations depending upon the
 * virtual-to-physical translation of those 2 bits.  The 21264 prevents this
 * aliasing by keeping only one of the four possible translated addresses in
 * the cache at any time.
 *
 * NOTE:	Both the Data and Instruction Caches have 2 sets of 512 rows of 64
 *			bytes of data/instructions.  What works for the data cache will
 *			also work for the instruction cache.  Therefore, the data structure
 *			below is for both Dcache and Icache.
 *
 * The following structure is used for Cache indexing:
 */
typedef struct
{
	u64			offset : 6;			/* offset within 64 bytes */
	u64			index : 9;			/* Cache index */
	u64			res : 49;
} AXP_CACHE_IDX;

/*
 * The following definition is used with the Dcache when checking the status of
 * a particular VA/PA.  Since we do the search for the status check, we might
 * as well save this information and have it passed on the AXP_DchacheWrite and
 * AXP_DcacheRead functions.
 */
typedef struct
{
	u8			set;		/* set 0 or 1 */
	u8			offset;		/* offset within 64 bytes */
	u16			index;		/* index into Dcache array */
} AXP_DCACHE_LOC;


/*
 * The following data structures are used to parse out the VPN.
 */
typedef struct
{
	u64			offset : 13;		/* Offset within page */
	u64			vpn : 51;			/* Virtual Page Number */
} AXP_VA_FIELDS;

typedef struct
{
	u64			offset : 6;			/* Offset within 64 bytes */
	u64			index : 9;			/* Index from 0-511 */
	u64			tag : 49;
} AXP_IDX_FIELDS;

typedef union
{
	u64				va;
	AXP_CACHE_IDX	vaIdx;
	AXP_VA_FIELDS	vaInfo;
	AXP_IDX_FIELDS	vaIdxInfo;
} AXP_VA;

#define AXP_VA2PA(paTag, paIdx)					\
	((((paTag) << 15) & 0xffffffffffff8000) ||	\
	 (((paIdx) <<  6) & 0x0000000000007fc0))

/*
 * The following macros are used to clear out the 2-bits in a virtual and
 * physical address that are beyond that address by a virtual page, and be able
 * increment through them.  These are utilized to anti-alias addresses that
 * could be in multiple locations within the dCache, DTAG, and CTAG structures.
 */
#define AXP_MASK_2_HIGH_INDEX_BITS	0xffffffffffff9fffll
#define AXP_2_HIGH_INDEX_BITS_INCR	0x2000
#define AXP_2_HIGH_INDEX_BITS_MAX	0x8000

/*
 * The following data structures are used to parse the VPC.
 */
typedef struct
{
	u64				res_1 : 2;	/* PALmode and reserved bits */
	u64				offset : 4;
	u64				index : 9;
	u64				tag : 33;
	u64				res_2 : 16;
} AXP_VPC_FIELDS;

typedef union
{
	u64				address;
	AXP_PC			pc;
	AXP_VPC_FIELDS	vpcFields;
} AXP_VPC;

#endif /* _AXP_21264_CACHE_DEFS_DEFS_ */
