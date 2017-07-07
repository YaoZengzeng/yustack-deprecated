#include "lib.h"
#include "types.h"
#include "ip_fib.h"
#include "notifier.h"
#include "rtnetlink.h"
#include "inetdevice.h"

struct fib_table *ip_fib_local_table;
struct fib_table *ip_fib_main_table;

void ip_fib_init(void) {
	ip_fib_local_table = fib_hash_init(RT_TABLE_LOCAL);
	ip_fib_main_table = fib_hash_init(RT_TABLE_MAIN);
}

struct fib_table *fib_get_table(uint32_t id) {
	if (id != RT_TABLE_LOCAL) {
		return ip_fib_main_table;
	}
	return ip_fib_local_table;
}

struct fib_table *fib_new_table(uint32_t id) {
	return fib_get_table(id);
}

void fib_magic(int cmd, int type, uint32_t dst, int dst_len, struct in_ifaddr *ifa) {
	struct fib_table *tb;

	struct fib_config cfg = {
		.fc_type = type,
		.fc_dst = dst,
		.fc_dst_len = dst_len,
		.fc_prefsrc = ifa->ifa_address,
		.fc_oif = ifa->ifa_dev->dev->ifindex,
	};

	if (type == RTN_UNICAST) {
		tb = fib_new_table(RT_TABLE_MAIN);
	} else {
		tb = fib_new_table(RT_TABLE_LOCAL);
	}

	if (tb == NULL) {
		printf("fib_magic: fib_new_table failed\n");
		return;
	}

	cfg.fc_table = tb->tb_id;

	if (type != RTN_LOCAL) {
		cfg.fc_scope = RT_SCOPE_LINK;
	} else {
		cfg.fc_scope = RT_SCOPE_HOST;
	}
	if (cmd == RTM_NEWROUTE) {
		tb->tb_insert(tb, &cfg);
	}
}

void fib_add_ifaddr(struct in_ifaddr *ifa) {
	struct in_device *in_dev = ifa->ifa_dev;
	struct net_device *dev = in_dev->dev;
	struct in_ifaddr *prim = ifa;
	uint32_t mask = ifa->ifa_mask;
	uint32_t addr = ifa->ifa_address;
	uint32_t prefix = ifa->ifa_address & mask;

	fib_magic(RTM_NEWROUTE, RTN_LOCAL, addr, 32, prim);

	fib_magic(RTM_NEWROUTE, RTN_UNICAST, prefix, ifa->ifa_prefixlen, prim);

	// Add network specific broadcasts, when it takes a sense
	if (ifa->ifa_prefixlen < 32) {
		fib_magic(RTM_NEWROUTE, RTN_BROADCAST, prefix, 32, prim);
		fib_magic(RTM_NEWROUTE, RTN_BROADCAST, prefix|~mask, 32, prim);
	}
}

int fib_netdev_event(unsigned long event, void *ptr) {
	struct net_device *dev = ptr;
	struct in_ifaddr *ifa = NULL;
	struct in_device *in_dev = in_dev_get(dev);

	if (in_dev == NULL) {
		printf("fib_netdev_event: in_dev_get failed\n");
		return -1;
	}

	switch(event) {
	case NETDEV_UP:
		for (ifa = in_dev->ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
			fib_add_ifaddr(ifa);
		}
		break;
	}

	return 0;
}

int fib_lookup(struct flowi *flp, struct fib_result *res) {
	if (ip_fib_local_table->tb_lookup(ip_fib_local_table, flp, res) &&
		ip_fib_main_table->tb_lookup(ip_fib_main_table, flp, res)) {
		return -1;
	}
	return 0;
}
