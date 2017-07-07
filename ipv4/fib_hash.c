#include "lib.h"
#include "types.h"
#include "ip_fib.h"
#include "rtnetlink.h"
#include "inetdevice.h"

struct fib_info *fib_create_info(struct fib_config *cfg) {
	struct fib_info *fi = NULL;
	int nhs = 1;

	fi = (struct fib_info *)malloc(sizeof(struct fib_info) + nhs * sizeof(struct fib_nh));
	if (fi == NULL) {
		printf("fib_create_info: malloc failed\n");
		return NULL;
	}

	struct fib_nh *nh = fi->fib_nh;
	nh->nh_oif = cfg->fc_oif;
	nh->nh_dev = dev_get_by_index(fi->fib_nh->nh_oif);
	nh->nh_gw = cfg->fc_gw;

	return fi;
}

struct fn_zone *fn_new_zone(struct fn_hash *table, int z) {
	struct fn_zone *fz = (struct fn_zone *)malloc(sizeof(struct fn_zone));
	if (fz == NULL) {
		printf("fn_new_zone: malloc failed\n");
		return NULL;
	}
	fz->fz_order = z;
	fz->fz_mask = inet_make_mask(z);
	table->fn_zones[z] = fz;

	return fz;
}

int fn_hash_insert(struct fib_table *tb, struct fib_config *cfg) {
	struct fn_hash *table = (struct fn_hash *) tb->tb_data;
	struct fib_node *f;
	struct fib_alias *fa;
	struct fn_zone *fz;
	struct fib_info *fi;

	fz = table->fn_zones[cfg->fc_dst_len];
	if (!fz && !(fz = fn_new_zone(table, cfg->fc_dst_len))) {
		printf("fn_hash_insert: fn_new_zone failed\n");
		return -1;
	}

	fi = fib_create_info(cfg);

	fa = (struct fib_alias *)malloc(sizeof(struct fib_alias));
	if (fa == NULL) {
		printf("fn_hash_insert: malloc fib_alias failed\n");
		return -1;
	}
	memset(fa, 0, sizeof(struct fib_alias));

	f = (struct fib_node *)malloc(sizeof(struct fib_node));
	if (f == NULL) {
		printf("fn_hash_insert: malloc fib_node failed\n");
		return -1;
	}
	memset(f, 0, sizeof(struct fib_node));

	// set key as the cfg->fc_dst;
	f->fn_key = cfg->fc_dst & inet_make_mask(cfg->fc_dst_len);

	fa->fa_info = fi;
	fa->fa_type = cfg->fc_type;
	fa->fa_scope = cfg->fc_scope;

	// Insert alias to node
	fa->next = f->alias_list;
	f->alias_list = fa;

	// Insert node to zone
	f->next = fz->node_list;
	fz->node_list = f;

	return 0;
}

int fib_semantic_match(struct fib_alias *head, struct flowi *flp,
				struct fib_result *res, uint32_t zone, uint32_t mask,
				int prefixlen) {
	struct fib_alias *fa = head;

	// For simplicity, we just consider destination address match, so the first
	// fib_alias is what we want, then fill res and return is all we should do
	res->prefixlen = prefixlen;
	res->type = fa->fa_type;
	res->scope = fa->fa_scope;
	res->fi = fa->fa_info;

	return 0;
}

int fn_hash_lookup(struct fib_table *tb, struct flowi *flp, struct fib_result *res) {
	int i, err;
	struct fn_zone *fz;
	struct fn_hash *t = (struct fn_hash *)tb->tb_data;

	for (i = 32; i >= 0; i--) {
		fz = t->fn_zones[i];
		if (fz == NULL) {
			continue;
		}
		uint32_t key = flp->fl4_dst & fz->fz_mask;
		struct fib_node *node = fz->node_list;
		while(node) {
			if (node->fn_key != key) {
				node = node->next;
				continue;
			}
			err = fib_semantic_match(node->alias_list, 
								flp, res,
								node->fn_key, fz->fz_mask,
								fz->fz_order);
			if (err <= 0) {
				goto out;
			}
		}
	}
	err = 1;
out:
	return err;
}

struct fib_table *fib_hash_init(uint32_t id) {
	struct fib_table *tb;

	tb = (struct fib_table *)malloc(sizeof(struct fib_table) + sizeof(struct fn_hash));

	tb->tb_id = id;

	memset(tb->tb_data, 0, sizeof(struct fn_hash));

	tb->tb_lookup = fn_hash_lookup;
	tb->tb_insert = fn_hash_insert;
	return tb;
}