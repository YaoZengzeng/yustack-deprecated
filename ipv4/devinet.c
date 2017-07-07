#include "lib.h"
#include "netdevice.h"
#include "inetdevice.h"

struct in_device *in_dev_get(struct net_device *dev) {
	struct in_device *in_dev = dev->ip_ptr;

	return in_dev;
}

int inet_insert_ifa(struct net_device *dev, struct in_ifaddr *ifa) {
	struct in_device *in_dev = in_dev_get(dev);
	if (in_dev == NULL) {
		printf("inet_insert failed: in_dev_get is NULL\n");
		return -1;
	}

	ifa->ifa_next = in_dev->ifa_list;
	ifa->ifa_dev = in_dev;
	in_dev->ifa_list = ifa;

	return 0;
}

struct in_device *inetdev_init(struct net_device *dev) {
	struct in_device *in_dev;

	in_dev = (struct in_device *)malloc(sizeof(struct in_device));
	if (in_dev == NULL) {
		printf("inetdev_init: malloc in_device failed\n");
		return NULL;
	}
	memset(in_dev, 0, sizeof(struct in_device));

	in_dev->dev = dev;

	return in_dev;
}

struct in_ifaddr *inet_alloc_ifa(void) {
	struct in_ifaddr *ifa = (struct in_ifaddr *)malloc(sizeof(struct in_ifaddr));
	if (ifa == NULL) {
		printf("inet_alloc_ifa: malloc ifa failed\n");
		return NULL;
	}
	memset(ifa, 0, sizeof(struct in_ifaddr));

	return ifa;
}