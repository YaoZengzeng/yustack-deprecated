#include "lib.h"
#include "netdevice.h"

struct net_device *dev_base;

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
