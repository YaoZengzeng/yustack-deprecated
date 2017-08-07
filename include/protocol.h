#ifndef _YUSTACK_PROTOCOL_H
#define _YUSTACK_PROTOCOL_H

#include "types.h"
#include "skbuff.h"

#define MAX_INET_PROTOS 256	// Must be a power of 2

// This is used to register protocols
struct net_protocol {
	struct net_protocol *next;
	uint8_t protocol;
	int 	(*handler)(struct sk_buff *skb);
	void	(*err_handler)(struct sk_buff *skb, uint32_t info);
};

// This is used to register socket interfaces for IP protocols
struct inet_protosw {
	unsigned short	type;	// This is the 2nd argument to socket(2).
	unsigned short	protocol;	// This is the L4 protocol number

	struct proto 	*prot;

	struct proto_ops *ops;
};

#endif /* _YUSTACK_PROTOCOL_H */
