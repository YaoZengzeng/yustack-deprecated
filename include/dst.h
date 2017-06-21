#ifndef _YUSTACK_DST_H
#define _YUSTACK_DST_H

struct sk_buff;
struct net_device;

struct dst_entry {
	struct net_device *dev;

	int (*input)(struct sk_buff *);
	int (*output)(struct sk_buff *);
};

int dst_output(struct sk_buff *skb);
int dst_input(struct sk_buff *skb);

#endif /* _YUSTACK_DST_H */
