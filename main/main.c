#include "in.h"
#include "lib.h"
#include "tap.h"
#include "net.h"
#include "ipv4.h"
#include "test.h"
#include "driver.h"
#include "socket.h"
#include "syscall.h"
#include "netdevice.h"

extern struct net_device *dev_base;

int main(int argc, char **argv) {
	pthread_t threadid;

	if (tapdevice_init() != 0 ) {
		printf("tapdevice_init failed\n");
		return -1;
	}

	if (inet_init() != 0) {
		printf("inet module initialize failed\n");
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

	while(1) {
		ether_rx(dev_base);
	}

	return 0;
}
