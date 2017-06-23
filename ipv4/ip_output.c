#include "lib.h"
#include "dst.h"
#include "skbuff.h"
#include "if_ether.h"
#include "netdevice.h"

int dst_output(struct sk_buff *skb) {
	return skb->dst->output(skb);
}

int ip_finish_output2(struct sk_buff *skb) {
	struct dst_entry *dst = skb->dst;
	struct net_device *dev = skb->dst->dev;

	if (dst->neighbour) {
		dst->neighbour->output(skb);
	} else {
		printf("ip_finish_output2: dst->neighbour is NULL\n");
		return -1;
	}

	return 0;
}

int ip_finish_output(struct sk_buff *skb) {
	return ip_finish_output2(skb);
}

int ip_output(struct sk_buff *skb) {
	struct net_device *dev = skb->dst->dev;

	skb->dev = dev;
	skb->protocol = htons(ETH_P_IP);

	return ip_finish_output(skb);
}