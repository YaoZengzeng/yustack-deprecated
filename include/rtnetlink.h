#ifndef _YUSTACK_RTNETLINK_H
#define _YUSTACK_RTNETLINK_H

// Types of messages
enum {
	RTM_NEWROUTE = 24
};

// Reserved table identifiers
enum rt_class_t {
	RT_TABLE_UNSPEC=0,
	// User defined values
	RT_TABLE_DEFAULT=253,
	RT_TABLE_MAIN=254,
	RT_TABLE_LOCAL=255,
	RT_TABLE_MAX=0xFFFFFFFF
};

// rtm_scope
//
// Really it is not scope, but sort of distance to the destination.
// NOWHERE are reserved for not existing destinations, HOST is our
// local addresses, LINK are destinations, located on directly attached
// link and UNIVERSE is everywhere in the Universe
//
// Intermediate values are also possible f.e. interior routes
// could be assigned a value between UNIVERSE and LINK.

enum rt_scope_t {
	RT_SCOPE_UNIVERSE=0,
	// User defined values
	RT_SCOPE_SITE=200,
	RT_SCOPE_LINK=253,
	RT_SCOPE_HOST=254,
	RT_SCOPE_NOWHERE=255
};

// rtm_type
enum {
	RTN_UNSPEC,
	RTN_UNICAST,		// Gateway or direct route
	RTN_LOCAL,			// Accept locally
	RTN_BROADCAST,		// Accept locally as broadcast, send as broadcast
	RTN_ANYCAST,		// Accept locally as broadcast, but send as unicast
	RTN_MULTICAST,		// Multicast route
	RTN_BLACKHOLE,		// Drop
	RTN_UNREACHABLE,	// Destination is unreachable
	RTN_PROHIBIT,		// Administratively prohibited
	RTN_THROW,			// Not in this table
	RTN_NAT,			// Translate this address
	RTN_XRESOLVE,		// Use external resolver
	_RTN_MAX
};

#endif /* _YUSTACK_RTNETLINK_H */
