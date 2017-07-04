#include "lib.h"
#include "sock.h"
#include "protocol.h"

struct sock *sk_alloc(int family, struct proto *prot) {
	struct sock *sk;

	sk = malloc(prot->obj_size);
	if (sk == NULL) {
		printf("sk_alloc: malloc failed\n");
		return NULL;
	}

	sk->sk_family = family;

	skb_queue_head_init(&(sk->sk_write_queue));

	return sk;
}

void sock_init_data(struct socket *sock, struct sock *sk) {
	sock->sk = sk;
}