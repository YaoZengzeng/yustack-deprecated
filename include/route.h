#ifndef _YUSTACK_ROUTE_H
#define _YUSTACK_ROUTE_H

#include "dst.h"
#include "types.h"
#include "skbuff.h"
#include "netdevice.h"

struct rtable {
	union {
		struct dst_entry	dst;
		struct rtable *rt_next;
	} u;

	uint32_t rt_gateway;
};

int ip_route_input(struct sk_buff*, uint32_t dst, uint32_t src, uint8_t tos, struct net_device *dev);

#endif /* _YUSTACK_ROUTE_H */
