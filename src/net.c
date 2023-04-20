#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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
        printf("%s\n", ifap->ifa_name);
        if(ifap->ifa_addr &&
           !strcmp(INTERFACE_NAME, ifap->ifa_name) &&
           ifap->ifa_addr->sa_family == AF_INET)
            break;
    } while((ifap = ifap->ifa_next));

    /* EVIL we use an alias of a different type to set the port */
    struct sockaddr_in *inaddr = (struct sockaddr_in *)ifap->ifa_addr;
    char buf[INET_ADDRSTRLEN + 1] = {0};
    inet_ntop(AF_INET, &inaddr->sin_addr, buf, sizeof(buf) - 1);
    printf("IP address is %s\n", buf);
    inaddr->sin_port = port;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(bind(sock, ifap->ifa_addr, sizeof(struct sockaddr_in)) < 0)
        perror("bind()");

    freeifaddrs(interfaces);

    if(listen(sock, 0) < 0)
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

uint8_t net_receive_keypress(int client)
{
    uint8_t keycode;
    if(recv(client, &keycode, 1, 0) != sizeof(keycode))
        perror("recv()");

    printf("KEYCODE IS %u\n", keycode & 0x7F);

    return keycode;
}
