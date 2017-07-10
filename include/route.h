#ifndef _YUSTACK_ROUTE_H
#define _YUSTACK_ROUTE_H

#include "dst.h"
#include "types.h"
#include "skbuff.h"
#include "ip_fib.h"
#include "netdevice.h"

struct rtable {
	union {
		struct dst_entry	dst;
		struct rtable *rt_next;
	} u;

	uint32_t rt_dst;	// Path destination
	uint32_t rt_src;	// Path source

	uint32_t rt_gateway;
};

int ip_rt_init(void);
int ip_route_input(struct sk_buff*, uint32_t dst, uint32_t src, uint8_t tos, struct net_device *dev);

int ip_route_output_key(struct rtable **, struct flowi *flp);
#endif /* _YUSTACK_ROUTE_H */
