#ifndef _YUSTACK_IP_H
#define _YUSTACK_IP_H

#include "inet_sock.h"

struct ipcm_cookie {
	struct ip_options *opt;
};

int ip_append_data(struct sock *sk,
				int getfrag(void *from, char *to, int offset, int len,
						int odd, struct sk_buff *skb),
				void *from, int len, int protolen,
				struct ipcm_cookie *ipc, struct rtable *rt,unsigned int flags);
void ip_init(void);

#endif /* _YUSTACK_IP_H */
