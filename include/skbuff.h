#ifndef _YUSTACK_SKBUFF_H
#define _YUSTACK_SKBUFF_H

#include "types.h"

struct net_device;

#define NET_SKB_PAD 16

struct icmphdr;

struct iphdr;
struct arphdr;

struct dst_entry;

struct sk_buff {
	// These two members must be first
	struct sk_buff	*next;
	struct sk_buff	*prev;

	//struct sock	*sk;
	struct net_device *dev;

	union {
		struct icmphdr *icmph;
		unsigned char *raw;
	} h;

	union {
		struct iphdr  *iph;
		struct arphdr *arph;
		unsigned char *raw;
	} nh;

	union {
		unsigned char *raw;
	} mac;

	struct dst_entry	*dst;

	unsigned int len;
	unsigned int mac_len;

	uint16_t	protocol;
	uint16_t	pkt_type;

	void 	(*destructor)(struct sk_buff *skb);

	unsigned char *head;
	unsigned char *data;
	unsigned char *tail;
	unsigned char *end;
};

struct sk_buff_head {
	struct sk_buff *next;
	struct sk_buff *prev;

	uint32_t qlen;
};

struct sk_buff *alloc_skb(unsigned int size);
struct sk_buff *dev_alloc_skb(unsigned int size);
void kfree_skb(struct sk_buff *skb);
#define dev_kfree_skb(a) kfree_skb(a);

void skb_reserve(struct sk_buff *skb, int len);
unsigned char *skb_push(struct sk_buff *skb, unsigned int len);
unsigned char *skb_pull(struct sk_buff *skb, unsigned int len);
unsigned char *skb_put(struct sk_buff *skb, unsigned int len);

int skb_may_pull(struct sk_buff *skb, unsigned int len);

void skb_queue_head_init(struct sk_buff_head *list);
void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk);
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);

struct sk_buff *skb_peek(struct sk_buff_head *list);

#endif /* _YUSTACK_SKBUFF_H */
