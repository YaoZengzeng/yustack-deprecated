#include "lib.h"
#include "net.h"
#include "sock.h"
#include "socket.h"
#include "skbuff.h"

// skb_copy_datagram_iovec - Copy a datagram to an iovec
int skb_copy_datagram_iovec(struct sk_buff *skb, int offset, struct iovec *to, int len) {
	int err;

	skb_pull(skb, offset);

	memcpy(to->iov_base, skb->data, skb->len);
	to->iov_len = skb->len;

	return 0;
}

// skb_recv_datagram - Receive a datagram skbuff
struct sk_buff *skb_recv_datagram(struct sock *sk, unsigned flags, int noblock, int *err) {
	struct sk_buff *skb;

	do {
		skb = skb_dequeue(&sk->sk_receive_queue);
		if (skb) {
			return skb;
		}
	} while(1);

	return NULL;
}