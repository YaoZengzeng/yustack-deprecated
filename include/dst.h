#ifndef _YUSTACK_DST_H
#define _YUSTACK_DST_H

struct sk_buff;

struct dst_entry {
	int (*input)(struct sk_buff *);
	int (*output)(struct sk_buff *);
};

#endif /* _YUSTACK_DST_H */
