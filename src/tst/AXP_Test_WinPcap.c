#include "AXP_Utility.h"
#include <pcap.h>

int main(void)
{
    pcap_if_t   *allDevices = NULL, *device;
    pcap_addr   *deviceAddr;
    char        errorBuf[PCAP_ERRBUF_SIZE];
    int         retVal, ii = 1;

    /*
     * Call WinPcap to return all network devices that can be opened.
     */
    retVal = pcap_findalldevs_ex(
                        PCAP_SRC_IF_STRING,
                        NULL,           /* Authentication is not needed */
                        &allDevices,
                        errorBuf);

    /*
     * If the above call returns a success, then if any devices were returned,
     * then we need to loop through them and display some information.
     * Otherwise, report the error returned.
     */
    if (retVal == 0)
    {

        /*
         * We only look through the list of devices if any have been returned.
         * If none were returned, then there could be an issue with the
         * installation of WinPcap.
         */
        if (allDevices != NULL)
        {

            /*
             * Loop through each of the devices, one at a time.
             */
            device = allDevices;
            while (device != NULL)
            {
                printf(
                    "%d: %s\n\tDescription: %s\n\tFlags: 0x%08x\n",
                    ii++,
                    device->name,
                    ((device->description != NULL) ?
                        device->description :
                        "No description available"),
                    device->flags);
                deviceAddr = device->addresses;
                printf("\tAddresses:\n"):
                if (deviceAddr != NULL)
                {
                    printf("\tAddresses:\n"):
                    while (deviceAddr != NULL)
                    {
                        printf(
                            "\t\taddr: Family: 0x%04x\n",
                            deviceAddr->addr->sa_family);
                        if (deviceAddr->netmask != NULL)
                            printf(
                                "\t\tnetmask: Family: 0x%04x\n",
                                deviceAddr->netmask->sa_family);
                        if (deviceAddr->broadaddr != NULL)
                            printf(
                                "\t\tbroadaddr: Family: 0x%04x\n",
                                deviceAddr->broadaddr->sa_family);
                        if (deviceAddr->dstaddr != NULL)
                            printf(
                                "\t\tdstaddr: Family: 0x%04x\n",
                                deviceAddr->dstaddr->sa_family);
                        deviceAddr = deviceAddr->next;
                    }
                }
                else
                    printf("\t\tNone.\n");
                device = device->next
            }

            /*
             * OK, we are all done with the devices returned by WinPcap.  Free
             * up the memory the interface allocated.
             */
            pcap_freealldevs(allDevices);
        }
        else
            printf("No interfaces found!  Make sure WinPcap is installed.\n");
    }
    else
        printf("Error returned by pcap_findalldevs_ex; %s\n", errorBuf);

    /*
     * Return from the main program.
     */
    return(0);
}