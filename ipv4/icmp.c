#include "in.h"
#include "ip.h"
#include "lib.h"
#include "net.h"
#include "ipv4.h"
#include "types.h"
#include "icmp.h"
#include "socket.h"
#include "skbuff.h"
#include "if_ether.h"

// ICMP control array. This specifies what to do with each ICMP
struct icmp_control {
	int output_entry;	// Field for increment on output
	int input_entry;	// Field for increment on input
	void (*handler)(struct sk_buff *skb);
	short error;	// This ICMP is classed as an error message
};

// Build xmit assembly blocks
struct icmp_bxm {
	struct sk_buff *skb;

	int offset;
	int data_len;

	struct {
		struct icmphdr icmph;
		uint32_t times[3];
	} data;

	int head_len;

	// struct ip_options replyopts;
	unsigned char optbuf[40];
};

struct socket *icmp_sock;

uint16_t checksum(uint16_t* buff, int size) {
	uint32_t cksum = 0;

	while(size > 1) {
		cksum += *buff++;
		size -= 2;
	}
	if (size == 1) {
		cksum += htons(*(char *)buff << 8);
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (uint16_t)(~cksum);
}

void icmp_push_reply(struct icmp_bxm *icmp_param,
			struct ipcm_cookie *ipc, struct rtable *rt) {
	struct sk_buff *skb = icmp_param->skb;
	struct iphdr *iph;
	struct icmphdr *icmph;
	uint32_t t;
	char c;
	int i, ret;
	
	iph = skb->nh.iph;
	// Exchange source and dest IP address
	t = iph->saddr;
	iph->saddr = iph->daddr;
	iph->daddr = t;

	icmph = skb->h.icmph;
	icmph->type = ICMP_ECHOREPLY;
	skb_push(skb, sizeof(struct icmphdr));
	icmph->checksum = 0;
	icmph->checksum = checksum((uint16_t *)skb->data, skb->len);

	skb_push(skb, sizeof(struct iphdr));

	// Just walk around
	ret = ip_route_output_slow(skb, iph->daddr);
	if (ret != 0) {
		printf("icmp_push_reply: ip_route_output_slow failed\n");
		return;
	}

	dst_output(skb);
}

// Driving logic for building and sending ICMP messages
void icmp_reply(struct icmp_bxm *icmp_param, struct sk_buff *skb) {
	icmp_push_reply(icmp_param, NULL, NULL);
}

// Handle ICMP_ECHO ("ping") requests
//
// RFC 1122: 3.2.2.6 MUST have an echo server that answer ICMP echo
// requests
// RFC 1122: 3.2.2.6 Data received in the ICMP_ECHO request MUST be
// included in the reply
// RFC 1812: 4.3.3.6 SHOULD have a config option for silently ignoring
//		echo reqeusts, MUST have default=NOT
void icmp_echo(struct sk_buff *skb) {
	struct icmp_bxm icmp_param;

	icmp_param.data.icmph = *(skb->h.icmph);
	icmp_param.data.icmph.type = ICMP_ECHOREPLY;
	icmp_param.skb = skb;
	icmp_param.offset = 0;
	icmp_param.data_len = skb->len;
	icmp_param.head_len = sizeof(struct icmphdr);
	icmp_reply(&icmp_param, skb);
}

struct icmp_control icmp_pointers[NR_ICMP_TYPES + 1] = {
	[ICMP_ECHO] = {
		.handler = icmp_echo,
	},
};

// Deal with incoming ICMP packets
int icmp_rcv(struct sk_buff *skb) {
	struct icmphdr *icmph;

	if (skb_pull(skb, sizeof(struct icmphdr)) == NULL) {
		printf("icmp_rcv: skb_pull failed\n");
		return -1;
	}

	icmph = skb->h.icmph;

	// 18 is the highest 'known' ICMP type. Anything else is a mystery
	//
	// RFC 1122: 3.2.2 Unkown ICMP messages types MUST be silently discarded
	if (icmph->type > NR_ICMP_TYPES) {
		goto drop;
	}

	// Parse the ICMP message
	icmp_pointers[icmph->type].handler(skb);

drop:
	kfree_skb(skb);
	return 0;
}

void icmp_init() {
	int err;

	err = sock_create(PF_INET, SOCK_RAW, IPPROTO_ICMP, &icmp_sock);

	if (err < 0) {
		printf("Failed to create the ICMP control socket\n");
	}
}
