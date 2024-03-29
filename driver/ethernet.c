#include "lib.h"
#include "ip_fib.h"
#include "if_arp.h"
#include "if_ether.h"
#include "notifier.h"
#include "if_packet.h"
#include "skbuff.h"
#include "netdevice.h"

extern int tapfd;

// Create the Ethernet header
int eth_header(struct sk_buff *skb, struct net_device *dev, unsigned short type,
		void *daddr, void *saddr, unsigned len) {
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);

	eth->h_proto = htons(type);

	// Set the source hardware address
	if (!saddr) {
		// NULL use device source address
		saddr = dev->dev_addr;
	}
	memcpy(eth->h_source, saddr, dev->addr_len);

	if (daddr) {
		memcpy(eth->h_dest, daddr, dev->addr_len);
		return ETH_HLEN;
	}

	// Anyway, the loopback-device should never use this function
/*	if (dev->flags & (IFF_LOOPBACK | IF_NOARP)) {
		memset(eth->h_dest, 0, dev->addr_len);
	}*/

	return -1;
}

int ether_tx(struct sk_buff *skb, struct net_device *dev) {
	int len;

	len = write(tapfd, skb->data, skb->len);
	if (len != skb->len) {
		printf("ether_tx failed\n");
		return -1;
	}

	return 0;
}

void ether_setup(struct net_device *dev) {
	dev->type = ARPHRD_ETHER;
	dev->hard_header_len = ETH_HLEN;
	dev->mtu = ETH_DATA_LEN;
	dev->addr_len = ETH_ALEN;
	dev->hard_header = eth_header;
	dev->hard_start_xmit = ether_tx;

	memcpy(dev->dev_addr, "\0YUSTK", ETH_ALEN);
	memset(dev->broadcast, 0xFF, ETH_ALEN);
}

int ether_init_module(void) {
	struct net_device *dev;
	int ret;

	// alloc net_device structure and initialize
	dev = alloc_netdev(0, "eth0", ether_setup);
	if (dev == NULL) {
		printf("ether_init_module: alloc_netdev failed\n");
		return -1;
	}

	// put dev to dev_base list
	ret = register_netdevice(dev);
	if (ret != 0) {
		printf("ether_init_module: register_netdev failed\n");
		return -1;
	}

	// Hard code IP config
	ret = config_netdevice(dev);
	if (ret != 0) {
		printf("ether_init_module: config_netdevice failed\n");
		return -1;
	}

	// This should be placed in dev_open, now for simplicity
	ret = fib_netdev_event(NETDEV_UP, dev);
	if (ret != 0) {
		printf("ether_init_module: fib_netdev_event failed\n");
		return -1;
	}

	return 0;
}

struct ethhdr *eth_hdr(struct sk_buff *skb) {
	return (struct ethhdr *)skb->mac.raw;
}

// is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
// Return true if the address is a multicast address.
// By definition the broadcast address is also a multicast address.
int is_multicast_ether_addr(uint8_t *addr) {
	return (0x01 & addr[0]);
}

int is_broadcast_ether_addr(uint8_t *addr) {
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff;
}

// compare_ether_addr - Compare two Ethernet addresses
// Compare two ethernet addresses, returns 0 if equal
unsigned compare_ether_addr(uint8_t *addr1, uint8_t *addr2) {
	uint16_t *a = (uint16_t *) addr1;
	uint16_t *b = (uint16_t *) addr2;

	return (a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]);
}

uint16_t eth_type_trans(struct sk_buff *skb, struct net_device *dev) {
	struct ethhdr *eth;

	skb->mac.raw = skb->data;
	skb_pull(skb, ETH_HLEN);
	eth = eth_hdr(skb);

	// default: skb->pkt_type = PACKET_HOST = 0
	if (is_multicast_ether_addr(eth->h_dest)) {
		if (!compare_ether_addr(eth->h_dest, dev->broadcast)) {
			skb->pkt_type = PACKET_BROADCAST;
			printf("eth_type_trans: a packet for broadcast\n");
		} else {
			skb->pkt_type = PACKET_MULTICAST;
			printf("eth_type_trans: a packet for multicast\n");
		}
	} else if (compare_ether_addr(eth->h_dest, dev->dev_addr)) {
		skb->pkt_type = PACKET_OTHERHOST;
		printf("eth_type_trans: a packet for other host\n");
	}
	printf("eth_type_trans: a packet for host\n");

	return eth->h_proto;
}

void ether_rx(struct net_device *dev) {
	struct sk_buff *skb;
	int len;
	char buff[ETH_HLEN + ETH_DATA_LEN];

	len = read(tapfd, buff, ETH_HLEN + ETH_DATA_LEN);
	if (len < ETH_HLEN) {
		printf("ether_rx receive packet failed, too small\n");
		return;
	}

	// head = data = tail, end, len = 0
	skb = dev_alloc_skb(len);
	if (skb == NULL) {
		printf("ether_rx : dev_alloc_skb failed\n");
		return;
	}

	// head = data, tail = end, len = len
	memcpy(skb_put(skb, len), buff, len);

	skb->dev = dev;
	// data point to the head of IP layer
	skb->protocol = eth_type_trans(skb, dev);

	if (DEBUG) {
		printf("packet protocol is %x\n", skb->protocol);
	}

	netif_rx(skb);
}
