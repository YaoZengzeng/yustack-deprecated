#include "lib.h"
#include "dst.h"
#include "types.h"
#include "ipv4.h"
#include "skbuff.h"
#include "route.h"
#include "netdevice.h"

// NOTE. We drop all the packets that has local source addresses
// because every properly looped back packet must have correct
// destination attached by output routine.
//
// Such approach solves two big problems:
// 1. Not simplex devices are handled properly.
// 2. IP spoofing attempts are filtered with 100% of guarantee.
int ip_route_input_slow(struct sk_buff *skb, uint32_t daddr, uint32_t saddr,
					uint8_t tos, struct net_device *dev) {

	// just walk around
	skb->dst = (struct dst_entry *) malloc(sizeof(struct rtable));
	if (skb->dst == NULL) {
		printf("ip_route_input_slow: malloc failed\n");
		return -1;
	}
	skb->dst->dev = dev;
	skb->dst->input = ip_local_deliver;
	skb->dst->output = ip_output;

	return 0;
}

// Just walk around
int ip_route_output_slow(struct sk_buff *skb, uint32_t daddr) {
	struct rtable *rt;
	rt = (struct rtable *)malloc(sizeof(struct rtable));
	if (rt == NULL) {
		printf("ip_route_output_slow malloc failed\n");
		return -1;
	}
	rt->rt_gateway = daddr;

	skb->dst = (struct dst_entry *)rt;
	skb->dst->dev = skb->dev;
	skb->dst->input = ip_local_deliver;
	skb->dst->output = ip_output;

	arp_bind_neighbour(skb->dst);

	return 0;
}

int ip_route_input(struct sk_buff *skb, uint32_t daddr, uint32_t saddr,
			uint8_t tos, struct net_device *dev) {
	// cache lookup failed, call ip_route_input_slow() to find directly in route table
	return ip_route_input_slow(skb, daddr, saddr, tos, dev);
}