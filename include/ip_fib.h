#ifndef _YUSTACK_IP_FIB_H
#define _YUSTACK_IP_FIB_H

#include "types.h"
#include "netdevice.h"

struct fib_config {
	uint8_t fc_dst_len;

	uint8_t fc_scope;

	uint8_t fc_type;

	uint32_t fc_table;

	uint32_t fc_dst;

	uint32_t fc_gw;

	int fc_oif;

	uint32_t fc_prefsrc;
};

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

	struct fib_alias *alias_list;

	uint32_t fn_key;
};

struct fn_zone {
	struct fib_node *node_list;

	int fz_order;

	uint32_t fz_mask;
};

struct fn_hash {
	struct fn_zone	*fn_zones[33];
};

struct flowi {
	int oif;
	int iif;

	union {
		struct {
			uint32_t 	daddr;
			uint32_t 	saddr;
			uint8_t 	tos;
			uint8_t		scope;
		} ip4_u;
	} nl_u;
	#define fl4_dst nl_u.ip4_u.daddr
	#define fl4_src nl_u.ip4_u.saddr
	#define fl4_tos	nl_u.ip4_u.tos
	#define fl4_scope nl_u.ip4_u.scope
	uint8_t proto;
	union {
		struct {
			uint16_t sport;
			uint16_t dport;
		} ports;
	} uli_u;
	#define fl_ip_sport uli_u.ports.sport
	#define fl_ip_dport uli_u.ports.dport
};

void ip_fib_init(void);
int fib_netdev_event(unsigned long event, void *ptr);

int fib_lookup(struct flowi *flp, struct fib_result *res);

#define FIB_RES_NH(res) ((res).fi->fib_nh[0])
#define FIB_RES_DEV(res) (FIB_RES_NH(res).nh_dev)
#endif /* _YUSTACK_IP_FIB_H */
