#include "in.h"
#include "ip.h"
#include "lib.h"
#include "arp.h"
#include "net.h"
#include "raw.h"
#include "udp.h"
#include "ipv4.h"
#include "icmp.h"
#include "sock.h"
#include "socket.h"
#include "if_ether.h"
#include "protocol.h"
#include "netdevice.h"

struct packet_type ip_packet_type = {
	.type = htons(ETH_P_IP),
	.func = ip_rcv,
};

struct net_protocol icmp_protocol = {
	.protocol = IPPROTO_ICMP,
	.handler = icmp_rcv,
};

struct net_protocol udp_protocol = {
	.protocol = IPPROTO_UDP,
	.handler = udp_rcv,
};

struct proto_ops inet_dgram_ops = {
	.family = PF_INET,
};

struct inet_protosw inetsw_array[] = {
	{
		.type = SOCK_RAW,
		.protocol = IPPROTO_IP,		// wild card
		.prot = &raw_prot,
	},

	{
		.type = SOCK_DGRAM,
		.protocol = IPPROTO_UDP,
		.prot = &udp_prot,
		.ops = &inet_dgram_ops,
	}
};

struct inet_protosw *inetsw; 

#define INETSW_ARRAY_LEN (sizeof(inetsw_array)/sizeof(struct inet_protosw))

// Create an inet socket
int inet_create(struct socket *sock, int protocol) {
	struct sock *sk;
	struct proto *answer_prot;
	struct inet_protosw *answer;
	struct inet_sock *inet;
	int i;

	answer = NULL;

	for (i = 0; i < INETSW_ARRAY_LEN; i++) {
		answer = inetsw_array + i;
		if (protocol == answer->protocol) {
			break;
		}
		if (IPPROTO_IP == answer->protocol) {
			break;
		}
		answer = NULL;
	}
	sock->ops = answer->ops;

	if (answer == NULL) {
		printf("inet_create: answer is NULL\n");
	}
	answer_prot = answer->prot;

	sk = sk_alloc(PF_INET, answer_prot);

	sk->sk_family = PF_INET;
	sk->sk_protocol = protocol;

	inet = inet_sk(sk);

	inet->id = 0;

	sock_init_data(sock, sk);

	return 0;
}

struct net_proto_family inet_family_ops = {
	.family = PF_INET,
	.create = inet_create,
};

void inet_register_protosw(struct inet_protosw *p) {
	p->next = inetsw;
	inetsw = p;

	return;
}

int inet_init(void) {
	int rc = -1;
	struct inet_protosw *q;

	// Tell SOCKET that we are alive...
	sock_register(&inet_family_ops);

	// Set the ARP module up
	arp_init();

	// Add all the base protocols
	if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0) {
		printf("inet_init: add ICMP protocol failed\n");
	}
	if (inet_add_protocol(&udp_protocol, IPPROTO_UDP) < 0) {
		printf("inet_init: add UDP protocol failed\n");
	}

	inetsw = NULL;
	for (q = inetsw_array; q < &inetsw_array[INETSW_ARRAY_LEN]; q++) {
		inet_register_protosw(q);
	}


	// Set the IP module up
	ip_init();

	icmp_init();

	dev_add_pack(&ip_packet_type);

	rc = 0;
out:
	return rc;
}