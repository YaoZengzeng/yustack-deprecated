#include "lib.h"
#include "dst.h"
#include "types.h"
#include "skbuff.h"
#include "neighbour.h"

struct neigh_table *neigh_tables;

void neigh_table_init(struct neigh_table *tbl) {

	tbl->next = neigh_tables;
	neigh_tables = tbl;
}

int neigh_blackhole(struct sk_buff *skb) {
	kfree_skb(skb);
	return 0;
}

struct neighbour *neigh_alloc(struct neigh_table *tbl) {
	struct neighbour *n = NULL;

	n = (struct neighbour *)malloc(sizeof(struct neighbour) + tbl->key_len);
	if (n == NULL) {
		return NULL;
	}

	skb_queue_head_init(&n->arp_queue);
	n->tbl = tbl;
	n->output = neigh_blackhole;
	n->nud_state = NUD_NONE;

	return n;
}

struct neighbour *neigh_create(struct neigh_table *tbl, void *pkey,
						struct net_device *dev) {
	int key_len = tbl->key_len;
	struct neighbour *n = neigh_alloc(tbl);

	if (n == NULL) {
		printf("neigh_create: neigh_alloc failed\n");
		goto out;
	}

	printf("neigh_create: pkey is %x\n", *(uint32_t *)pkey);
	memcpy(n->primary_key, pkey, key_len);
	n->dev = dev;

	// Protocol specific setup
	if (tbl->constructor && tbl->constructor(n) < 0) {
		printf("neigh_create: table constructor failed\n");
		goto out;
	}

	n->next = tbl->buckets;
	tbl->buckets = n;

	printf("neigh_create succeeded, addr is %x\n", *((uint32_t *)pkey));

	return n;
out:
	return NULL;
}

struct neighbour *neigh_lookup(struct neigh_table *tbl, void *pkey,
					struct net_device *dev) {
	struct neighbour *n;
	int key_len = tbl->key_len;

	for (n = tbl->buckets; n != NULL; n = n->next) {
		if (dev == n->dev && !memcmp(n->primary_key, pkey, key_len)) {
			break;
		}
	}

	return n;
}

int neigh_update(struct neighbour *neigh, uint8_t *lladdr, uint8_t new) {
	struct net_device *dev = neigh->dev;
	struct sk_buff *skb = NULL;

	neigh->nud_state = new;
	memcpy(neigh->ha, lladdr, dev->addr_len);

	if (neigh->nud_state & NUD_REACHABLE) {
		while((skb = skb_dequeue(&neigh->arp_queue)) != NULL) {
			printf("neigh_update: skb_dequeue one\n");
			neigh->output(skb);
		}
	}

	return 0;
}

struct neighbour * __neigh_lookup_errno(struct neigh_table *tbl, void *pkey,
			struct net_device *dev) {
	struct neighbour *n = neigh_lookup(tbl, pkey, dev);

	if (n) {
		return n;
	}

	return neigh_create(tbl, pkey, dev);
}

struct neighbour *
__neigh_lookup(struct neigh_table *tbl, void *pkey, struct net_device *dev, int creat) {
	struct neighbour *n = neigh_lookup(tbl, pkey, dev);

	if (n || !creat) {
		return n;
	}

	n = neigh_create(tbl, pkey, dev);

	return n;
}

struct neighbour *neigh_event_ns(struct neigh_table *tbl,uint8_t *lladdr,
	void *saddr, struct net_device *dev) {
	struct neighbour *neigh = __neigh_lookup(tbl, saddr, dev, lladdr);

	if (neigh) {
		neigh_update(neigh, lladdr, NUD_REACHABLE);
	}

	return neigh;
}

int neigh_event_send(struct neighbour *neigh, struct skb_buff *skb) {
	if (neigh->nud_state == NUD_REACHABLE) {
		return 0;
	}

	if (neigh->nud_state == NUD_NONE) {
		neigh->ops->solicit(neigh, skb);
		neigh->nud_state = NUD_INCOMPLETE;
	}

	if (neigh->nud_state == NUD_INCOMPLETE) {
		skb_queue_tail(&neigh->arp_queue, skb);
		return 1;
	}


	return 0;
}

// Slow and careful
int neigh_resolve_output(struct sk_buff *skb) {
	struct dst_entry *dst = skb->dst;
	struct neighbour *neigh;
	int rc = 0;

	if (!dst || !(neigh = dst->neighbour)) {
		goto discard;
	}

	// skb_pull(skb, skb->nh.raw - skb->data);
	if (!neigh_event_send(neigh, skb)) {
		int err;
		struct net_device *dev = neigh->dev;
		err = dev->hard_header(skb, dev, ntohs(skb->protocol),
							neigh->ha, NULL, skb->len);
		if (err >= 0) {
			rc = neigh->ops->queue_xmit(skb);
		} else {
			goto out_kfree_skb;
		}
	} else {
		printf("neigh_resolve_output: neigh_event_send failed\n");
	}

out:
	return rc;
discard:
	printf("neigh_resolve_output: dst = %p neigh=%p\n",
			dst, dst ? dst->neighbour : NULL);
out_kfree_skb:
	rc = -1;
	kfree_skb(skb);
	goto out;
}
