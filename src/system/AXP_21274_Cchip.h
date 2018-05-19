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
 *	This header file contains the definitions required for the Tsunami/Typhoon
 *	Cchip emulation.
 *
 * Revision History:
 *
 *	V01.000		18-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_CCHIP_H_
#define _AXP_21274_CCHIP_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_21274_21264_Common.h"
#include "AXP_21274_Registers.h"
#include "AXP_21274_Dchip.h"
#include "AXP_21274_Pchip.h"

/*
 * The following definitions are used for the bit vectors, addrMatchWait,
 * pageHit, and olderRqs.  Each bit represents an entry in the queue of 6
 * entries.
 *
 *	Note:	Although there are conceptually 3-bit vectors, they can be combined
 *			into a 2-bit vector to represent the following four comparisons
 *			against each of the other requests in the array queue (address
 *			match wait includes and overrides page hit):
 *
 *				1. Not younger
 *				2. Younger and page hit match (but not address match wait)
 *				3. Younger and address match wait
 *				4. Younger and no match
 */
#define AXP_21274_NOT_YOUNG		0x00
#define AXP_21274_YOUNG_HIT		0x01
#define AXP_21274_YOUNG_WAIT	0x02
#define AXP_21274_YOUNG			0x03
#define AXP_21274_AGE_MASK		0x03

/*
 * This macro extracts the requested 2 bits used to determine the combination
 * of Address Match Wait, Page Hit, and Older Request vectors.
 */
#define AXP_21264_ENTRY(bitVector, entry)	\
	(((bitVecotr) >> ((entry * 2)) & AXP_21274_AGE_MASK)

/*
 * HRM Table 6-7 Cchip-to-Pchip Commands
 * ----------------------------------------------------------------------------
 * Code	Command 						Cycles		Valid Fields
 * ----------------------------------------------------------------------------
 * 0000	PCI IACK cycle					2			T, Mask
 * 0001 PCI special cycle				2			T, Mask
 * 0010 PCI IO read						2			T, Mask
 * 0011 PCI IO write					2			T, Mask
 * 0100 Reserved						�			�
 * 0101 PCI memory write, PTP			2			T, Mask
 * 0110 PCI memory read					2			T, Mask
 * 0111 PCI memory write, from CPU		2			T, Mask
 * 1000 CSR read						1 or 5(1)	C-bit, CSR#
 * 1001 CSR write						1 (+1)(1)	C-bit, CSR#
 * 1010 PCI configuration read			2			T, Mask
 * 1011 PCI configuration write			2			T, Mask
 * 1100 Load PADbus data downstream		1			LDP
 * 1101 Reserved (Pchip upstream LoadP)	�			�
 * 1110 Reserved						�			�
 * 1111 No-op							1			�
 * ----------------------------------------------------------------------------
 * (1) For details, refer to command descriptions.
 *
 * HRM Table 6�8 Pchip-to-Cchip and Pchip-to-Pchip Bypass Commands
 * ----------------------------------------------------------------------------
 * Code	Command 								Cycles		Valid Fields
 * ----------------------------------------------------------------------------
 * 0000	DMA read N QW							2			T=10, Mask
 * 0001	Scatter-gather table entry read N QW	2			T=10, Mask
 * 0010	Reserved								�			�
 * 0011	Reserved								�			�
 * 0100	Reserved								�			�
 * 0101	Reserved								�			�
 * 0110	PTP memory read							2			T, Mask
 * 0111	PTP memory write						2			T=10, Mask
 * 1000	DMA RMW QW								2			T=10, only one mask
 * 															bit set
 * 1001	DMA write N QW							2			T=10, Mask
 * 1010	Reserved								�			�
 * 1011	Reserved								�			�
 * 1100	Reserved (Cchip downstream LoadP)		�			�
 * 1101	Load PADbus data upstream				2 or 5(1)	LDP
 * 1110	PTP write byte-mask bypass				2			See text in Section
 * 															6.2.3.2
 * 1111	No-op									1			�
 * ----------------------------------------------------------------------------
 * (1) For details, refer to command descriptions.
 */
typedef enum
{
    PIO_IACK, /* b'0000' - C2P */
    DMAReadNQW, /* b'0000' - P2C and P2P */
    PIO_SpecialCycle, /* b'0001' - C2P */
    SGTEReadNQW, /* b'0001' - P2C and P2P */
    PIO_Read, /* b'0010' - C2P */
    PIO_Write, /* b'0011' - C2P */
    PIO_MemoryWritePTP, /* b'0101' - C2P */
    PIO_MemoryRead, /* b'0110' - C2P */
    PTPMemoryRead, /* b'0110' - P2C and P2P */
    PIO_MemoryWriteCPU, /* b'0111' - C2P : from CPU, not to */
    PTPMemoryWrite, /* b'0111' - P2C and P2P */
    CSR_Read, /* b'1000' - C2P */
    DMARdModyWrQW, /* b'1000' - P2C and P2P */
    CSR_Write, /* b'1001' - C2P */
    DMAWriteNQW, /* b'1001' - P2C and P2P */
    PCI_ConfigRead, /* b'1010' - C2P */
    PCI_ConfigWrite, /* b'1011' - C2P */
    LoadPADbusDataDown, /* b'1100' - P2C and P2P Reserved */
    LoadPADbusDataUp, /* b'1101' - C2P Reserved */
    PTPWrByteMaskByp, /* b'1110' - P2C and P2P */
    CAPbus_NoOp /* b'1111' - C2P, P2C, and P2P */
} AXP_CAPbus_Command;

/*
 * Table 6�4 Encoding of T Field T Mask Type PADbus Transfer Characteristics
 * ----------------------------------------------------------------------------
 * T	Mask Type	PADbus Transfer Characteristics
 * ----------------------------------------------------------------------------
 * 00	Byte		One quadword transferred.
 * 01	Longword	Four quadwords are always transferred. The 8-bit mask
 * 					comprises four longword pairs. The first transferred
 * 					quadword is specified by addr<4:3> and corresponds to the
 * 					lowest-order nonzero pair of mask bits. If the number of
 * 					nonzero mask bit pairs is less than four, the trailing
 * 					quadwords on the PADbus are discarded.
 * 10	Quadword	The number of quadwords transferred is equal to the number
 * 					of asserted mask bits. The first transferred quadword is
 * 					specified by addr<5:3> and corresponds to the lowest-order
 * 					asserted mask bit.
 * 11	Illegal		Causes unspecified results.
 * ----------------------------------------------------------------------------
 */
typedef enum
{
    CAPbus_NoMask,
    CAPbus_Byte,
    CAPbus_Lowngword,
    CAPbus_Quadword
} AXP_MaskType;

/*
 * Table 6�5 C-Bit Encoding
 * ----------------------------
 * C-Bit	Meaning
 * ----------------------------
 * 0		Pchip CSR operation
 * 1		Cchip CSR operation
 * ----------------------------
 */
typedef enum
{
    NoCSR,
    PchipCSR,
    CchipCSR
} AXP_CAPbusCbit;

/*
 * Table 6�6 LDP Encoding
 * -------------------------------------------------------------------
 * LDP		Meaning for Downstream LoadP	Meaning for Upstream LoadP
 * -------------------------------------------------------------------
 * 00		LoadP DMA read					LoadP PCI read
 * 01		LoadP DMA RMW (to Pchip)		LoadP DMA RMW (from Pchip)
 * 10		LoadP PTP						LoadP CSR read
 * 11		LoadP SGTE read					LoadP CSR write
 * -------------------------------------------------------------------
 */
typedef enum
{
    NoLDP,
    LoadP_DMARead,
    LoadP_DMARMW,
    LoadP_PTP,
    LoadP_SGTERead
} AXP_CAPbusLDP;

/*
 * HRM 6.2.3 CAPbus Command Encodings
 *
 * Figure 6�3 shows the format of the 2-cycle CAPbus commands. Table 6�4 lists
 * the encoding of the T field, the number of quadwords for which the PADbus is
 * busy with the transfer, and the address of the first quadword transferred.
 * The mask field denotes which data is valid in the transfer, and is aligned
 * to eight times the data type. That is, for byte transfers, the eight bits
 * represent a total of one quadword. For longword transfers, the eight bits
 * represent four quadwords. For quadword transfers, the eight bits represent
 * up to a full cache block (eight quadwords).
 *
 *	Figure 6�3 Format of 2-Cycle Commands
 *
 *			 2  2  2  2  1  1  1  1  1  1  1  1  1  1
 *			 3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0
 *			+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *	Phase 1	|  Command  |						Address<31:12>						|
 *			+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *	Phase 2	| T| X|				Mask			|		    Address<34::3>			|
 *			+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 *	Figure 6�4 Format of 2-Cycle Commands
 *
 *			 2  2  2  2  1  1  1  1  1  1  1  1  1  1
 *			 3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0
 *			+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *	Phase 1	|  Command  | LDP | C|	Reserved |					CSR #				|
 *			+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
typedef struct
{
    AXP_QUEUE_HDR header;
    AXP_CAPbus_Command cmd; /* Cchip2Pchip or Pchip2Cchip */
    AXP_MaskType maskType; /* Cchipe2Pchip */
    AXP_CAPbusLDP ldp; /* Pchip2Cchip */
    u64 data[AXP_21274_DATA_SIZE]; /* Dchip2Pchip or Pchip2Dchip */
    u32 addr; /* Cchipe2Pchip */
    u16 csr; /* Pchip2Cchip */
    u8 mask; /* Cchipe2Pchip */
    u8 res; /* reserved */
} AXP_CAPbusMsg;

/*
 * Cchip Function Prototypes
 */
void AXP_21274_CchipInit(AXP_21274_SYSTEM *);
void *AXP_21274_CchipMain(void *);

#endif /* _AXP_21274_CCHIP_H_ */
