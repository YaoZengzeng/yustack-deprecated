#ifndef _YUSTACK_ICMP_H
#define _YUSTACK_ICMP_H

#include "types.h"
#include "skbuff.h"
#include "inet_sock.h"

#define ICMP_ECHOREPLY	0	// Echo Reply
#define ICMP_ECHO 		8	// Echo Request
#define NR_ICMP_TYPES	18

struct icmphdr {
	uint8_t	type;
	uint8_t code;
	uint16_t checksum;
	union {
		struct {
			uint16_t id;
			uint16_t sequence;
		} echo;
		uint32_t gateway;
		struct {
			uint16_t __unused;
			uint16_t mtu;
		} frag;
	} un;
};

struct raw_sock {
	struct inet_sock inet;
};

void icmp_init(void);
int icmp_rcv(struct sk_buff *skb);

#endif /* _YUSTACK_ICMP_H */
