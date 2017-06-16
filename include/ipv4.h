#ifndef _YUSTACK_IPV4_H
#define _YUSTACK_IPV4_H

struct sk_buff;
struct net_device;
struct packet_type;

int inet_init(void);
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);

#endif /* _YUSTACK_IPV4_H */
