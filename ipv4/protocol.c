#include "lib.h"
#include "types.h"
#include "protocol.h"

struct net_protocol *inet_protos;

int inet_add_protocol(struct net_protocol *prot, uint8_t protocol) {
	prot->next = inet_protos;

	inet_protos = prot;

	return 0;
}

