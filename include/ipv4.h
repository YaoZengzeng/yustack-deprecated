#ifndef _YUSTACK_IPV4_H
#define _YUSTACK_IPV4_H

#include "lib.h"

struct sk_buff;
struct net_device;
struct packet_type;

int inet_init(void);
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);

// Some random defines to make things easier
#define LOOPBACK(x) (((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define MULTICAST(x) (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

#endif /* _YUSTACK_IPV4_H */
