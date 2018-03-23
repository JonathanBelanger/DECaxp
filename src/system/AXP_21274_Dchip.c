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
 *	The Dchip performs the following functions:
 *		- Implements data flow between the Pchips, CPUs, and memory
 *		- Shifts data to and from the PADbus, as required
 *		- Provides Pchip queue buffering
 *		- Provides memory data buffering
 *		- Implements data merging for quadword write operations to memory and
 *		  the DMA RMW command
 *
 * Dchip architecture does not implement:
 * 		- Flow control
 * 		- Error detection
 * 		- Error reporting
 * 		- Error correction
 * 		- Data wrapping
 *
 * The Dchip uses multiplexers to switch data among its ports and queues. In
 * addition to moving data from one port to another, these multiplexers must
 * support the various system configurations. The system may have two, four, or
 * eight Dchips. This allows for one or two 21264 CPUs, one or two Pchip ports,
 * and one or two 16-byte or 32-byte memory buses. Data may be moved between
 * the CPU, Pchips, or memory ports. Also, data may be transferred between the
 * two or up to 4 CPU ports. PTP transfers are supported between Pchip ports.
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21274_Dchip.h"

/*
 * AXP_21274_DchipInit
 *	This function is called to initialize the Dchip CSRs as documented in HRM
 *	10.2 Chipset Registers.
 *
 * Input Parameters:
 *	sys:
 *		A pointer to the system data structure from which the emulation
 *		information is maintained.
 *
 * Output Parameters:
 *	sys:
 *		A pointer to the system data structure, with the Dchip CSRs initialized
 *		based on the HRM for Tsunami/Typhoon chip set.
 *
 * Return Values:
 *	None.
 */
void AXP_21274_DchipInit(AXP_21274_SYSTEM *sys)
{

	/*
	 * Initialization for DSC (HRM Table 10-31)
	 *
	 * TODO: Need to determine how a CPM command from the Cchip is handled.
	 */
	sys->dsc.res_7 = 0;
	sys->dsc.p1p = 0;		/* Pchip 1 present from CPM command from Cchip */
	sys->dsc.c3cfp = 0;		/* CPU3 clock forward from CPM command from Cchip */
	sys->dsc.c2cfp = 0;		/* CPU2 clock forward from CPM command from Cchip */
	sys->dsc.c1cfp = 0;		/* CPU1 clock forward from CPM command from Cchip */
	sys->dsc.c0cfp = 0;		/* CPU0 clock forward from CPM command from Cchip */
	sys->dsc.bc = 0;		/* Base Configuration from CPM command from Cchip */

	/*
	 * The first byte is replicated 8 times.
	 */
	sys->dsc.dchip7 = sys->dsc.dchip6 = sys->dsc.dchip5 = sys->dsc.dchip4 =
			sys->dsc.dchip3 = sys->dsc.dchip2 = sys->dsc.dchip1 =
			sys->dsc.dchip0;

	/*
	 * Initialization for DSC2 (HRM Table 10-32)
	 *
	 * TODO: Need to determine how a PADCMD buss from the Cchip is handled.
	 */
	sys->dsc2.res_5 = 0;
	sys->dsc2.res_2 = 0;
	sys->dsc2.p1w = 0;
	sys->dsc2.p0w = 0;

	/*
	 * Initialization for STR (HRM Table 10-33)
	 */
	sys->str.res_7 = 0;
	sys->str.iddw = 2;
	sys->str.iddr = 4;
	sys->str.aw = 0;

	/*
	 * The first byte is replicated 8 times.
	 */
	sys->str.dchip7 = sys->str.dchip6 = sys->str.dchip5 = sys->str.dchip4 =
			sys->str.dchip3 = sys->str.dchip2 = sys->str.dchip1 =
			sys->str.dchip0;

	/*
	 * Initialization for DREV (HRM Table 10-34)
	 */
	sys->dRev.res_60 = 0;
	sys->dRev.rev7 = 1;
	sys->dRev.res_52 = 0;
	sys->dRev.rev6 = 1;
	sys->dRev.res_44 = 0;
	sys->dRev.rev5 = 1;
	sys->dRev.res_36 = 0;
	sys->dRev.rev4 = 1;
	sys->dRev.res_28 = 0;
	sys->dRev.rev3 = 1;
	sys->dRev.res_20 = 0;
	sys->dRev.rev2 = 1;
	sys->dRev.res_12 = 0;
	sys->dRev.rev1 = 1;
	sys->dRev.res_4 = 0;
	sys->dRev.rev0 = 1;

	/*
	 * Return back to the caller.
	 */
	return;
}
