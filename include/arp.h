#ifndef _YUSTACK_ARP_H
#define _YUSTACK_ARP_H

struct dst_entry;

void arp_init(void);

int arp_bind_neighbour(struct dst_entry *dst);

#endif /* _YUSTACK_ARP_H */
