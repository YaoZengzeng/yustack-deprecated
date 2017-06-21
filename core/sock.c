#include "lib.h"
#include "sock.h"

struct sock *sk_alloc(int family) {
	struct sock *sk;

	sk = malloc(sizeof(struct sock));
	if (sk == NULL) {
		printf("sk_alloc: malloc failed\n");
		return NULL;
	}

	sk->sk_family = family;

	skb_queue_head_init(&(sk->sk_write_queue));

	return sk;
}
