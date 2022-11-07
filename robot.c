#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>

#define PORT "1337"

int create_socket()
{
    /* TODO: ERROR CHECKING MINDENHOL */
    struct ifaddrs *interfaces, *wlan;
    getifaddrs(&interfaces);

    for(wlan = interfaces; wlan->ifa_next; wlan = wlan->ifa_next) {
        struct iwreq pwrq = {0};
        strncpy(pwrq.ifr_ifrn.ifrn_name, wlan->ifa_name, IFNAMSIZ);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        int ret = ioctl(sock, SIOCGIWNAME, &pwrq);
        close(sock);
        if(ret != -1) {
            break;
        }
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    bind(sock, wlan->ifa_addr, sizeof(*(wlan->ifa_addr)));
    listen(sock, 1);
    freeifaddrs(interfaces);

    return sock;
}

int main(void)
{
    //int sock = create_socket();
}
