#include "lib.h"
#include "ip_fib.h"
#include "rtnetlink.h"

struct fib_table *ip_fib_local_table;
struct fib_table *ip_fib_main_table;

void ip_fib_init(void) {
	ip_fib_local_table = fib_hash_init(RT_TABLE_LOCAL);
	ip_fib_main_table = fib_hash_init(RT_TABLE_MAIN);
}

int fib_netdev_event(unsigned long event, void *ptr) {
	struct net_device *dev = ptr;
}