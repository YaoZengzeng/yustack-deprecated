#include "lib.h"
#include "dst.h"
#include "types.h"
#include "ipv4.h"
#include "skbuff.h"
#include "route.h"
#include "ip_fib.h"
#include "rtnetlink.h"
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
	struct flowi fl = {
		.nl_u = { .ip4_u =
			{
				.daddr = daddr,
				.saddr = saddr,
				.tos = tos,
			}
		},
		.iif = dev->ifindex,
	};
	int err = -1;
	struct rtable *rth;
	struct fib_result res;

	// Now we are ready to route packet
	if ((err = fib_lookup(&fl, &res)) != 0) {
		printf("ip_route_input_slow: fib_lookup　failed\n");
		return -1;
	}

	if (res.type == RTN_LOCAL) {
		goto local_input;
	} else {
		printf("res.type != RTN_LOCAL, crash!\n");
		return -1;
	}

local_input:
	rth = (struct rtable *) malloc(sizeof(struct rtable));
	if (rth == NULL) {
		printf("ip_route_input_slow failed\n");
		return -1;
	}
	rth->u.dst.dev = dev;
	rth->u.dst.input = ip_local_deliver;
	rth->u.dst.output = ip_output;

	skb->dst = (struct dst_entry *)rth;

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

int ip_rt_init(void) {
	int rc = 0;

	ip_fib_init();
}
