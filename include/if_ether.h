#ifndef _YUSTACK_IF_ETHER_H
#define _YUSTACK_IF_ETHER_H

// IEEE 802.3 Ethernet magic constants. The frame sizes omit the preamble
// and FCS/CRC (frame check sequence).
#define ETH_ALEN	6		// Octets in one ethernet addr
#define ETH_HLEN	14		// Total octets in header
#define ETH_DATA_LEN	1500	// Max. octets in payload

#endif /* _YUSTACK_IF_ETHER_H */
