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
 *  This header file contains the definitions to allow the emulator to use one
 *  or more ethernet devices and send and receive packets over then for
 *  specific MAC addresses.
 *
 * Revision History:
 *
 *  V01.000	28-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_ETHERNET_H_
#define _AXP_ETHERNET_H_
#define HAVE_REMOTE	1
#include <pcap.h>

#define AXP_ETH_READ_TIMEOUT	1000

/*
 * Ethernet handle.  Ethernet packets will be sent and received through this
 * handle.
 */
typedef struct
{
    pcap_t	*handle;
    char	errorBuf[PCAP_ERRBUF_SIZE];
} AXP_Ethernet_Handle;

/*
 * Function prototypes.
 */
AXP_Ethernet_Handle *AXP_EthernetOpen(char *);
void AXP_EthernetClose(AXP_Ethernet_Handle *);


#endif /* _AXP_ETHERNET_H_ */
