#include "if.h"
#include "lib.h"
#include "skbuff.h"
#include "notifier.h"
#include "netdevice.h"
#include "inetdevice.h"

struct net_device *dev_base;
struct packet_type *ptype_base;

struct net_device *alloc_netdev(int sizeof_priv, const char *name,
	void (*setup)(struct net_device *))
{
	struct net_device *dev;

	dev = (struct net_device*)malloc(sizeof(*dev));
	if (dev == NULL) {
		printf("alloc_netdev: malloc failed\n");
		return NULL;
	}

	memset(dev, 0, sizeof(*dev));

	setup(dev);
	strcpy(dev->name, name);

	return dev;
}

int dev_new_index(void) {
	static int ifindex;

	return ifindex++;
}

struct net_device *dev_get_by_index(int ifindex) {
	struct net_device *dev = dev_base;

	while(dev) {
		if (dev->ifindex == ifindex) {
			return dev;
		}
	}

	return NULL;
}

int register_netdevice(struct net_device *dev) {
	int ret;

	if (dev->init) {
		ret = dev->init(dev);
		if (ret) {
			goto out;
		}
	}

	dev->ifindex = dev_new_index();

	dev->next = dev_base;
	dev_base = dev;

	ret = 0;

out:
	return ret;
}

int config_netdevice(struct net_device *dev) {
	struct in_ifaddr *ifa = NULL;

	dev->ip_ptr = inetdev_init(dev);
	if (dev->ip_ptr == NULL) {
		return -1;
	}

	ifa = inet_alloc_ifa();
	if (ifa == NULL) {
		return -1;
	}

	ifa->ifa_address = FIXED_IP_ADDR;
	ifa->ifa_mask = FIXED_NETMASK;
	ifa->ifa_prefixlen = FIXED_IP_PREFIXLEN;
	ifa->ifa_broadcast = ifa->ifa_address | ~ifa->ifa_mask;

	return inet_insert_ifa(dev, ifa);
}



int netif_rx(struct sk_buff *skb) {
	return netif_receive_skb(skb);
}

int netif_receive_skb(struct sk_buff *skb) {
	struct packet_type *ptype;
	uint16_t type;
	int ret = NET_RX_DROP;

	skb->h.raw = skb->nh.raw = skb->data;
	skb->mac_len = skb->nh.raw - skb->mac.raw;

	ptype = ptype_base;
	type = skb->protocol;
	while(ptype) {
		if (ptype->type == type && 
			(!ptype->dev || ptype->dev == skb->dev)) {
			ptype->func(skb, skb->dev, ptype);
			ret = NET_RX_SUCCESS;
			break;
		}
		ptype = ptype->next;
	}

	if (ret == NET_RX_DROP) {
		kfree_skb(skb);
	}

	return ret;
}

void dev_add_pack(struct packet_type *pt) {
	pt->next = ptype_base;
	ptype_base = pt;
}

// dev_queue_xmit - transmit a buffer
int dev_queue_xmit(struct sk_buff *skb) {
	struct net_device *dev = skb->dev;

	return dev_hard_start_xmit(skb, dev);
}

int dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev) {
	return dev->hard_start_xmit(skb, dev);
}