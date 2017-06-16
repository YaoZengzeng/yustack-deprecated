#ifndef _YUSTACK_IF_ETHER_H
#define _YUSTACK_IF_ETHER_H

#include "types.h"
#include "skbuff.h"

// IEEE 802.3 Ethernet magic constants. The frame sizes omit the preamble
// and FCS/CRC (frame check sequence).
#define ETH_ALEN	6		// Octets in one ethernet addr
#define ETH_HLEN	14		// Total octets in header
#define ETH_DATA_LEN	1500	// Max. octets in payload

// These are the defined Ethernet Protocol ID's
#define ETH_P_LOOP	0x0060		// Ethernet Loopback packet
#define ETH_P_IP	0x0800		// Internet Protocol packet
#define ETH_P_ARP	0x0806		// Address Resolution packet
#define ETH_P_IPV6	0x86DD		// IPv6 over bluebook

// This is an Ethernet frame header
struct ethhdr {
	unsigned char h_dest[ETH_ALEN];	// destination ether addr
	unsigned char h_source[ETH_ALEN]; // source ether addr
	uint16_t h_proto;	// packet type ID field
} __attribute__((packed));

struct ethhdr *eth_hdr(struct sk_buff *skb);

#endif /* _YUSTACK_IF_ETHER_H */
