#ifndef _YUSTACK_UDP_H
#define _YUSTACK_UDP_H

#include "sock.h"
#include "types.h"
#include "protocol.h"
#include "inet_sock.h"

struct sk_buff;

struct udphdr {
	uint16_t	source;
	uint16_t	dest;
	uint16_t	len;
	uint16_t	check;
};

struct udp_sock {
	// inet_sock has to be the first member
	struct inet_sock inet;

	uint16_t len;	// total length of pending frames
};

int udp_rcv(struct sk_buff *skb);

extern struct proto udp_prot;

#define udp_sk(sk) ((struct udp_sock *)(sk))

#endif /* _YUSTACK_UDP_H */
