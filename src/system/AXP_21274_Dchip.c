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
