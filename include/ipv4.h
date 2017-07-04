#ifndef _YUSTACK_IPV4_H
#define _YUSTACK_IPV4_H

#include "lib.h"

struct iphdr {
	uint8_t	ihl:4,
		version:4;
	uint8_t tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t frag_off;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
	// The options start here
};

struct sk_buff;
struct net_device;
struct packet_type;

int inet_init(void);
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);
int ip_local_deliver(struct sk_buff *skb);
int ip_output(struct sk_buff *skb);

// Some random defines to make things easier
#define LOOPBACK(x) (((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define MULTICAST(x) (((x) & htonl(0xf0000000)) == htonl(0xe0000000))

#endif /* _YUSTACK_IPV4_H */
