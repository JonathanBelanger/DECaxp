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
 *	The Pchip is the interface chip between devices on the PCI bus and the rest
 *	of the system.  There can be one or two Pchips, and corresponding single or
 *	dual PCI buses, connected to the Cchip and Dchips. The Pchip performs the
 *	following functions:
 *		- Accepts requests from the Cchip by means of the CAPbus and enqueues
 *		  them
 *		- Issues commands to the PCI bus based on these requests
 *		- Accepts requests from the PCI bus and enqueues them
 *		- Issues commands to the Cchip by means of the CAPbus based on these
 *		  requests
 *		- Transfers data to and from the Dchips based on the above commands and
 *		  requests
 *		- Buffers the data when necessary
 *		- Reports errors to the Cchip, after recording the nature of the error
 *
 * Revision History:
 *
 *	V01.000		22-Mar-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_21274_Pchip.h"
