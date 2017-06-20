#include "lib.h"
#include "types.h"
#include "icmp.h"
#include "skbuff.h"

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

// Driving logic for building and sending ICMP messages
void icmp_reply(struct icmp_bxm *icmp_param, struct sk_buff *skb) {

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
