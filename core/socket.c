#include "in.h"
#include "lib.h"
#include "net.h"
#include "socket.h"

struct net_proto_family *net_families[NPROTO];

struct socket_map {
	struct socket *sock;
	int used;
};

#define MAXSOCKETNUM 512

struct socket_map socket_maps[MAXSOCKETNUM];

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

	*res = sock;

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

int sys_socket(int family, int type, int protocol) {
	int retval, fd;
	struct socket *sock;

	retval = sock_create(family, type, protocol, &sock);
	if (retval < 0) {
		printf("sys_socket: sock_create failed\n");
		return -1;
	}

	// work around, use the index of socket_maps as fd
	for(fd = 0; fd < MAXSOCKETNUM; fd++) {
		if (socket_maps[fd].used) {
			continue;
		}
		socket_maps[fd].sock = sock;
		socket_maps[fd].used = 1;
		break;
	}

	if (fd == MAXSOCKETNUM) {
		printf("sys_socket: alloc fd-socket map failed\n");
		return -1;
	}

	return fd;
}

struct socket *sock_from_fd(int fd) {
	if (socket_maps[fd].used) {
		return socket_maps[fd].sock;
	}

	printf("sock_from_fd: fd has never been allocated\n");
	return NULL;
}

int sock_sendmsg(struct socket *sock, struct msghdr *msg, int size) {
	return sock->ops->sendmsg(sock, msg, size);
}

int sys_sendto(int fd, void *buff, int len, unsigned flags,
			struct sockaddr *addr, int addr_len) {
	struct socket *sock;
	struct msghdr msg;
	struct iovec iov;
	int err;

	sock = sock_from_fd(fd);
	if (sock == NULL) {
		printf("sys_sendto: sock_from_fd failed\n");
		return -1;
	}

	iov.iov_base = buff;
	iov.iov_len = len;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	if (addr) {
		msg.msg_name = addr;
		msg.msg_namelen = addr_len;
	}

	err = sock_sendmsg(sock, &msg, len);
	if (err != 0) {
		printf("sys_sendto: sock_sendmsg failed\n");
		return -1;
	}

	return 0;
}