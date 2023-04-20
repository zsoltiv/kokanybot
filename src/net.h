#ifndef NET_H
#define NET_H

#include <stdint.h>

int net_listener_new(int port);
int net_accept(int listener);
uint8_t net_receive_keypress(int client);

#endif
