#ifndef _YUSTACK_NEIGHBOUR_H
#define _YUSTACK_NEIGHBOUR_H

#include "skbuff.h"
#include "types.h"
#include "netdevice.h"

#define NUD_NONE	0x00

#define NUD_INCOMPLETE 0x01

#define NUD_REACHABLE 0x02

struct neigh_table;
struct neigh_ops;

struct neighbour {
	struct neighbour *next;

	struct neigh_table *tbl;

	struct net_device *dev;

	uint8_t	nud_state;

	unsigned char ha[MAX_ADDR_LEN];

	//struct hh_cache *hh;

	int (*output)(struct sk_buff *skb);

	struct sk_buff_head arp_queue;

	struct neigh_ops *ops;

	uint8_t primary_key[0];
};

struct neigh_ops {
	int family;

	void (*solicit)(struct neighbour *, struct sk_buff *);

	void (*error_report)(struct neighbour *, struct sk_buff *);

	int (*output)(struct sk_buff *);

	int (*connected_output)(struct sk_buff *);

	//int (*hh_output)(struct sk_buff *);

	int (*queue_xmit)(struct sk_buff *);
};

// Neighbour table manipulation
struct neigh_table {
	struct neigh_table *next;

	int family;

	int key_len;

	int (*constructor)(struct neighbour *);

	struct neighbour *buckets;

	char *id;
};

void neigh_table_init(struct neigh_table *tbl);
struct neighbour *neigh_event_ns(struct neigh_table *tbl, uint8_t *lladdr,
							void *saddr, struct net_device *dev);
int neigh_resolve_output(struct sk_buff *skb);

#endif /* _YUSTACK_NEIGHBOUR_H */
