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
 *	V01.000		16-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21264_BLOCKS_
#define _AXP_21264_BLOCKS_

#include "AXP_Utility.h"

typedef enum
{
	AXP_21264_CPU_BLK,
	AXP_BLOCK_MAX
} AXP_BLOCK_TYPE;

typedef struct
{
	AXP_BLOCK_TYPE	type;
	size_t			size;
} AXP_BLOCK_DSC;

void *AXP_Allocate_Block(AXP_BLOCK_TYPE blockType);
void AXP_Deallocate_Block(AXP_BLOCK_DSC *block);

#endif /* _AXP_21264_BLOCKS_ */
