#ifndef _YUSTACK_RAW_H
#define _YUSTACK_RAW_H

#include "icmp.h"
#include "protocol.h"

struct proto raw_prot = {
	.name = "RAW",
	.obj_size = sizeof(struct raw_sock),
};

#endif /* _YUSTACK_RAW_H */
