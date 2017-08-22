#ifndef _YUSTACK_SYSCALL_H
#define _YUSTACK_SYSCALL_H

int sys_socket(int family, int type, int protocol);

int sys_recv(int fd, void *ubuf, int size, unsigned flags);

int sys_sendto(int fd, void *buff, int len, unsigned flags,
			struct sockaddr *addr, int addr_len);

int sys_recvfrom(int fd, void *ubuf, int size, unsigned flags,
			struct sockaddr *addr, int *addr_len);

#endif /* _YUSTACK_ARP_H */
