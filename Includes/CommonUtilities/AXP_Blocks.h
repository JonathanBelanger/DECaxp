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
 *	This header file contains the definitions required to implement the memory
 *	allocation of data blocks for the emulation of the Alpha 21264 (EV68)
 *	processor.
 *
 *	Revision History:
 *
 *  V01.000		16-May-2017	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001		19-Jul-2018	Jonathan D. Belanger
 *  I am changing the way memory is allocated and deallocated.  In the initial
 *  implementation, a memory block needed to have the below structure defined
 *  at it's top.  The new implementation will have a hidden block before and
 *  after the block being requested.  The header and trailer blocks will have
 *  magic numbers in them at the boundary of the memory block being requested.
 *  This will help to determine when memory was overwritten.  Also, the header
 *  header block will contain a place for the block to be queued up, so that we
 *  can detect when a block is deallocated more than once, or not deallocated
 *  at all.
 */
#ifndef _AXP_21264_BLOCKS_
#define _AXP_21264_BLOCKS_

#include "CommonUtilities/AXP_Utility.h"
#include "21264Processor/AXP_21264_CPU.h"
#include "TyphoonTsunami/AXP_21274_System.h"
#include "Devices/Console/AXP_Telnet.h"
#include "Devices/VirtualDisks/AXP_VHDX.h"
#include "Devices/VirtualDisks/AXP_RAW.h"
#include "Devices/VirtualDisks/AXP_SSD.h"
#include "Devices/Ethernet/AXP_Ethernet.h"

typedef enum
{
    AXP_UNKNOWN_BLK,
    AXP_21264_CPU_BLK,
    AXP_21274_SYS_BLK,
    AXP_TELNET_SES_BLK,
    AXP_ETHERNET_BLK,
    AXP_SSD_BLK,
    AXP_VHDX_BLK,
    AXP_VOID_BLK,
    AXP_RAW_BLK,
    AXP_BLOCK_MAX
} AXP_BLOCK_TYPE;

#define AXP_HD_MAGIC	0x5555deadbeefaaaall
#define AXP_TL_MAGIC	0xaaaa215241105555ll

typedef struct
{
    u64			magicNumber;
    size_t		size;
    AXP_BLOCK_TYPE	type;
} AXP_BLOCK_TL;

typedef struct
{
    AXP_QUEUE_HDR	head;
    AXP_BLOCK_TYPE	type;
    size_t		size;
    AXP_BLOCK_TL	*tail;
    u64			magicNumber;
} AXP_BLOCK_HD;

/*
 * These are local typedefs to make allocating and returning memory a bit
 * easier.
 */
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_21264_CPU	cpu;
    AXP_BLOCK_TL	tail;
} _CPU_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_21274_SYSTEM	sys;
    AXP_BLOCK_TL	tail;
} _SYS_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_TELNET_SESSION	ses;
    AXP_BLOCK_TL	tail;
} _SES_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_Ethernet_Handle	eth;
    AXP_BLOCK_TL	tail;
} _ETH_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_SSD_Handle	ssd;
    AXP_BLOCK_TL	tail;
} _SSD_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_VHDX_Handle	vhdx;
    AXP_BLOCK_TL	tail;
} _VHDX_BLK;
typedef struct
{
    AXP_BLOCK_HD	head;
    AXP_RAW_Handle	raw;
    AXP_BLOCK_TL	tail;
} _RAW_BLK;

void *AXP_Allocate_Block(i32 blockType, ...);
void AXP_Deallocate_Block(void *block);
AXP_BLOCK_TYPE AXP_ReturnType_Block(void *);

#endif /* _AXP_21264_BLOCKS_ */
