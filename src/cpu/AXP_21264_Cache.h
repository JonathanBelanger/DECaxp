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
 */
#define GH_KEEP(gh)		(AXP_21264_PAGE_SIZE < (3 * (gh)) - 1)
#define GH_MATCH(gh)	(((1 << (AXP_21264_MEM_BITS - 1)) - 1) & ~(GH_KEEP(gh)))
#define GH_PHYS(gh)		(((1 << AXP_21264_MEM_BITS) - 1) & ~(GH_KEEP(gh)))

/*
 * Define the Translation Lookaside Buffer (TLB) for both the Instruction and
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
	u64			spe2 : 7;
	u64			res_2 : 16;
} AXP_VA_SPE1;

/*
 * This structure is used when the SPE[0] bit is set.
 */
typedef struct
{
	u64			res_1 : 30;
	u64			spe2 : 18;
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
	None,		// No access
	Read,		// Read
	Write,		// Write
	Execute,	// Read
	Modify		// Read & Write
} AXP_21264_ACCESS;

#define AXP_21264_DCACHE_DATA_LEN	64

typedef struct
{
	u64			res_1 : 13;
	u64			tag : 33;
	u64			res_2 : 18;
} AXP_21264_PHYS_TAG;

/*
 * This structure is the definition for one data cache block.  A block contains
 * the following:
 *
 *		64 data bytes
 *		Physical tag bits
 *		Valid, dirty, shared, and modified bits
 *		One bit to control round-robin set allocation (one bit per two cache blocks)
 */
typedef struct
{
	u8					data[AXP_21264_DCACHE_DATA_LEN];
	AXP_21264_PHYS_TAG	physTag;
	bool				valid;
	bool				dirty;
	bool				shared;
	bool				modified;
	bool				set_0_1;
} AXP_21264_DCACHE_BLK;

#endif /* _AXP_21264_CACHE_DEFS_ */
