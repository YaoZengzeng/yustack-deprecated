#include "lib.h"
#include "dst.h"
#include "ipv4.h"
#include "route.h"
#include "skbuff.h"
#include "socket.h"
#include "if_arp.h"
#include "if_ether.h"
#include "if_packet.h"
#include "neighbour.h"
#include "netdevice.h"

void arp_solicit(struct neighbour *neigh, struct sk_buff *skb) {
	uint32_t saddr = 0;
	struct net_device *dev = neigh->dev;
	uint8_t *dst_ha = NULL;
	uint32_t target = *(uint32_t *)neigh->primary_key;

	saddr = skb->nh.iph->saddr;
	arp_send(ARPOP_REQUEST, ETH_P_ARP, target, dev, saddr,
		dst_ha, dev->dev_addr, NULL);
}

struct neigh_ops arp_generic_ops = {
	.family = AF_INET,
	.solicit = arp_solicit,
	//.error_report = arp_error_report,
	.output = neigh_resolve_output,
	//.connected_output = neigh_connected_output,
	//.hh_output = dev_queue_xmit,
	.queue_xmit = dev_queue_xmit,
};

int arp_constructor(struct neighbour *neigh) {

	neigh->ops = &arp_generic_ops;

	neigh->output = neigh->ops->output;

	return 0;
}

struct neigh_table arp_tbl = {
	.family = AF_INET,

	.key_len = 4,

	.constructor = arp_constructor,

	.id = "arp_cache",
};

// Create an arp packet. If (dest_hw == NULL)
// we create a broadcast message
struct sk_buff *arp_create(int type, int ptype, uint32_t dest_ip,
			struct net_device *dev, uint32_t src_ip,
			unsigned char *dest_hw, unsigned char *src_hw,
			unsigned char *target_hw) {

	struct sk_buff *skb;
	struct arphdr *arp;
	unsigned char *arp_ptr;

	// Allocate a buffer
	skb = alloc_skb(sizeof(struct arphdr) + 2 * (dev->addr_len + 4)
					+ dev->hard_header_len);
	if (skb == NULL) {
		return NULL;
	}

	skb_reserve(skb, dev->hard_header_len);
	skb->nh.raw = skb->data;
	arp = (struct arphdr *)skb_put(skb, sizeof(struct arphdr) + 2 * (dev->addr_len + 4));
	skb->dev = dev;
	skb->protocol = htons(ETH_P_ARP);
	if (src_hw == NULL) {
		src_hw = dev->dev_addr;
	}
	if (dest_hw == NULL) {
		dest_hw = dev->broadcast;
	}

	// Fill the device header for the ARP frame
	if (dev->hard_header &&
		dev->hard_header(skb, dev, ptype, dest_hw, src_hw, skb->len) < 0) {
		printf("arp_create: fill the device header failed\n");
		goto out;
	}

	arp->ar_hrd = htons(dev->type);
	arp->ar_pro = htons(ETH_P_IP);

	arp->ar_hln = dev->addr_len;
	arp->ar_pln = 4;
	arp->ar_op = htons(type);

	arp_ptr = (unsigned char *)(arp + 1);

	memcpy(arp_ptr, src_hw, dev->addr_len);
	arp_ptr += dev->addr_len;
	memcpy(arp_ptr, &src_ip, 4);
	arp_ptr += 4;
	if (target_hw != NULL) {
		memcpy(arp_ptr, target_hw, dev->addr_len);
	} else {
		memset(arp_ptr, 0, dev->addr_len);
	}
	arp_ptr += dev->addr_len;
	memcpy(arp_ptr, &dest_ip, 4);

	return skb;

out:
	kfree_skb(skb);
	return NULL;
}

// Send an arp request
void arp_xmit(struct sk_buff *skb) {
	dev_queue_xmit(skb);
}

// create and send an arp packet
void arp_send(int type, int ptype, uint32_t dest_ip,
		struct net_device *dev, uint32_t src_ip,
		unsigned char *dest_hw, unsigned char *src_hw,
		unsigned char *target_hw) {

	struct sk_buff *skb;

	skb = arp_create(type, ptype, dest_ip, dev, src_ip,
			dest_hw, src_hw, target_hw);
	if (skb == NULL) {
		printf("arp_send: arp_create failed\n");
		return;
	}

	arp_xmit(skb);
}

static int arp_process(struct sk_buff *skb) {
	struct net_device *dev = skb->dev;
	struct arphdr *arp;
	unsigned char *arp_ptr;
	unsigned char *sha, *tha;
	uint32_t sip, tip;
	uint16_t dev_type = dev->type;
	struct neighbour *n;

	arp = skb->nh.arph;

	if (arp->ar_pro != htons(ETH_P_IP)||
		htons(dev_type) != arp->ar_hrd) {
		printf("arp->ar_pro is %x, htons(ETH_P_IP) is %x\n", arp->ar_pro, htons(ETH_P_IP));
		printf("htons(dev_type) is %x, arp->ar_hrd is %x\n", htons(dev_type), arp->ar_hrd);
		printf("arp_process: protocol check failed\n");
		goto out;
	}

	// Understand only these message types
	if (arp->ar_op != htons(ARPOP_REPLY) &&
		arp->ar_op != htons(ARPOP_REQUEST)) {
		printf("arp_process: arp op code check failed\n");
		goto out;
	}

	// Extract fields
	arp_ptr = (unsigned char *)(arp + 1);
	sha = arp_ptr;
	arp_ptr += dev->addr_len;
	memcpy(&sip, arp_ptr, 4);
	arp_ptr += 4;
	tha = arp_ptr;
	arp_ptr += dev->addr_len;
	memcpy(&tip, arp_ptr, 4);

	// Check for bad requests for 127.x.x.x and requests for multicast
	// address. If this is one such, delete it.
	if (LOOPBACK(tip) || MULTICAST(tip)) {
		printf("arp_process: target ip is loopback or multicast\n");
		goto out;
	}

	if (arp->ar_op == htons(ARPOP_REQUEST)) {
		// before the code or route finished,
		// directly call arp_send by using dev->dev_addr
		n = neigh_event_ns(&arp_tbl, sha, &sip, dev);
		if (n) {
			arp_send(ARPOP_REPLY, ETH_P_ARP, sip, dev, tip, sha, dev->dev_addr, sha);
		} else {
			printf("arp_process: neigh_event_ns failed\n");
		}
		goto out;
	}

	// Update our ARP tables
	n = __neigh_lookup(&arp_tbl, &sip, dev, 0);

	if (n) {
		int state = NUD_REACHABLE;

		neigh_update(n, sha, state);
	}

out:
	kfree_skb(skb);
	return 0;
}

int arp_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt) {
	struct arphdr *arp;

	if (!skb_may_pull(skb, sizeof(struct arphdr) +
					(2 * dev->addr_len) +
					(2 * sizeof(uint32_t)))) {
		printf("arp_rcv: skb_may_pull failed\n");
		goto freeskb;
	}

	arp = skb->nh.arph;
	if (arp->ar_hln != dev->addr_len ||
		skb->pkt_type == PACKET_OTHERHOST ||
		skb->pkt_type == PACKET_LOOPBACK ||
		arp->ar_pln != 4) {
		printf("arp_rcv: sanity check failed\n");
		goto freeskb;
	}

	return arp_process(skb);

freeskb:
	kfree_skb(skb);
	return 0;
}

struct packet_type arp_packet_type = {
	.type = htons(ETH_P_ARP),
	.func = arp_rcv,
};

int arp_bind_neighbour(struct dst_entry *dst) {
	struct net_device *dev = dst->dev;
	struct neighbour *n = dst->neighbour;

	if (dev == NULL) {
		printf("arp_bind_neighbour: dst->dev is NULL\n");
		return -1;
	}

	if (n == NULL) {
		uint32_t nexthop = ((struct rtable *)dst)->rt_gateway;
		n = __neigh_lookup_errno(&arp_tbl, &nexthop, dev);
		if (n == NULL) {
			printf("arp_bind_neighbour: __neigh_lookup_errno failed\n");
			return -1;
		}
		dst->neighbour = n;
	}

	return 0;
}

void arp_init(void) {
	neigh_table_init(&arp_tbl);

	dev_add_pack(&arp_packet_type);
}
