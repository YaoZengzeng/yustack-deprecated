#ifndef _YUSTACK_SOCKET_H
#define _YUSTACK_SOCKET_H

struct iovec {
	void 	*iov_base;
	uint16_t	iov_len;
};

struct msghdr {
	void 	*msg_name;		// Socket name
	int 	msg_namelen;	// Length of name
	struct iovec	*msg_iov;	// Data blocks
	uint16_t	msg_iovlen;		// Number of blocks
};

// Supported address families
#define AF_INET	2	// Internet IP Protocol

// Protocol families, same as address families
#define PF_INET	AF_INET

#endif /* _YUSTACK_SOCKET_H */
