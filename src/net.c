#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>

#include "net.h"

#define INTERFACE_NAME "wlan0"

int net_listener_new(int port)
{
    struct ifaddrs *interfaces;
    if(getifaddrs(&interfaces) < 0)
        perror("getifaddrs()");

    struct ifaddrs *ifap = interfaces;
    do {
        if(!strcmp(INTERFACE_NAME, ifap->ifa_name) &&
           ifap->ifa_addr->sa_family == AF_INET)
            break;
    } while((ifap = ifap->ifa_next));

    /* EVIL we use an alias of a different type to set the port */
    struct sockaddr_in *inaddr = (struct sockaddr_in *)ifap->ifa_addr;
    inaddr->sin_port = port;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(sock, ifap->ifa_addr, sizeof(struct sockaddr_in)) < 0)
        perror("bind()");

    freeifaddrs(interfaces);

    if(listen(sock, 1) < 0)
        perror("listen()");

    return sock;
}

int net_accept(int listener)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client = accept(listener,
                        (struct sockaddr *)&client_addr,
                        &client_addr_len);
    if(client < 0)
        perror("accept()");

    return client;
}
