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

int ip_rt_bug(struct sk_buff *skb) {
	printf("ip_rt_bug: maybe you should not invoke this function in the datapath\n");

	return 0;
}

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

	// For example, to address 192.168.1.2/32
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
	rth->u.dst.output = ip_rt_bug;
	rth->rt_gateway = daddr;
	rth->rt_dst = daddr;
	rth->rt_src = saddr;
	rth->rt_iif = dev->ifindex;
	rth->rt_type = res.type;
	// In linux kernel, we assign every fields of fl in turn
	rth->fl = fl;

	skb->dst = (struct dst_entry *)rth;

	return 0;
}

int ip_route_input(struct sk_buff *skb, uint32_t daddr, uint32_t saddr,
			uint8_t tos, struct net_device *dev) {
	// Temporarily do not do routing cache lookup

	// cache lookup failed, call ip_route_input_slow() to find directly in route table
	return ip_route_input_slow(skb, daddr, saddr, tos, dev);
}

int __mkroute_output(struct rtable **result, struct fib_result *res,
				struct flowi *fl, struct net_device *dev_out, unsigned flags) {
	struct rtable *rth;
	struct in_device *in_dev = NULL;

	// If the destination address is loopback, the output device should be loopback device
	// if (LOOPBACK(fl->fl4_src) && !(dev_out->flags & IFF_LOOPBACK)) {
	//	return -1;
	// }

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

	// rth->fl.fl4_dst = oldflp->fl4_dst;
	// rth->fl.fl4_tos = tos;
	// rth->fl.fl4_src = oldflp->fl4_src;
	// rth->fl.mark = oldflp->mark;
	rth->fl = *fl;
	rth->rt_dst = fl->fl4_dst;
	rth->rt_src = fl->fl4_src;
	rth->fl.oif = fl->oif;
	rth->rt_type = res->type;
	// Get references to the devices that are to be hold by the routing
	// cache entry
	rth->u.dst.dev = dev_out;
	rth->rt_gateway = fl->fl4_dst;
	rth->u.dst.output = ip_output;

	*result = rth;

	return 0;
}

int ip_mkroute_output_def(struct rtable **rp, struct fib_result *res,
				struct flowi *fl, struct net_device *dev_out, unsigned flags) {
	struct rtable *rth = NULL;
	int err = __mkroute_output(&rth, res, fl, dev_out, flags);
	if (err != 0) {
		printf("ip_mkroute_output_def: __mkroute_output failed\n");
	}

	// Try to bind route to arp only if it is output
	// route or unicast forwarding path
	if (rth->rt_type == RTN_UNICAST || rth->fl.iif == 0) {
		err = arp_bind_neighbour(&(rth->u.dst));
		if (err != 0) {
			printf("ip_mkroute_output_def: arp_bind_neighbour failed\n");
			return -1;
		}
	}
	*rp = rth;

	// Don't save route cache temporarily

	return 0;
}

int ip_mkroute_output(struct rtable **rp, struct fib_result *res,
				struct flowi *fl, struct net_device *dev_out, unsigned flags) {

	return ip_mkroute_output_def(rp, res, fl, dev_out, flags);
}

// Major route resolver routine
int ip_route_output_slow(struct rtable **rp, struct flowi *fl) {
	struct fib_result res;
	unsigned flags = 0;
	struct net_device *dev_out = NULL;
	int err;

	// Ignore source address validation
	// if (oldflp->fl4_src) {
	// }

	// If we have already know the output device
	// if (oldflp->oif) {
	// }

	// If we don't know the destination address
	// if (!fl.fl4_dst) {
	// }

	if (fib_lookup(fl, &res)) {
		printf("ip_route_output_slow: fib_lookup failed\n");
		return -1;
	}

	// destination address is local address
	// if (res.type == RTN_LOCAL) {
	// }

	// default route
	// if (!res.prefixlen && res.type == RTN_UNICAST && !fl.oif) {
	// 	fib_select_default(&fl, &res);
	// }

	dev_out = FIB_RES_DEV(res);
	if (dev_out == NULL) {
		printf("ip_route_output_slow: FIB_RES_DEV return NULL\n");
		return -1;
	}
	fl->oif = dev_out->ifindex;

	if (!fl->fl4_src) {
		fl->fl4_src = FIB_RES_PREFSRC(res);
	}

	err = ip_mkroute_output(rp, &res, fl, dev_out, flags);

	return err;
}

int ip_route_output_key(struct rtable **rp, struct flowi *flp) {
	// Temporarily do not do routing cache lookup

	return ip_route_output_slow(rp, flp);
}

int ip_route_output_flow(struct rtable **rp, struct flowi *flp, struct sock *sk, int flags) {
	int err;

	if ((err = ip_route_output_key(rp, flp)) != 0) {
		printf("ip_route_output_flow: ip_route_output_key failed\n");
		return -1;
	}

	return 0;
}

int ip_rt_init(void) {
	int rc = 0;

	ip_fib_init();
}
