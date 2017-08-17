#include "in.h"
#include "ip.h"
#include "lib.h"
#include "udp.h"
#include "ipv4.h"
#include "sock.h"
#include "socket.h"
#include "checksum.h"
#include "inet_sock.h"
#include "skbuff.h"

// Push out all pending data as one UDP datagram
int udp_push_pending_frames(struct sock *sk) {
	struct udp_sock *up = udp_sk(sk);
	struct inet_sock *inet = inet_sk(sk);
	//struct flowi *fl = &inet->cork.fl;
	struct sk_buff *skb;
	struct udphdr *uh;
	uint16_t csum;
	int err  = 0;

	// Grab the skbuff where UDP header space exists
	if ((skb = skb_peek(&sk->sk_write_queue)) == NULL) {
		printf("udp_push_pending_frames: skb_peek return NULL\n");
		return -1;
	}

	// Create a UDP header
	uh = skb->h.uh;
	// uh->source = fl->fl_ip_sport;
	// uh->dest = fl->fl_ip_dport;
	uh->source = inet->sport;
	uh->dest = inet->dport;
	uh->len = htons(up->len);
	uh->check = 0;

	csum = checksum((uint16_t *)(skb->h.raw), up->len);
	uh->check = csum_tcpudp_magic(inet->saddr, inet->daddr, uh->len,
						sk->sk_protocol, csum);

	err = ip_push_pending_frames(sk);
	if (err != 0) {
		printf("udp_push_pending_frames: ip_push_pending_frames failed\n");
		return -1;
	}

	return 0;
}

int udp_sendmsg(struct sock *sk, struct msghdr *msg, int len) {
	struct inet_sock *inet = inet_sk(sk);
	struct udp_sock	*up = udp_sk(sk);
	int ulen = len;
	struct ipcm_cookie ipc;
	struct rtable *rt = NULL;
	uint32_t daddr, saddr;
	uint16_t dport;
	int err;
	int (*getfrag)(void *, char *, int, int, int, struct sk_buff *);

	ulen += sizeof(struct udphdr);

	// Get and verify the address
	if (msg->msg_name) {
		struct sockaddr_in * usin = (struct sockaddr_in*)msg->msg_name;
		if (msg->msg_namelen < sizeof(*usin)) {
			printf("udp_sendmsg: msg->msg_namelen not long enough\n");
			return -1;
		}
		if (usin->sin_family != AF_INET) {
			printf("udp_sendmsg: usin->sin_family is not AF_INET\n");
			return -1;
		}

		daddr = usin->sin_addr.s_addr;
		dport = usin->sin_port;
		if (dport == 0) {
			printf("udp_sendmsg: usin->sin_port could not be 0\n");
			return -1;
		}
	}

	if (rt == NULL) {
		struct flowi fl = { .nl_u = { .ip4_u = {
				.daddr = daddr,
			} },
		};
		err = ip_route_output_flow(&rt, &fl, sk, 0);
		if (err != 0) {
			printf("udp_sendmsg: ip_route_output_flow failed\n");
			return -1;
		}
	}

	saddr = rt->rt_src;

	up->len = ulen;

	getfrag = ip_generic_getfrag;
	err = ip_append_data(sk, getfrag, msg->msg_iov, ulen, sizeof(struct udphdr),
				&ipc, rt, 0);
	if (err != 0) {
		printf("udp_sendmsg: ip_append_data failed\n");
		return -1;
	}

	inet->daddr = daddr;
	inet->saddr = saddr;
	inet->dport = dport;
	// inet->sport has already been configured

	err = udp_push_pending_frames(sk);
	if (err != 0) {
		printf("udp_sendmsg: udp_push_pending_frames failed\n");
		return -1;
	}

	return 0;
}

struct udp_sk_map {
	struct sock *sk;
	int used;
};

#define MAXUDPSOCKNUM 512

struct udp_sk_map udptable[MAXUDPSOCKNUM];

int udp_v4_get_port(struct sock *sk, unsigned short snum) {
	int port;

	if (snum == 0) {
		for (port = 0; port < MAXUDPSOCKNUM; port++) {
			if (udptable[port].used) {
				continue;
			}
			udptable[port].sk =sk;
			udptable[port].used = 1;
			break;
		}

		if (port == MAXUDPSOCKNUM) {
			printf("udp_v4_get_port: alloc port-sock failed\n");
			return -1;
		}
		snum = port;
	}

	inet_sk(sk)->num = snum;

	return 0;
}

struct proto udp_prot = {
	.name = "UDP",
	.obj_size = sizeof(struct udp_sock),
	.sendmsg  = udp_sendmsg,
	.get_port = udp_v4_get_port,
};

// UDP is nearly always wildcards out of the wazoo, it makes no sense to try
// harder than this
struct sock * __udp4_lib_lookup(uint32_t saddr, uint16_t sport,
					uint32_t daddr, uint16_t dport) {
	struct sock *sk, *result = NULL;
	unsigned short hnum = ntohs(dport);


	return result;
}

// return:
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
