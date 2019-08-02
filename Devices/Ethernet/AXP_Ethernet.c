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
 *  This source file contains the code to allow the emulator to use one or more
 *  ethernet devices and send and receive packets over then for specific MAC
 *  addresses.
 *
 * Revision History:
 *
 *  V01.000	28-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Configure.h"
#include "CommonUtilities/AXP_Trace.h"
#include "Devices/Ethernet/AXP_Ethernet.h"
#include "CommonUtilities/AXP_Blocks.h"

/*
 * AXP_EthernetOpen
 *  This function is called to open an ethernet device for sending and
 *  receiving packets over the device.
 *
 * Input Parameters:
 *  name:
 *	A pointer to the string containing the name of the device to open.
 *  cardNo:
 *	An unsigned byte indicating the network card number (0x00, 0x01, ...,
 *	0xff).
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  NULL:	Failed to open the device indicated.
 *  ~NULL:	A pointer to the Ethernet Handle through which packets are
 *		send and received.
 */
AXP_Ethernet_Handle *AXP_EthernetOpen(char *name, u8 cardNo)
{
    AXP_Ethernet_Handle *retVal;

    retVal = AXP_Allocate_Block(AXP_ETHERNET_BLK);
    if (retVal != NULL)
    {

  retVal->handle = pcap_open_live(name,
                                  SIXTYFOUR_K,
                                  true,
                                  AXP_ETH_READ_TIMEOUT,
                                  retVal->errorBuf);
  if (retVal->handle == NULL)
  {
      AXP_Deallocate_Block(retVal);
      retVal = NULL;
  }
  else
  {
      retVal->macAddr[0] = 0x08;
      retVal->macAddr[1] = 0x00;
      retVal->macAddr[2] = 0x2b;
      retVal->macAddr[3] = 0xde;
      retVal->macAddr[4] = 0xcc;
      retVal->macAddr[5] = cardNo;
  }
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_EthernetClose
 *  This function is called to close an ethernet device that is no longer
 *  needed.
 *
 * Input Parameters:
 *  handle:
 *	A pointer to the handle created in the AXP_EthernetOpen call.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void AXP_EthernetClose(AXP_Ethernet_Handle *handle)
{
    if (handle->handle != NULL)
    {
  pcap_close(handle->handle);
  handle->handle = NULL;
    }
    AXP_Deallocate_Block(handle);

    /*
     * Return back to the caller.
     */
    return;
}



