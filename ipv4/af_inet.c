#include "in.h"
#include "lib.h"
#include "arp.h"
#include "net.h"
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

// Create an inet socket
int inet_create(struct socket *sock, int protocol) {
	struct sock *sk;

	sk = sk_alloc(PF_INET);
	return 0;
}

struct net_proto_family inet_family_ops = {
	.family = PF_INET,
	.create = inet_create,
};

int inet_init(void) {
	int rc = -1;

	// Tell SOCKET that we are alive...
	sock_register(&inet_family_ops);

	// Set the ARP module up
	arp_init();

	// Add all the base protocols
	if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0) {
		printf("inet_init: add ICMP protocol failed\n");
	}
	// Set the IP module up
	//ip_init();

	icmp_init();

	dev_add_pack(&ip_packet_type);

	rc = 0;
out:
	return rc;
}