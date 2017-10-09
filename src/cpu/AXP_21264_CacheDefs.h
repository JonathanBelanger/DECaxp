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
#ifndef _AXP_21264_CACHE_DEFS_
#define _AXP_21264_CACHE_DEFS_

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

/*
 * This structure is the definition for one data cache block.  A block contains
 * the following:
 *
 *		64 data bytes
 *		Physical tag bits
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
 */
typedef struct
{
	u8					data[AXP_DCACHE_DATA_LEN];
	u64					physTag;
	bool				valid;
	bool				dirty;
	bool				shared;
	bool				modified;
	bool				set_0_1;
	bool				locked;
} AXP_DCACHE_BLK;

/*
 * This structure is the definition for one Duplicate Dcache Tag Entry.  A
 * block contains the following:
 *
 *		Physical tag bits
 *		Valid bit
 *			The DTAG array entry is in use.
 */
typedef struct
{
	u64					physTag;
	bool				valid;
} AXP_DTAG_BLK;

#define AXP_ICACHE_LINE_INS		16

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
 *	|                                                            |           |      | 64 bytes
 *	|                                                            |           | 512 rows
 *	|                                                               |               | 8KB page
 *	|                                                               | Virtual Page Number (VPN)
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
	u64			index : 7;
	u64			counter : 2;
	u64			res : 49;
} AXP_IDX_COUNTER;

typedef union
{
	u64				va;
	AXP_CACHE_IDX	vaIdx;
	AXP_VA_FIELDS	vaInfo;
	AXP_IDX_COUNTER	vaIdxCntr;
} AXP_VA;

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

#endif /* _AXP_21264_CACHE_DEFS_ */
