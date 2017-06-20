#include "lib.h"
#include "ipv4.h"
#include "skbuff.h"
#include "protocol.h"
#include "if_packet.h"
#include "netdevice.h"

extern struct net_protocol *inet_protos;

int ip_local_deliver_finish(struct sk_buff *skb) {
	int ret = 0;
	int ihl = skb->nh.iph->ihl * 4;

	skb_pull(skb, ihl);

	// Point into the IP datagram, just past the header
	skb->h.raw = skb->data;

	int protocol = skb->nh.iph->protocol;
	struct net_protocol *ipprot = inet_protos;

	while(ipprot != NULL) {
		if (protocol == ipprot->protocol) {
			ret = ipprot->handler(skb);
			break;
		}
	}

	return ret;
}

// Deliver IP Packets to the higher protocol layers
int ip_local_deliver(struct sk_buff *skb) {
	// Reassemble IP fragments
	// ...

	return ip_local_deliver_finish(skb);
}

// Input packet from network to transport
int dst_input(struct sk_buff *skb) {
	return skb->dst->input(skb);
}

int ip_rcv_finish(struct sk_buff *skb) {
	struct iphdr *iph = skb->nh.iph;

	// Initialise the virtual paht cache for the packet. It describes
	// how the packet travels inside Linux networking
	if (skb->dst == NULL) {
		int err = ip_route_input(skb, iph->daddr, iph->saddr, iph->tos,
					skb->dev);
		if (err != 0) {
			printf("ip_rcv_finish: ip_route_input failed\n");
			goto drop;
		}
	}

/*	if (iph->ihl > 5 && ip_rcv_options(skb)) {
		printf("ip_rcv_finish: ip_rcv_options failed\n");
		goto drop;
	}*/

	return dst_input(skb);

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

// Main IP Receive routine
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt) {
	struct iphdr *iph;
	uint32_t len;

	// When the interface is in promisc mode, drop all the crap
	// that it receives, do not try to analyse it.
	if (skb->pkt_type == PACKET_OTHERHOST) {
		goto drop;
	}

	if (!skb_may_pull(skb, sizeof(struct iphdr))) {
		printf("ip_rcv: skb is too small, < sizeof(struct iphdr)\n");
		goto drop;
	}

	iph = skb->nh.iph;

	// RFC1122: 3.1.2.2 MUST silently discard any IP frame that fails the checksum
	// Is the datagram acceptable?
	//
	// 1. Length at least the size of an ip header
	// 2. Version of 4
	// 3. Checksums correctly
	// 4. Doesn't have a bogus length
	if (iph->ihl < 5 || iph->version != 4) {
		printf("ip_rcv: iph->ihl < 5 or iph->version != 4\n");
		goto drop;
	}

	if (!skb_may_pull(skb, iph->ihl * 4)) {
		printf("ip_rcv: skb is too small, < iph->ihl * 4\n");
		goto drop;
	}

/*
	if (ip_fast_csum((uint8_t *)iph, iph->ihl)) {
		printf("ip_rcv: checksum failed\n");
		goto drop;
	}*/

	len = ntohs(iph->tot_len);
	if (skb->len < len || len < (iph->ihl * 4)) {
		printf("ip_rcv: length of skb is small than packet size, \
			or packet size is small than head size\n");
		goto drop;
	}

	return ip_rcv_finish(skb);

drop:
	kfree_skb(skb);
out:
	return NET_RX_DROP;
}
