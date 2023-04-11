#ifndef NET_H
#define NET_H

int net_listener_new(int port);
int net_accept(int listener);

#endif
