#include "lib.h"
#include "arp.h"
#include "ipv4.h"
#include "if_ether.h"
#include "netdevice.h"

struct packet_type ip_packet_type = {
	.type = htons(ETH_P_IP),
	.func = ip_rcv,
};

int inet_init(void) {
	int rc = -1;

	// Set the ARP module up
	arp_init();

	// Set the IP module up
	//ip_init();

	dev_add_pack(&ip_packet_type);

	rc = 0;
out:
	return rc;
}