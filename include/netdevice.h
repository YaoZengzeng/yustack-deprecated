#ifndef _YUSTACK_NETDEVICE_H
#define _YUSTACK_NETDEVICE_H

#include "if.h"
#include "types.h"

#define MAX_ADDR_LEN	32		// Largest hardware address length

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

	//int 	(*hard_start_xmit) (struct sk_buff *skb, struct net_device *dev);

	// The device initialization function. Called only once
	int 	(*init)(struct net_device *dev);
	// Called after device is detached from network
	void	(*uninit)(struct net_device *dev);
	// Called after last user reference disappears
	void 	(*destructor)(struct net_device *dev);

	int 	(*open)(struct net_device *dev);
	int 	(*stop)(struct net_device *dev);
};

struct net_device *alloc_netdev(int sizeof_priv, const char *name,
							void (*setup)(struct net_device *));
int register_netdevice(struct net_device *dev);

#endif /* _YUSTACK_NETDEVICE_H */
