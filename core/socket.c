#include "in.h"
#include "lib.h"
#include "net.h"

struct net_proto_family *net_families[NPROTO];

struct socket *sock_alloc(void) {
	struct socket *sock;

	sock = (struct socket*)malloc(sizeof(struct socket));

	return sock;
}

int sock_create(int family, int type, int protocol, struct socket **res) {
	int err;
	struct socket *sock;
	struct net_proto_family *pf;

	sock = sock_alloc();
	if (sock == NULL) {
		printf("sock_create: sock_alloc failed\n");
		return -1;
	}

	sock->type = type;

	pf = net_families[family];
	if (pf == NULL) {
		printf("sock_create: family in net_families is NULL\n");
		return -1;
	}

	err = pf->create(sock, protocol);
	if (err < 0) {
		printf("sock_create: pf->create() failed\n");
		return -1;
	}

	return 0;
}

int sock_register(struct net_proto_family *ops) {
	int err;

	if( ops->family >= NPROTO) {
		printf("sock_register: protocol %d >= NPROTO(%d)\n", ops->family,
				NPROTO);
		return -1;
	}

	if (net_families[ops->family]) {
		err = -1;
	} else {
		net_families[ops->family] = ops;
		err = 0;
	}

	printf("NET: Registered protocol family %d\n", ops->family);

	return err;
}
