#include "in.h"
#include "lib.h"
#include "net.h"
#include "socket.h"

struct net_proto_family *net_families[NPROTO];

#define MAX_SOCK_ADDR 128

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

int sock_recvmsg(struct socket *sock, struct msghdr *msg, int size, int flags) {
	return sock->ops->recvmsg(sock, msg, size, flags);
}

// Receive a frame from the socket and optionally record the address of the sender.
// We verify the buffers are writable and if needed move the sender address from
// kernel to user space.
int sys_recvfrom(int fd, void *ubuf, int size, unsigned flags,
			struct sockaddr *addr, int *addr_len) {
	struct socket *sock;
	struct iovec iov;
	struct msghdr msg;
	char address[MAX_SOCK_ADDR];
	int err;

	sock = sock_from_fd(fd);
	if (sock == NULL) {
		printf("sys_recvfrom: sock_from_fd failed\n");
		return -1;
	}

	msg.msg_iovlen = 1;
	msg.msg_iov = &iov;
	iov.iov_len = size;
	iov.iov_base = ubuf;
	msg.msg_name = address;
	msg.msg_namelen = MAX_SOCK_ADDR;

	err = sock_recvmsg(sock, &msg, size, flags);
	if (err < 0) {
		printf("sys_recvfrom: sock_recvmsg failed\n");
		return -1;
	}

	return 0;
}

// Receive a datagram from a socket
int sys_recv(int fd, void *ubuf, int size, unsigned flags) {
	return sys_recvfrom(fd, ubuf, size, flags, NULL, NULL);
}

int sys_bind(int fd, struct sockaddr *addr, int addrlen) {
	struct socket *sock;
	int err;

	sock = sock_from_fd(fd);
	if (sock == NULL) {
		printf("sys_bind: sock_from_fd failed\n");
		return -1;
	}

	err = sock->ops->bind(sock, addr, addrlen);
	if (err != 0) {
		printf("sys_bind sock->ops->bind failed\n");
		return -1;
	}

	return 0;
}