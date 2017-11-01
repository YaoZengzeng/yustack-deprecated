#include "in.h"
#include "lib.h"
#include "net.h"
#include "socket.h"
#include "syscall.h"

#define UDP_SERVER_PORT 55555
#define UDP_SERVER_ADDRESS 0x0a000001		// 10.0.0.1

#define UDP_BIND_PORT 222
#define UDP_BIND_ADDRESS 0x0a000002			// 10.0.0.2

#define MAX_BUFF_SIZE	1024

int test_udp() {
	int fd, err;
	char buff[] = "Hello, I'm yustack!";
	char recvbuff[MAX_BUFF_SIZE];

	fd = sys_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		printf("test_udp_server: sys_socket failed\n");
		return -1;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(UDP_SERVER_PORT);
	servaddr.sin_addr.s_addr = htonl(UDP_SERVER_ADDRESS);

	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(UDP_BIND_PORT);
	bindaddr.sin_addr.s_addr = htonl(UDP_BIND_ADDRESS);

	err = sys_bind(fd, &bindaddr, sizeof(bindaddr));
	if(err != 0) {
		printf("test_udp: sys_bind failed\n");
		return -1;
	}

	err = sys_sendto(fd, buff, sizeof(buff), 0, &servaddr, sizeof(servaddr));
	if (err != 0) {
		printf("test_udp: sys_sendto failed\n");
		return -1;
	}

	err = sys_recv(fd, recvbuff, sizeof(recvbuff), 0);
	if (err != 0) {
		printf("test_udp: sys_recv failed\n");
		return -1;
	}
	printf("test_udp_server: sys_recv get msg: %s\n", recvbuff);

	return 0;
}

int test() {
	pthread_t thread_udp;
	if (pthread_create(&thread_udp, NULL, test_udp, NULL) != 0) {
		printf("test: pthread_create test_udp_client failed\n");
		return -1;
	}

	return 0;
}