#ifndef _YUSTACK_INETDEVICE_H
#define _YUSTACK_INETDEVICE_H

#include "types.h"
#include "netdevice.h"

struct in_ifaddr {
	struct in_ifaddr *ifa_next;

	struct in_device *ifa_dev;

	uint32_t ifa_address;

	uint32_t ifa_mask;

	uint32_t ifa_broadcast;

	unsigned char ifa_scope;

	unsigned char ifa_prefixlen;
};

struct in_device {
	struct net_device *dev;

	struct in_ifaddr *ifa_list;
};

struct in_device *inetdev_init(struct net_device *dev);
struct in_ifaddr *inet_alloc_ifa(void);

struct in_device *in_dev_get(struct net_device *dev);
int inet_insert_ifa(struct net_device *dev, struct in_ifaddr *ifa);

#endif /* _YUSTACK_INETDEVICE_H */
