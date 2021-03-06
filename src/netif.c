/*
  Example code to obtain IP and MAC for all available interfaces on Linux.
  by Adam Pierce <adam@doctort.org>

http://www.doctort.org/adam/

*/

#include "netif.h"

int enum_if(void)
{
        char          buf[1024];
        struct ifconf ifc;
        struct ifreq *ifr;
        int           sck;
        int           nInterfaces;
        int           i;

/* Get a socket handle. */
        sck = socket(AF_INET, SOCK_DGRAM, 0);
        if(sck < 0)
        {
                //perror("socket");
                return 1;
        }

/* Query available interfaces. */
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
        {
                //perror("ioctl(SIOCGIFCONF)");
                return 2;
        }

/* Iterate through the list of interfaces. */
        ifr         = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
        for(i = 0; i < nInterfaces; i++)
        {

/* Query available interfaces. */
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
        {
                //perror("ioctl(SIOCGIFCONF)");
                return 3;
        }

        /* Iterate through the list of interfaces. */
        ifr         = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
        for(i = 0; i < nInterfaces; i++)
        {
                struct ifreq *item = &ifr[i];

            /* Show the device name and IP address */
            printf("%s: IP %s",
                   item->ifr_name,
                   inet_ntoa(((struct sockaddr_in *)&item->ifr_addr)->sin_addr));

            /* Get the MAC address */
            if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
            {
                    //perror("ioctl(SIOCGIFHWADDR)");
                    return 4;
            }
            printf(", MAC %02X:%02X:%02X:%02X:%02X:%02X",
                        (unsigned char)item->ifr_hwaddr.sa_data[0],
                        (unsigned char)item->ifr_hwaddr.sa_data[1],
                        (unsigned char)item->ifr_hwaddr.sa_data[2],
                        (unsigned char)item->ifr_hwaddr.sa_data[3],
                        (unsigned char)item->ifr_hwaddr.sa_data[4],
                        (unsigned char)item->ifr_hwaddr.sa_data[5]);


        /* Get the broadcast address (added by Eric) */
                if(ioctl(sck, SIOCGIFBRDADDR, item) >= 0)
                        printf(", BROADCAST %s", inet_ntoa(((struct sockaddr_in *)&item->ifr_broadaddr)->sin_addr));
                printf("\n");
        }
    }
    return 0;
}


int first_if_mac(char* sn, size_t len)
{
    char          buf[1024];
    struct ifconf ifc;
    struct ifreq *ifr;
    int           sck;
    int           nInterfaces;
    int           i;

    /* Get a socket handle. */
    sck = socket(AF_INET, SOCK_DGRAM, 0);
    if(sck < 0) return -1;

    /* Query available interfaces. */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if(ioctl(sck, SIOCGIFCONF, &ifc) < 0) return -2;

    /* Iterate through the list of interfaces. */
    ifr         = ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    for(i = 0; i < nInterfaces; i++) {
        /* Query available interfaces. */
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if(ioctl(sck, SIOCGIFCONF, &ifc) < 0) return -3;

        /* Iterate through the list of interfaces. */
        ifr         = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
        for(i = 0; i < nInterfaces; i++)
        {
            struct ifreq *item = &ifr[i];
            /* Get the MAC address */
            if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
                return -4;
            if ((unsigned char)item->ifr_hwaddr.sa_data[0] +
                (unsigned char)item->ifr_hwaddr.sa_data[1] +
                (unsigned char)item->ifr_hwaddr.sa_data[2] +
                (unsigned char)item->ifr_hwaddr.sa_data[3] +
                (unsigned char)item->ifr_hwaddr.sa_data[4] +
                (unsigned char)item->ifr_hwaddr.sa_data[5] > 0) {
                    snprintf(sn, len, "%02X:%02X:%02X:%02X:%02X:%02X",
                                (unsigned char)item->ifr_hwaddr.sa_data[0],
                                (unsigned char)item->ifr_hwaddr.sa_data[1],
                                (unsigned char)item->ifr_hwaddr.sa_data[2],
                                (unsigned char)item->ifr_hwaddr.sa_data[3],
                                (unsigned char)item->ifr_hwaddr.sa_data[4],
                                (unsigned char)item->ifr_hwaddr.sa_data[5]);
                    return (0);
            }
        }
    }

    return 0;
}
