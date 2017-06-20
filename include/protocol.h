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

#endif /* _YUSTACK_PROTOCOL_H */
