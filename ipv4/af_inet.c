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

// IP protocol layer initializer
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

// Automatically bind an unbound socket
int inet_autobind(struct sock *sk) {
	struct inet_sock *inet;

	// We may need to bind the socket
	inet = inet_sk(sk);
	if (!inet->num) {
		if (sk->sk_prot->get_port(sk, 0)) {
			printf("inet_autobind: get_port failed\n");
			return -1;
		}
		inet->sport = htons(inet->num);
	}

	return 0;
}

int inet_sendmsg(struct socket *sock, struct msghdr *msg, int size) {
	struct sock *sk = sock->sk;

	// We may need to bind the socket
	if (!inet_sk(sk)->num && inet_autobind(sk)) {
		printf("inet_sendmsg: inet_autobind failed\n");
		return -1;
	}

	return sk->sk_prot->sendmsg(sk, msg, size);
}

int inet_bind(struct socket *sock, struct sockaddr *uaddr, int addrlen) {
	struct sockaddr_in *addr = (struct sockaddr_in *)uaddr;
	struct sock *sk = sock->sk;
	struct inet_sock *inet = inet_sk(sk);
	unsigned short snum;

	// If the socket has its own bind function then use it. (RAW)
	// if (sk->sk_prot->bind) {
	//	err = sk->sk_prot->bind(sk, uaddr, addr_len);
	// }

	snum = ntohs(addr->sin_port);

	inet->rcv_saddr = inet->saddr = addr->sin_addr.s_addr;

	if (sk->sk_prot->get_port(sk, snum) != 0) {
		printf("inet_bind: sk->sk_prot->get_port failed\n");
		return -1;
	}

	return 0;
}

struct proto_ops inet_dgram_ops = {
	.family = PF_INET,
	.sendmsg = inet_sendmsg,
	.recvmsg = sock_common_recvmsg,
	.bind = inet_bind,
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

#define INETSW_ARRAY_LEN (sizeof(inetsw_array)/sizeof(struct inet_protosw))

// Create an inet socket
int inet_create(struct socket *sock, int protocol) {
	struct sock *sk;
	struct proto *answer_prot;
	struct inet_protosw *answer;
	struct inet_sock *inet;
	int i;

	// sock->state = SS_UNCONNECTED;

	answer = NULL;

	for (i = 0; i < INETSW_ARRAY_LEN; i++) {
		answer = inetsw_array + i;
		if (protocol == answer->protocol) {
			break;
		}
		answer = NULL;
	}

	// work around, IPPROTO_ICMP is same with IPPROTO_ICMP
	if (protocol == IPPROTO_ICMP) {
		answer = inetsw_array;
	}

	if (answer == NULL) {
		printf("inet_create: answer is NULL\n");
		return -1;
	}

	sock->ops = answer->ops;

	answer_prot = answer->prot;

	sk = sk_alloc(PF_INET, answer_prot);

	inet = inet_sk(sk);

	inet->id = 0;

	// Assign sk to sock and initialize sk->sk_write_queue and sk->sk_receive_queue
	sock_init_data(sock, sk);

	sk->sk_family = PF_INET;
	sk->sk_protocol = protocol;

	return 0;
}

// Used to create socket
struct net_proto_family inet_family_ops = {
	.family = PF_INET,
	.create = inet_create,
};

int inet_init(void) {
	int rc = -1;
	struct inet_protosw *q;

	// Tell SOCKET that we are alive...
	sock_register(&inet_family_ops);

	// Add all the base protocols
	if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0) {
		printf("inet_init: add ICMP protocol failed\n");
	}
	if (inet_add_protocol(&udp_protocol, IPPROTO_UDP) < 0) {
		printf("inet_init: add UDP protocol failed\n");
	}

	// Set the ARP module up
	arp_init();

	// Set the IP module up
	ip_init();

	// Set the ICMP layer up
	icmp_init();

	dev_add_pack(&ip_packet_type);

	rc = 0;
out:
	return rc;
}