#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <arpa/inet.h>

void net_get_interface_addr(struct sockaddr *out);
int net_listener_new(struct sockaddr *saddr, int port);
int net_accept(int listener);
uint8_t net_receive_keypress(int client);

#endif
