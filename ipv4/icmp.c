#include "in.h"
#include "ip.h"
#include "lib.h"
#include "net.h"
#include "ipv4.h"
#include "types.h"
#include "icmp.h"
#include "route.h"
#include "socket.h"
#include "skbuff.h"
#include "ip_fib.h"
#include "if_ether.h"
#include "checksum.h"

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

struct socket *icmp_socket;

int icmp_glue_bits(void *from, char *to, int offset, int len, int odd,
			struct sk_buff *skb) {
	struct icmphdr *icmph;
	struct icmp_bxm *icmp_param = (struct icmp_bxm *)from;

	memcpy(to, icmp_param->skb->data, len);

	icmph = skb->h.icmph;
	memcpy(icmph, &(icmp_param->data.icmph), icmp_param->head_len);
	icmph->checksum = 0;
	icmph->checksum = checksum((uint16_t *)icmph, skb->len);
	skb_push(skb, icmp_param->head_len);

	return 0;
}

void icmp_push_reply(struct icmp_bxm *icmp_param,
			struct ipcm_cookie *ipc, struct rtable *rt) {
	struct sk_buff *skb;
	int ret;

	if (ip_append_data(icmp_socket->sk, icmp_glue_bits, icmp_param,
					icmp_param->data_len + icmp_param->head_len,
					icmp_param->head_len, ipc, rt, 0) < 0) {
		printf("icmp_push_reply: ip_append_data failed\n");
	 	return;
	} else if ((skb = skb_peek(&icmp_socket->sk->sk_write_queue)) != NULL){
		ip_push_pending_frames(icmp_socket->sk);
	}
}

// Driving logic for building and sending ICMP messages
void icmp_reply(struct icmp_bxm *icmp_param, struct sk_buff *skb) {
	struct ipcm_cookie ipc;
	struct rtable *rt = (struct rtable *)skb->dst;
	uint32_t daddr, saddr;
	int r;

	daddr = rt->rt_src;
	saddr = rt->rt_dst;
	struct flowi fl = { .nl_u = { .ip4_u = {
			.daddr = daddr,
			.saddr = saddr,
		} },
	};

	r = ip_route_output_key(&rt, &fl);
	if (r < 0) {
		printf("icmp_reply: ip_route_output_key failed\n");
		return -1;
	}
	ipc.opt = NULL;

	icmp_push_reply(icmp_param, &ipc, rt);
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

	err = sock_create(PF_INET, SOCK_RAW, IPPROTO_ICMP, &icmp_socket);

	if (err < 0) {
		printf("Failed to create the ICMP control socket\n");
	}
}
