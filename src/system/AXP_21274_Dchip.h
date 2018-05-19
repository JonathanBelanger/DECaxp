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
 *	Dchip emulation.
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_DCHIP_H_
#define _AXP_21274_DCHIP_H_

#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_21274_Registers.h"
#include "AXP_21274_Cchip.h"
#include "AXP_21274_Pchip.h"

/*
 * Table 7–2 PADbus Command Encodings
 * ----------------------------------------------------------------------------
 * VCCT		Mnemonic	Command
 * ----------------------------------------------------------------------------
 * 0---		—			No-op
 * 1000		P–FPQ		Move data from the Pchip to the Dchips
 * 1001		TPQM–P		Move data to the Pchip from the Dchip’s TPQM
 * 1010		P–WMB		Return data from Pchip to Dchips for RMW
 * 1011		WMB–P		Move data from Dchips to Pchip for RMW
 * 1100		PP–FPQ		Stutter move of data from the Pchip to the Dchips
 * 1101		TPQP–P		Move data to the Pchip from the Dchip’s TPQP
 * 111x		—			Reserved
 * ----------------------------------------------------------------------------
 *
 * The special “stutter” command is used for PIO read byte and PIO read
 * longword operations from a CPU. In these cases, the transfer to the CPU must
 * have each quadword sent twice in succession. To accomplish this, each
 * quadword from the Pchip is written into two successive locations in the FPQ
 * when the PP–FPQ command is received. Then, a normal CPM command is used to
 * transfer the data from the FPQ to the CPU.
 */
typedef enum
{
    PADbus_NoOp, /* b'0000'-b'0111' */
    P_FPQ, /* b'1000' */
    TPQM_P, /* b'1001' */
    P_WMB, /* b'1010' */
    PP_FPQ, /* b'1011' */
    TPQP_P /* b'1101' */
} AXP_PADbusCommand;

/*
 * HRM 7.3.1 Dchip-PADbus Interface Control — PAD Commands
 *
 * The Cchip issues PADbus commands to the Dchips to control the movement of
 * data between the Pchips and the Dchips. Data from a Pchip is loaded into the
 * FPQ, and data to the Pchips is unloaded from the TPQ. The two-phase command
 * encoding is shown in Table 7–1.
 *
 *	Table 7–1 PADbus Command Format
 *
 *			  4   3   2   1   0
 *			+---+---+---+---+---+
 *	Cycle 1	| V | C | C | T | P |
 *			+---+---+---+---+---+
 *	Cycle 2	| S1 S0 |   Length  |
 *			+---+---+---+---+---+
 *
 * The V bit typically indicates that the command is valid. The T field
 * typically indicates that data movement is to the Pchip. The P field
 * indicates whether Pchip0 or Pchip1 is involved in the transaction.
 *
 * The full VCCT field is interpreted as outlined in Table 7–2.
 */
typedef struct
{
    AXP_PADbusCommand cmd;
    u8 shift;
    u8 len; /* in quadwords */
    u64 data[8]; /* up to 8 quadwords */
} AXP_PADbusMsg;

/*
 * Dchip Function Prototypes
 */
void AXP_21274_DchipInit(AXP_21274_SYSTEM *);

#endif /* _AXP_21274_DCHIP_H_ */
