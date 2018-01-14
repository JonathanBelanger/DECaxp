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
 *
 *	V01.001		15-Jun-2017	Jonathan D. Belanger
 *	Added a structure to more easily decode the register masks, especially
 *	for the opcodes that have varying ways its registers get associated as a
 *	destination and sources.
 *
 *	V01.002		16-Jun-2017	Jonathan D. Belanger
 *	The structured defined in the above change is too complex to be useful.
 *	There just needs to be a field for each 4 bits (DEST, SRC1, SRC2, and
 *	OPCODEFUNC).
 */
#ifndef _AXP_REGISTER_RENAMING_DEFS_
#define _AXP_REGISTER_RENAMING_DEFS_

/*
 * This enumeration is used during during the Issue, Register Read, and Execute
 * stages (Stages 3, 4, and 5).  It indicates the state of one PR.  There are
 * an equal number of these entries as there are physical registers.
 */
typedef enum
{
	Free,
	PendingUpdate,
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
#define AXP_DEST_RA		0x0100		/* Ra as a destination register 	*/
#define AXP_DEST_RB		0x0200		/* Rb as a destination register 	*/
#define AXP_DEST_RC		0x0400		/* Rc as a destination register 	*/
#define AXP_DEST_FA		0x0900		/* Fa as a destination register 	*/
#define AXP_DEST_FB		0x0a00		/* Fb as a destination register 	*/
#define AXP_DEST_FC		0x0c00		/* Fc as a destination register 	*/
#define AXP_DEST_FLOAT	0x0800		/* Destination register is FP		*/

#define AXP_SRC1_RA		0x0010		/* Ra as a source 1 register 		*/
#define AXP_SRC1_RB		0x0020		/* Rb as a source 1 register 		*/
#define AXP_SRC1_RC		0x0040		/* Rc as a source 1 register 		*/
#define AXP_SRC1_FA		0x0090		/* Fa as a source 1 register 		*/
#define AXP_SRC1_FB		0x00a0		/* Fb as a source 1 register 		*/
#define AXP_SRC1_FC		0x00c0		/* Fc as a source 1 register 		*/

#define AXP_SRC2_RA		0x0001		/* Ra as a source 2 register 		*/
#define AXP_SRC2_RB		0x0002		/* Rb as a source 2 register 		*/
#define AXP_SRC2_RC		0x0004		/* Rc as a source 2 register 		*/
#define AXP_SRC2_FA		0x0009		/* Fa as a source 2 register 		*/
#define AXP_SRC2_FB		0x000a		/* Fb as a source 2 register 		*/
#define AXP_SRC2_FC		0x000c		/* Fc as a source 2 register 		*/

#define AXP_OPCODE_11	0x1000		/* Opcode 11 needs special handling	*/
#define AXP_OPCODE_14	0x2000		/* Opcode 14 needs special handling	*/
#define AXP_OPCODE_15	0x3000		/* Opcode 15 needs special handling	*/
#define AXP_OPCODE_16	0x4000		/* Opcode 16 needs special handling	*/
#define AXP_OPCODE_17	0x5000		/* Opcode 17 needs special handling	*/
#define AXP_OPCODE_18	0x6000		/* Opcode 18 needs special handling	*/
#define AXP_OPCODE_1C	0x7000		/* Opcode 1c needs special handling	*/

#define AXP_REG_RA		0x1
#define AXP_REG_RB		0x2
#define AXP_REG_RC		0x4
#define AXP_REG_FA		0x9
#define AXP_REG_FB		0xa
#define AXP_REG_FC		0xc

struct regDecode
{
	u16				src2 : 4;
	u16				src1 : 4;
	u16				dest : 4;
	u16				opcodeRegDecode	: 4;
};

typedef union
{
	u16					raw;
	struct regDecode	bits;
} AXP_REG_DECODE;

#endif /* _AXP_REGISTER_RENAMING_DEFS_ */
