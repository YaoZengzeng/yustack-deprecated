#include "lib.h"
#include "types.h"
#include "skbuff.h"
#include "netdevice.h"

struct sk_buff *alloc_skb(unsigned int length) {
	struct sk_buff *skb;
	uint8_t	*data;

	skb = (struct sk_buff*) malloc(sizeof(struct sk_buff));
	if (skb == NULL) {
		printf("alloc_skb: malloc skb failed\n");
		return NULL;
	}
	memset(skb, 0, sizeof(struct sk_buff));

	data = malloc(length);
	if (data == NULL) {
		printf("alloc_skb: malloc data failed\n");
		return NULL;
	}

	skb->head = data;
	skb->data = data;
	skb->tail = data;
	skb->end  = data + length;

	return skb;
}

struct sk_buff *dev_alloc_skb(unsigned int length) {
	return alloc_skb(length);
}

void kfree_skb(struct sk_buff *skb) {
	free(skb);
}

void skb_reserve(struct sk_buff *skb, int len) {
	if (skb->data + len > skb->end) {
		printf("skb_reserve failed\n");
		return;
	}
	skb->data += len;
	skb->tail += len;
}

unsigned char *skb_push(struct sk_buff *skb, unsigned int len) {
	if (skb->data - len < skb->head) {
		printf("skb_push failed\n");
		return NULL;
	}
	skb->data -= len;
	skb->len += len;

	return skb->data;
}

unsigned char *skb_pull(struct sk_buff *skb, unsigned int len) {
	if (len > skb->len) {
		printf("skb_pull failed\n");
		return NULL;
	}
	skb->len -= len;
	return skb->data += len;
}

int skb_may_pull(struct sk_buff *skb, unsigned int len) {
	return skb->len >= len;
}

unsigned char *skb_put(struct sk_buff *skb, unsigned int len) {
	unsigned char *tmp;
	if (skb->tail + len > skb->end) {
		printf("skb_put failed\n");
		return NULL;
	}
	tmp = skb->tail;
	skb->tail += len;
	skb->len += len;

	return tmp;
}

void skb_queue_head_init(struct sk_buff_head *list) {
	list->prev = list->next = (struct sk_buff *)list;
	list->qlen = 0;
}

void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk) {
	struct sk_buff *next;

	next = list->next;
	list->next = next->prev = newsk;

	newsk->prev = (struct sk_buff *)list;
	newsk->next = next;
}

void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk) {
	struct sk_buff *prev;

	prev = list->prev;
	list->prev = prev->next = newsk;

	newsk->prev = prev;
	newsk->next = (struct sk_buff *)list;
}

struct sk_buff *skb_peek(struct sk_buff_head *list) {
	return list->next;
}

struct sk_buff *skb_dequeue(struct sk_buff_head *list) {
	struct sk_buff *next, *prev, *result;

	prev = (struct sk_buff *) list;
	next = prev->next;
	if (next != prev) {
		result 	= next;
		next	= next->next;
		next->prev = prev;
		prev->next = next;
		result->next = result->prev = NULL;
	}
	return result;
}
