#include "lib.h"
#include "dst.h"
#include "ipv4.h"
#include "sock.h"
#include "skbuff.h"
#include "route.h"
#include "if_ether.h"
#include "inet_sock.h"
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

// ip_append_data() can make one large IP datagram from many pieces of data
// Each pieces will be holded on the socket until ip_push_pending_frames() is
// called. Each piece can be a page or non-page data.
//
// Not only UDP, other transport protocols - e.g. raw sockets - can use this
// interface opentially.
int ip_append_data(struct sock *sk,
			int getfrag(void *from, char *to, int offset, int len,
					int odd, struct sk_buff *skb),
			void *from, int length, int transhdrlen,
			struct ipcm_cookie *ipc, struct rtable *rt,
			unsigned int flags) {
	int hh_len;
	int copy, offset = 0;
	struct sk_buff *skb;
	struct inet_sock *inet = inet_sk(sk);
	struct ip_options *opt = NULL;
	unsigned int maxfraglen, fragheaderlen;
	unsigned int datalen, alloclen, fraglen, fraggap;

	hh_len = LL_RESERVED_SPACE(rt->u.dst.dev);

	inet->cork.rt = rt;

	fragheaderlen = sizeof(struct iphdr) + (opt ? opt->optlen : 0);

	fraggap = 0;
	datalen = length + fraggap;
	alloclen = length + fragheaderlen;
	fraglen = length + fragheaderlen;

	skb = alloc_skb(alloclen + hh_len);
	if (skb == NULL) {
		printf("ip_append_data: alloc_skb failed\n");
		return -1;
	}

	skb_reserve(skb, hh_len);

	// Find where to start putting bytes
	char *data;
	data = skb_put(skb, fraglen);
	skb->nh.raw = data;
	skb_pull(skb, fragheaderlen);
	data += fragheaderlen;
	skb->h.raw = data;
	skb_pull(skb, transhdrlen);

	copy = datalen - transhdrlen - fraggap;
	if (copy > 0 && getfrag(from, data + transhdrlen, offset, copy, fraggap, skb) < 0) {
		printf("ip_append_data: getfrag failed\n");

	}

	// Put the packet on the pending queue.
	skb_queue_tail(&sk->sk_write_queue, skb);
	return 0;

error:
	kfree_skb(skb);
	return -1;
}

// Combine all pending IP fragments on the socket as one IP datagram
// and push them out
int ip_push_pending_frames(struct sock *sk) {
	struct sk_buff *skb;
	struct inet_sock *inet = inet_sk(sk);
	struct rtable *rt = inet->cork.rt;
	struct iphdr *iph;
	uint16_t df = 0;

	if ((skb = skb_dequeue(&sk->sk_write_queue)) == NULL) {
		printf("ip_push_pending_frames: skb_dequeue failed\n");
		return -1;
	}

	// Move skb->data to ip header from ext header
	if (skb->nh.raw < skb->data) {
		skb_push(skb, skb->data - skb->nh.raw);
	}

	iph = (struct iphdr *)skb->data;
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	iph->tot_len = htons(skb->len);
	iph->frag_off = df;
	iph->id = 0;
	iph->frag_off = htons(0x4000);
	iph->ttl = 64;
	iph->protocol = sk->sk_protocol;
	iph->saddr = rt->rt_src;
	iph->daddr = rt->rt_dst;
	iph->check = 0;
	iph->check = checksum((uint16_t *)iph, iph->ihl * 4);

	skb->dst = (struct dst_entry *)rt;

	// no route system, just walk around
	arp_bind_neighbour(skb->dst);

	printf("finish arp_bind_neighbour\n");

	dst_output(skb);

	return 0;
}

void ip_init(void) {
	ip_rt_init();
}

int ip_generic_getfrag(void *from, char *to, int offset, int len, int odd, struct sk_buff *skb) {
	struct iovec *iov = from;

	memcpy(to, iov->iov_base, len);

	return 0;
}