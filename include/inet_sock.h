#ifndef _YUSTACK_INET_SOCK_H
#define _YUSTACK_INET_SOCK_H

#include "sock.h"
#include "route.h"

struct ip_options {
	unsigned char optlen;
};

struct inet_sock {
	struct sock sk;

	uint16_t id;

	uint32_t daddr;

	uint32_t rcv_saddr;

	uint16_t dport;
	// Port number stored in host byte order
	uint16_t num;

	uint32_t saddr;
	// Port number stored in network byte order
	uint16_t sport;

	struct {
		struct rtable	*rt;

		struct flowi fl;
	} cork;
};

#define inet_sk(sk) ((struct inet_sock *)(sk))

#endif /* _YUSTACK_INET_SOCK_H */
