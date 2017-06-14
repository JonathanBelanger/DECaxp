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
 *	This header file contains definitions required to implement register
 *	renaming for the Alpha AXP 21264 CPU.
 *
 * Revision History:
 *
 *	V01.000		10-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_REGISTER_RENAMING_DEFS_
#define _AXP_REGISTER_RENAMING_DEFS_

#include "AXP_Utility.h"

/*
 * This enumeration is used during during the Issue, Register Read, and Execute
 * stages (Stages 3, 4, and 5).  It indicates the state of one PR.  There are
 * an equal number of these entries as there are physical registers.
 */
typedef enum
{
	Free,
	Pending,
	Valid
} AXP_21264_REG_STATE;

/*
 * This structure is used during the Map stage (Stage 2) to locate the current
 * AR to PR mapping.  The source registers, there can be 0, 1, or 2 of these,
 * utilize the existing mapping of that register.  The destination register, of
 * which there can only be 0 or 1, gets a new mapping from the free list.
 * There are an equal number of these entries as there are architectural
 * registers.
 */
typedef struct
{
	u16				pr;				/* Current Physical Register */
	u16				prevPr;			/* Previous Physical Register */
} AXP_21264_REG_MAP;

/*
 * Alpha AXP Instructions have between 0 and 3 register references.  These
 * registers can represent 0 to 2 source registers and 0 or 1 destination
 * registers.  The destination register is most often Ra.  The first source
 * register is most often Rb, with the second being Rc.  Unfortunately, this is
 * not always the case.  The below masks will be used to determine which
 * register is which.  There will be one set of masks for each instruction.
 * The masks will be used during the decoding to set up the values required for
 * the register naming process.  These masks will be defined for use with a
 * unsigned 16-bit value.
 */
#define AXP_DEST_RA	0x0100
#define AXP_DEST_RB	0x0200
#define AXP_DEST_RC	0x0400
#define AXP_DEST_FA	0x0900
#define AXP_DEST_FB	0x0a00
#define AXP_DEST_FC	0x0c00

#define AXP_SRC1_RA	0x0010
#define AXP_SRC1_RB	0x0020
#define AXP_SRC1_RC	0x0040
#define AXP_SRC1_FA	0x0090
#define AXP_SRC1_FB	0x00a0
#define AXP_SRC1_FC	0x00c0

#define AXP_SRC2_RA	0x0001
#define AXP_SRC2_RB	0x0002
#define AXP_SRC2_RC	0x0004
#define AXP_SRC2_FA	0x0009
#define AXP_SRC2_FB	0x000a
#define AXP_SRC2_FC	0x000c

#endif /* _AXP_REGISTER_RENAMING_DEFS_ */
