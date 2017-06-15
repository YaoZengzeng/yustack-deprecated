#include "lib.h"
#include "if_arp.h"
#include "if_ether.h"
#include "netdevice.h"

void ether_setup(struct net_device *dev) {
	dev->type = ARPHRD_ETHER;
	dev->hard_header_len = ETH_HLEN;
	dev->mtu = ETH_DATA_LEN;
	dev->addr_len = ETH_ALEN;

	memset(dev->broadcast, 0xFF, ETH_ALEN);
}

int ether_init_module(void) {
	struct net_device *dev;
	int ret;

	dev = alloc_netdev(0, "eth0", ether_setup);
	if (dev == NULL) {
		printf("ether_init_module: alloc_netdev failed\n");
		return -1;
	}

	ret = register_netdevice(dev);
	if (ret != 0) {
		printf("ether_init_module: register_netdev failed\n");
		return -1;
	}

	return 0;
}
