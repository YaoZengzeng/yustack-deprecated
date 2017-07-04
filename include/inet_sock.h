#ifndef _YUSTACK_INET_SOCK_H
#define _YUSTACK_INET_SOCK_H

#include "sock.h"
#include "route.h"

struct ip_options {
	unsigned char optlen;
};

struct inet_sock {
	struct sock sk;

	struct {
		struct rtable	*rt;
	} cork;
};

#define inet_sk(sk) ((struct inet_sock *)(sk))

#endif /* _YUSTACK_INET_SOCK_H */
