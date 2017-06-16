#include "lib.h"
#include "skbuff.h"
#include "netdevice.h"

// Main IP Receive routine
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt) {
	printf("ip_rcv not implemented yet\n");

	return 0;
}
