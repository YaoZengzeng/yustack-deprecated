#ifndef _YUSTACK_IF_ARP_H
#define _YUSTACK_IF_ARP_H

#include "types.h"

// ARP protocol HARDWARE identifiers
#define ARPHRD_ETHER	1	// Ethernet

// ARP protocol opcodes
#define ARPOP_REQUEST	1	// ARP request
#define ARPOP_REPLY	2	// ARP reply

struct arphdr {
	uint16_t	ar_hrd;	// Format of hardware address
	uint16_t	ar_pro;	// Format of protocol address

	unsigned char	ar_hln;	// Length of hardware address
	unsigned char	ar_pln;	// Length of protocol address

	uint16_t	ar_op;	// ARP opcode (command)

	// Ethernet looks like this:
/*	unsigned char	ar_sha[ETH_ALEN];	// Sender hardware address
	unsigned char	ar_sip[4];	// Sender IP address
	unsigned char 	ar_tha[ETH_ALEN];	// Target hardware address
	unsigned char	ar_tip[4];	// Target IP address
	*/
};

#endif /* _YUSTACK_IF_ARP_H */
