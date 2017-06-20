#ifndef _YUSTACK_IN_H
#define _YUSTACK_IN_H

// Standard well-defined IP protocols
enum {
	IPPROTO_IP = 0,		// Dummy protocol for TCP
	IPPROTO_ICMP = 1,	// Internet Control Message Protocol
	IPPROTO_IGMP = 2,	// Internet Group Management Protocol
	IPPROTO_IPIP = 4,	// IPIP tunnels (older KA9Q tunnels use 94)
	IPPROTO_TCP = 6,	// Transmission Control Protocol
	IPPROTO_UDP = 17,	// User Datagram Protocol

	IPPROTO_IPV6	= 41,	// IPv6-in-IPv4 tunnelling

	IPPROTO_RAW = 255,		// Raw IP packets
};

#endif /* _YUSTACK_IN_H */
