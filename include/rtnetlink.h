#ifndef _YUSTACK_RTNETLINK_H
#define _YUSTACK_RTNETLINK_H

// Reserved table identifiers
enum rt_class_t {
	RT_TABLE_UNSPEC=0,
	// User defined values
	RT_TABLE_DEFAULT=253,
	RT_TABLE_MAIN=254,
	RT_TABLE_LOCAL=255,
	RT_TABLE_MAX=0xFFFFFFFF
};

#endif /* _YUSTACK_RTNETLINK_H */
