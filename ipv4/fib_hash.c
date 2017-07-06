#include "lib.h"
#include "types.h"
#include "ip_fib.h"

struct fib_table *fib_hash_init(uint32_t id) {
	struct fib_table *tb;

	tb = (struct fib_table *)malloc(sizeof(struct fib_table) + sizeof(struct fn_hash));

	tb->tb_id = id;

	memset(tb->tb_data, 0, sizeof(struct fn_hash));

	return tb;
}