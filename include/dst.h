#ifndef _YUSTACK_DST_H
#define _YUSTACK_DST_H

#include "neighbour.h"

struct sk_buff;
struct net_device;

struct dst_entry {
	struct net_device *dev;

	struct neighbour *neighbour;

	int (*input)(struct sk_buff *);
	int (*output)(struct sk_buff *);
};

int dst_output(struct sk_buff *skb);
int dst_input(struct sk_buff *skb);

#endif /* _YUSTACK_DST_H */
