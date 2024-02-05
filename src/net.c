/*
 * copyright (c) 2023 Zsolt Vadasz
 *
 * This file is part of kokanybot.
 *
 * kokanybot is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * kokanybot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kokanybot. If not, see <https://www.gnu.org/licenses/>. 
*/

#define _XOPEN_SOURCE 700
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <errno.h>

#include "net.h"

#define INTERFACE_NAME "eth0"

static struct ifaddrs *net_get_interface_addr(void)
{
    struct ifaddrs *interfaces;
    if(getifaddrs(&interfaces) < 0) {
        perror("getifaddrs()");
        exit(1);
    }

    struct ifaddrs *ifap = interfaces;
    do {
        printf("%s\n", ifap->ifa_name);
        if(ifap->ifa_addr &&
           !strcmp(INTERFACE_NAME, ifap->ifa_name) &&
           ifap->ifa_addr->sa_family == AF_INET)
            break;
    } while((ifap = ifap->ifa_next));
    if(!ifap) {
        fprintf(stderr, "%s not found, exiting\n", INTERFACE_NAME);
        exit(1);
    }

    struct ifaddrs *interface = malloc(sizeof(struct ifaddrs));
    memcpy(interface, ifap, sizeof(struct ifaddrs));
    freeifaddrs(interfaces);

    return interface;
}

int net_accept(int port)
{
    struct ifaddrs *interface = net_get_interface_addr();
    struct sockaddr_in *iaddr = (struct sockaddr_in *)interface->ifa_addr;
    iaddr->sin_port = htons(port);
    int listener = socket(iaddr->sin_family, SOCK_STREAM, 0);
    if(listener < 0)
        perror("socket()");
    int yes = 1;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    if(bind(listener, (struct sockaddr *)iaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind()");
        exit(1);
    }
    if(listen(listener, 1) < 0) {
        perror("listen()");
        exit(1);
    }
    char addrstr[INET_ADDRSTRLEN] = {0};
    if(!inet_ntop(AF_INET, &iaddr->sin_addr, addrstr, sizeof(addrstr))) {
        perror("inet_ntop()");
        exit(1);
    }
    printf("Listening on %s:%u\n", addrstr, port);
    int client = accept(listener,
                        NULL,
                        NULL);
    if(client < 0) {
        perror("accept()");
        exit(1);
    }
    close(listener);
    free(interface);

    return client;
}

uint8_t net_receive_keypress(int client)
{
    uint8_t keycode;
    if(recv(client, &keycode, 1, 0) < sizeof(keycode)) {
        if(errno) {
            perror("recv()");
        }
    }

    return keycode;
}
