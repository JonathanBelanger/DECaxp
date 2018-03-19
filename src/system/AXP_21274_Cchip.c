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
 *	This module contains the code for the Tsunami/Typhoon Cchip.  The Cchip
 *	performs the following functions:
 *		- Accepts requests from the Pchips and the CPUs
 *		- Orders the arriving requests as required
 *		- Selects among the requests to issue controls to the DRAMs
 *		- Issues probes to the CPUs as appropriate to the selected requests
 *		- Translates CPU PIO addresses to PCI and CSR addresses
 *		- Issues commands to the Pchip as appropriate to the selected (PIO or
 *		  PTP) requests
 *		- Issues responses to the Pchip and CPU as appropriate to the issued
 *		  requests
 *		- Issues controls to the Dchip as appropriate to the DRAM accesses, and
 *		  the probe and Pchip responses
 *		- Controls the TIGbus to manage interrupts, and maintains CSRs
 *		  including those that represent interrupt status
 *	The Tsunami supports up to 2 CPUs and the Typhoon supports up to 4 CPUs.
 *
 * Revision History:
 *
 *	V01.000		18-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21274_Cchip.h"
