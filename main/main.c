#include "in.h"
#include "lib.h"
#include "tap.h"
#include "net.h"
#include "ipv4.h"
#include "driver.h"
#include "socket.h"
#include "syscall.h"
#include "netdevice.h"

extern struct net_device *dev_base;

#define UDP_SERVER_PORT 55555
#define UDP_SERVER_ADDRESS 0x0a000001		// 10.0.0.1

#define UDP_LOCAL_PORT	222
#define UDP_LOCAL_ADDRESS 0x0a000002

#define MAX_BUFF_SIZE 1024

int test() {
	int fd, err;
	char buff[] = "Hello, I'm yustack!";
	char recvbuff[MAX_BUFF_SIZE];

	fd = sys_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		printf("test: sys_socket failed\n");
		return -1;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(UDP_SERVER_PORT);
	servaddr.sin_addr.s_addr = htonl(UDP_SERVER_ADDRESS);

	struct sockaddr_in localaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(UDP_LOCAL_PORT);
	localaddr.sin_addr.s_addr = htonl(UDP_LOCAL_ADDRESS);
	err = sys_bind(fd, &localaddr, sizeof(localaddr));

/*	err = sys_sendto(fd, buff, sizeof(buff), 0, &servaddr, sizeof(servaddr));
	if (err != 0) {
		printf("test: sys_sendto failed\n");
		return -1;
	}*/

	err = sys_recv(fd, recvbuff, sizeof(recvbuff), 0);
	if (err != 0) {
		printf("test: sys_recv failed\n");
	} else {
		printf("test: sys_recv get '%s'\n", recvbuff);
	}

	return 0;
}

int main(int argc, char **argv) {
	pthread_t threadid;

	if (inet_init() != 0) {
		printf("inet module initialize failed\n");
		return -1;
	}
	if (tapdevice_init() != 0 ) {
		printf("tapdevice_init failed\n");
		return -1;
	}

	if (ether_init_module() != 0) {
		printf("init ethernet driver failed\n");
		return -1;
	}

	if (pthread_create(&threadid, NULL, test, NULL) != 0) {
		printf("pthread_create create the test thread failed\n");
		return -1;
	}
	//test();

	while(1) {
		ether_rx(dev_base);
	}

	return 0;
}
