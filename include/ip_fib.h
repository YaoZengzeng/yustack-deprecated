#ifndef _YUSTACK_IP_FIB_H
#define _YUSTACK_IP_FIB_H

#include "types.h"
#include "netdevice.h"

struct fib_nh {
	struct net_device *nh_dev;

	unsigned char nh_scope;

	int nh_oif;

	uint32_t nh_gw;
};

// This structure contains data shared by many of routes
struct fib_info {
	struct fib_nh fib_nh[0];
};

struct fib_result {
	unsigned char	prefixlen;

	unsigned char type;

	unsigned char scope;

	struct fib_info *fi;
};

struct fib_table {
	uint32_t	tb_id;

	int 	(*tb_lookup)(struct fib_table *tb, struct flowi *flp, struct fib_result *res);

	int		(*tb_insert)(struct fib_table * tb, struct fib_config *cfg);

	int 	(*tb_delete)(struct fib_table * tb, struct fib_config *cfg);

	unsigned char tb_data[0];
};

struct fib_alias {
	struct fib_alias *next;

	struct fib_info *fa_info;

	uint8_t fa_type;

	uint8_t fa_scope;
};

struct fib_node {
	struct fib_node *next;

	struct fib_alias *head;

	uint32_t fn_key;
};

struct fn_zone {
	struct fib_node *head;

	int fz_order;

	uint32_t fz_mask;
};

struct fn_hash {
	struct fn_zone	*fn_zones[33];
};

void ip_fib_init(void);
int fib_netdev_event(unsigned long event, void *ptr);

#endif /* _YUSTACK_IP_FIB_H */
