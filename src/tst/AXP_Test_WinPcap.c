#include "AXP_Utility.h"
#include <netdb.h>
#include <pcap.h>

/*
 * Function Prototypes
 */
char *ip2s(u32);
char *ip62s(struct sockaddr *, char *, u32);

/*
 * interfacePrint
 *  Print all the available information on the given interface
 */
void interfacePrint(pcap_addr_t *deviceAddr)
{
    pcap_addr_t         *tmpDevAddr;
    struct sockaddr_in  *addr;
    char                ip6str[128];

    /*
     * IP addresses
     */
    tmpDevAddr = deviceAddr;
    while (tmpDevAddr != NULL)
    {
        addr = (struct sockaddr_in *) tmpDevAddr->addr;
        printf("\t\tAddress Family: #%d\n", addr->sin_family);

        /*
         * Dump out the address information, based upon the family.
         */
        switch(addr->sin_family)
        {
            case AF_INET:
                printf("\t\t\tAddress Family Name: AF_INET\n");
                printf("\t\t\tAddress: %s\n", ip2s(addr->sin_addr.s_addr));
                if (tmpDevAddr->netmask != NULL)
                {
                    addr = (struct sockaddr_in *) tmpDevAddr->netmask;
                    printf("\t\t\tNetmask: %s\n", ip2s(addr->sin_addr.s_addr));
                }
                if (tmpDevAddr->broadaddr != NULL)
                {
                    addr = (struct sockaddr_in *) tmpDevAddr->broadaddr;
                    printf(
                        "\t\t\tBroadcast Address: %s\n",
                        ip2s(addr->sin_addr.s_addr));
                }
                if (tmpDevAddr->dstaddr != NULL)
                {
                    addr = (struct sockaddr_in *) tmpDevAddr->dstaddr;
                    printf(
                        "\t\t\tDestination Address: %s\n",
                        ip2s(addr->sin_addr.s_addr));
                }
                break;

            case AF_INET6:
                printf("\t\t\tAddress Family Name: AF_INET6\n");
                printf(
                    "\t\t\tAddress: %s\n",
                    ip62s(tmpDevAddr->addr, ip6str, sizeof(ip6str)));
                break;

            default:
                printf("\t\t\tAddress Family Name: Unknown\n");
                break;
        }

        /*
         * Go to the next address in the list, if any.
         */
        tmpDevAddr = tmpDevAddr->next;
    }
    printf("\n");

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * ip2s
 *  From tcp tracert, convert a numeric IP address to a string
 */
#define IPTOSBUFFERS    12
char *ip2s(u32 in)
{
    static char     output[IPTOSBUFFERS][16];
    static short    which;
    u8              *ptr;

    ptr = (u8 *) &in;
    which = (((which + 1) == IPTOSBUFFERS) ? 0 : (which + 1));
    snprintf(
        output[which],
        sizeof(output[which]),
        "%d.%d.%d.%d",
        ptr[0],
        ptr[1],
        ptr[2],
        ptr[3]);

    /*
     * Return the string back to the caller.
     */
    return(output[which]);
}

/*
 * ip62s
 */
char *ip62s(struct sockaddr *sockaddr, char *address, u32 addrlen)
{
    socklen_t sockaddrlen;

    sockaddrlen = sizeof(struct sockaddr_storage);

    /*
     * Get the name information from the sock address supplied on the call.
     */
    if (getnameinfo(
            sockaddr, 
            sockaddrlen, 
            address, 
            addrlen, 
            NULL, 
            0, 
            NI_NUMERICHOST) != 0)
    {
        strcpy(address, "<Error Getting IPv6 hostname>");
    }

    /*
     * Return the string back to the caller.
     */
    return(address);
}

/*
 * main
 */
int main(void)
{
    pcap_if_t   *allDevices = NULL, *device;
    char        errorBuf[PCAP_ERRBUF_SIZE];
    int         retVal, ii = 1;

    /*
     * Call WinPcap to return all network devices that can be opened.
     */
    retVal = pcap_findalldevs(&allDevices, errorBuf);

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
                    "%d: %s\n\tDescription: %s\n\tLoopback: %s\n",
                    ii++,
                    device->name,
                    ((device->description != NULL) ?
                        device->description :
                        "No description available"),
                    ((device->flags & PCAP_IF_LOOPBACK) != 0) ? "Yes" : "No");
                printf("\tAddresses:\n");
                if (device->addresses != NULL)
                    interfacePrint(device->addresses);
                else
                    printf("\t\tNone.\n");
                device = device->next;
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
