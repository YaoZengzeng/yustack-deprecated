#ifndef _YUSTACK_NET_H
#define _YUSTACK_NET_H

#include "sock.h"
#include "socket.h"

#define NPROTO	32

enum sock_type {
	SOCK_STREAM	= 1,
	SOCK_DGRAM	= 2,
	SOCK_RAW	= 3,
};

#define SOCK_MAX (SOCK_RAW + 1)

struct socket {
	struct proto_ops	*ops;

	struct sock *sk;

	short 		type;
};

struct proto_ops {
	int		family;
	int		(*sendmsg) (struct socket *sock, struct msghdr *m, int total_len);
	int 	(*recvmsg) (struct socket *sock, struct msghdr *m, int total_len, int flags);
	int 	(*bind) (struct socket *sock, struct sockaddr *uaddr, int sockaddr_len);
};

struct net_proto_family {
	int family;

	int (*create)(struct socket *sock, int protocol);
};

int sock_create(int family, int type, int proto, struct socket **res);
int sock_register(struct net_proto_family *fam);

#endif /* _YUSTACK_NET_H */
