#ifndef _YUSTACK_NETDEVICE_H
#define _YUSTACK_NETDEVICE_H

#include "if.h"
#include "types.h"

#define MAX_ADDR_LEN	32		// Largest hardware address length

#define NET_RX_SUCCESS	0
#define	NET_RX_DROP		1

struct sk_buff;

struct net_device {
	char name[IFNAMSIZ];

	int ifindex;

	struct net_device *next;

	unsigned char 	state;

	unsigned mtu;

	// Interface hardware type
	unsigned short	type;
	// Hardware hdr len
	unsigned short 	hard_header_len;

	// Protocol specific pointers
	void	*ip_ptr;

	// Interface address info used in eth_type_trans()
	unsigned char 	addr_len;	// hardware length addresses
	unsigned char	dev_addr[MAX_ADDR_LEN];
	unsigned char 	broadcast[MAX_ADDR_LEN];

	int 	(*hard_start_xmit) (struct sk_buff *skb, struct net_device *dev);

	// for ehternet is eth_header()
	int 	(*hard_header)(struct sk_buff *skb, struct net_device *dev,
						unsigned short type, void *daddr, void *saddr, unsigned len);
};

struct packet_type {
	uint16_t type;
	struct net_device *dev;
	int (*func) (struct sk_buff *, struct net_device*, struct packet_type *);
	struct packet_type *next;
};

struct net_device *alloc_netdev(int sizeof_priv, const char *name,
							void (*setup)(struct net_device *));
int register_netdevice(struct net_device *dev);
int config_netdevice(struct net_device *dev);

int netif_rx(struct sk_buff *skb);
int netif_receive_skb(struct sk_buff *skb);

void dev_add_pack(struct packet_type *pt);

int dev_queue_xmit(struct sk_buff *skb);
int dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);

struct net_device *dev_get_by_index(int ifindex);

#define LL_RESERVED_SPACE(dev) ((dev)->hard_header_len);

#endif /* _YUSTACK_NETDEVICE_H */
