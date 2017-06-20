#include "in.h"
#include "lib.h"
#include "arp.h"
#include "ipv4.h"
#include "icmp.h"
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

int inet_init(void) {
	int rc = -1;

	// Set the ARP module up
	arp_init();

	// Add all the base protocols
	if (inet_add_protocol(&icmp_protocol, IPPROTO_ICMP) < 0) {
		printf("inet_init: add ICMP protocol failed\n");
	}
	// Set the IP module up
	//ip_init();

	dev_add_pack(&ip_packet_type);

	rc = 0;
out:
	return rc;
}