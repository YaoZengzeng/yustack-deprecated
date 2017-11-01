#include "lib.h"
#include "sock.h"
#include "skbuff.h"
#include "protocol.h"

// sk_alloc - All socket objects are allocated here
// family: protocol family
// prot: struct proto associated with this new sock instance
struct sock *sk_alloc(int family, struct proto *prot) {
	struct sock *sk;

	// For udp, prot is udp_prot and obj_size is sizeof(struct udp_sock)
	sk = malloc(prot->obj_size);
	if (sk == NULL) {
		printf("sk_alloc: malloc failed\n");
		return NULL;
	}
	memset(sk, 0, prot->obj_size);

	sk->sk_family = family;
	sk->sk_prot = prot;

	return sk;
}

void sock_init_data(struct socket *sock, struct sock *sk) {
	skb_queue_head_init(&(sk->sk_write_queue));
	skb_queue_head_init(&(sk->sk_receive_queue));

	if (sock) {
		sock->sk = sk;
	} else {
		printf("sock_init_data: sock is NULL\n");
	}

	return;
}

int sock_queue_rcv_skb(struct sock *sk, struct sk_buff *skb) {
	skb_queue_tail(&sk->sk_receive_queue, skb);

	return 0;
}

int sock_common_recvmsg(struct socket *sock, struct msghdr *msg, int size, int flags) {
	struct sock *sk = sock->sk;
	int addr_len = 0;
	int err;

	err = sk->sk_prot->recvmsg(sk, msg, size, 0, 0, &addr_len);

	if (err <0) {
		printf("sock_common_recvmsg: sk->sk_prot->recvmsg failed\n");
		return -1;
	}

	msg->msg_namelen = addr_len;

	return 0;
}
