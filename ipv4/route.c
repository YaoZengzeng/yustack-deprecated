#include "lib.h"
#include "dst.h"
#include "types.h"
#include "ipv4.h"
#include "sock.h"
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
		printf("ip_route_input_slow: malloc struct rtable failed\n");
		return -1;
	}
	rth->u.dst.dev = dev;
	rth->u.dst.input = ip_local_deliver;
	rth->u.dst.output = ip_output;
	rth->rt_dst = daddr;
	rth->rt_src = saddr;

	skb->dst = (struct dst_entry *)rth;

	return 0;
}

int ip_route_input(struct sk_buff *skb, uint32_t daddr, uint32_t saddr,
			uint8_t tos, struct net_device *dev) {
	// cache lookup failed, call ip_route_input_slow() to find directly in route table
	return ip_route_input_slow(skb, daddr, saddr, tos, dev);
}

int __mkroute_output(struct rtable **result, struct fib_result *res,
				struct flowi *fl, struct flowi *oldflp,
				struct net_device *dev_out, unsigned flags) {
	struct rtable *rth;
	struct in_device *in_dev = NULL;

	// Get work reference to inet device
	in_dev = in_dev_get(dev_out);
	if (in_dev == NULL) {
		printf("__mkroute_output: in_dev_get failed\n");
		return -1;
	}

	rth = (struct rtable *) malloc(sizeof(struct rtable));
	if (rth == NULL) {
		printf("__mkroute_output: malloc struct rtable failed\n");
		return -1;
	}

	rth->rt_dst = fl->fl4_dst;
	rth->rt_src = fl->fl4_src;
	// Get references to the devices that are to be hold by the routing
	// cache entry
	rth->u.dst.dev = dev_out;
	rth->rt_gateway = fl->fl4_dst;
	rth->u.dst.output = ip_output;

	*result = rth;

	return 0;
}

int ip_mkroute_output_def(struct rtable **rp, struct fib_result *res,
				struct flowi *fl, struct flowi *oldflp,
				struct net_device *dev_out, unsigned flags) {
	struct rtable *rth = NULL;
	int err = __mkroute_output(&rth, res, fl, oldflp, dev_out, flags);
	if (err == 0) {
		err = arp_bind_neighbour(&(rth->u.dst));
		*rp = rth;
	} else {
		printf("ip_mkroute_output_def: __mkroute_output failed\n");
	}

	return err;
}

int ip_mkroute_output(struct rtable **rp, struct fib_result *res,
				struct flowi *fl, struct flowi *oldflp,
				struct net_device *dev_out, unsigned flags) {

	return ip_mkroute_output_def(rp, res, fl, oldflp, dev_out, flags);
}

// Major route resolver routine
int ip_route_output_slow(struct rtable **rp, struct flowi *oldflp) {
	struct flowi fl = { .nl_u = { .ip4_u = {
			.daddr = oldflp->fl4_dst,
			.saddr = oldflp->fl4_src,
			.scope = RT_SCOPE_UNIVERSE,
		}}
	};
	struct fib_result res;
	unsigned flags = 0;
	struct net_device *dev_out = NULL;
	int err;

	if (fib_lookup(&fl, &res)) {
		printf("ip_route_output_slow: fib_lookup failed\n");
		return -1;
	}
	dev_out = FIB_RES_DEV(res);
	if (dev_out == NULL) {
		printf("ip_route_output_slow: FIB_RES_DEV return NULL\n");
		return -1;
	}

	if (!fl.fl4_src) {
		fl.fl4_src = FIB_RES_PREFSRC(res);
	}

	err = ip_mkroute_output(rp, &res, &fl, oldflp, dev_out, flags);

	return err;
}

int __ip_route_output_key(struct rtable **rp, struct flowi *flp) {
	return ip_route_output_slow(rp, flp);
}

int ip_route_output_flow(struct rtable **rp, struct flowi *flp, struct sock *sk, int flags) {
	int err;

	if ((err = __ip_route_output_key(rp, flp)) != 0) {
		return err;
	}

	return 0;
}

int ip_route_output_key(struct rtable **rp, struct flowi *flp) {
	return ip_route_output_flow(rp, flp, NULL, 0);
}

int ip_rt_init(void) {
	int rc = 0;

	ip_fib_init();
}
