#ifndef _YUSTACK_SOCK_H
#define _YUSTACK_SOCK_H

#include "skbuff.h"

struct sock {
	unsigned short sk_family;

	struct sk_buff_head	sk_write_queue;
};

struct sock *sk_alloc(int family);

#endif /* _YUSTACK_SOCK_H */
