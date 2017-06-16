#include "lib.h"
#include "skbuff.h"
#include "if_ether.h"
#include "netdevice.h"

int arp_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt) {
	printf("arp_rcv not implemented yet\n");

	return 0;
}

struct packet_type arp_packet_type = {
	.type = htons(ETH_P_ARP),
	.func = arp_rcv,
};

void arp_init(void) {
	dev_add_pack(&arp_packet_type);
}