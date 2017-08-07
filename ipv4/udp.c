#include "lib.h"
#include "udp.h"
#include "ipv4.h"
#include "sock.h"
#include "skbuff.h"

struct proto udp_prot = {
	.name = "UDP",
	.obj_size = sizeof(struct udp_sock),
};

// UDP is nearly always wildcards out of the wazoo, it makes no sense to try
// harder than this
struct sock * __udp4_lib_lookup(uint32_t saddr, uint16_t sport,
					uint32_t daddr, uint16_t dport) {
	struct sock *sk, *result = NULL;
	unsigned short hnum = ntohs(dport);


	return result;
}

// returns:
//	-1: error
//	 0: success
//	>0: "udp encap" protocol resubmission
//
// Note that in the success and error cases, the skb is assumed to
// have either been requested or freed
int udp_queue_rcv_skb(struct sock *sk, struct sk_buff *skb) {

	printf("udp_queue_rcv_skb not implemented yet\n");

	return 0;
}

// All we need to do is get the socket, and then do a checksum
int __udp4_lib_rcv(struct sk_buff *skb) {
	struct sock *sk;
	struct udphdr *uh = skb->h.uh;
	uint32_t saddr = skb->nh.iph->saddr;
	uint32_t daddr = skb->nh.iph->daddr;

	// Validate the packet
	if (!skb_may_pull(skb, sizeof(struct udphdr))) {
		goto drop;
	}

	// udp4_csum_init(skb, uh);

	sk = __udp4_lib_lookup(saddr, uh->source, daddr, uh->dest);

	if (sk != NULL) {
		int ret = udp_queue_rcv_skb(sk, skb);

		if (ret > 0) {
			return -ret;
		}

		return ret;
	}

drop:
	kfree_skb(skb);
	return 0;
}

int udp_rcv(struct sk_buff *skb) {
	printf("udp_rcv get an packet\n");
	return __udp4_lib_rcv(skb);
}
