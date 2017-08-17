#ifndef _YUSTACK_SOCK_H
#define _YUSTACK_SOCK_H

#include "skbuff.h"
#include "net.h"

struct sock {
	unsigned short sk_family;

	unsigned char sk_protocol;

	struct sk_buff_head	sk_write_queue;

	struct proto 	*sk_prot;
};

// Networking protocol blocks we attach to sockets
// socket layer -> transport layer interface
// transport -> network interface is defined by struct inet_proto
struct proto {
	char name[32];

	unsigned int obj_size;

	int 	(*sendmsg)(struct sock *sk, struct msghdr *msg, int len);

	int 	(*get_port)(struct sock *sk, unsigned short snum);
};

struct sock *sk_alloc(int family, struct proto *prot);

// Initialization core socket variables
void sock_init_data(struct socket *sock, struct sock *sk);

#endif /* _YUSTACK_SOCK_H */
