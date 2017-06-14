#include <net/if.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/if_tun.h>

#include "lib.h"
#include "types.h"
#include "tap.h"

int tapfd;

int tapdevice_alloc(char *name) {
	struct ifreq ifr;
	int fd;

	fd = open(TUNDEVICE, O_RDWR);
	if (fd < 0) {
		printf("tapdevice_alloc: open %s failed\n", TUNDEVICE);
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	if (name != NULL) {
		strcpy(ifr.ifr_name, name);
	}

	// Create a new tap device, if already exists, just bind fd with it.
	if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
		printf("tapdevice_alloc: ioctl create a new tap device failed\n");
		close(fd);
		return -1;
	}

	return fd;
}

int set_tapdevice_persist(int fd) {
	if (ioctl(fd, TUNSETPERSIST, 1) < 0) {
		printf("set_tapdevice_persist failed\n");
		return -1;
	}

	return 0;
}

int set_tapdevice_ipaddr(char *name, uint32_t addr) {
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *saddr;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		printf("set_tapdevice_ipaddr: socket open failed\n");
		return -1;
	}

	strcpy(ifr.ifr_name, name);
	saddr = (struct sockaddr_in *)&ifr.ifr_addr;
	saddr->sin_family = AF_INET;
	saddr->sin_addr.s_addr = addr;
	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
		printf("set_tapdevice_ipaddr: ioctl failed\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int set_tapdevice_netmask(char *name, uint32_t mask) {
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *saddr;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		printf("set_tapdevice_netmask: socket failed\n");
		return -1;
	}

	strcpy(ifr.ifr_name, name);
	saddr = (struct sockaddr_in *)&ifr.ifr_netmask;
	saddr->sin_family = AF_INET;
	saddr->sin_addr.s_addr = mask;
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
		printf("set_tapdevice_netmask: ioctl failed\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int set_tapdevice_flags(char *name, unsigned short flags, int set) {
	int fd;
	struct ifreq ifr;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		printf("set_tapdevice_flags: socket failed\n");
		return -1;
	}

	strcpy(ifr.ifr_name, name);
	// Get the original flags
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		printf("set_tapdevice_flags: ioctl SIOCGIFFLAGS failed\n");
		close(fd);
		return -1;
	}

	// if set is true, set new flags, otherwise clear the corresponding bits
	if (set) {
		ifr.ifr_flags |= flags;
	} else {
		ifr.ifr_flags &= ~flags & 0xffff;
	}

	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		printf("set_tapdevice_flags: ioctl SIOCSIFFLAGS failed\n");
		close(fd);
		return -1;
	}

	return 0;
}

int set_tapdevice_up(char *name) {
	return set_tapdevice_flags(name, IFF_UP | IFF_RUNNING, 1);
}

// Alloc a tap device, and set basic configurations
int tapdevice_init(void) {
	tapfd = tapdevice_alloc(TAPDEVICE_NAME);
	if (tapfd < 0) {
		goto failed;
	}

	if (set_tapdevice_persist(tapfd) < 0) {
		goto failed;
	}

	if (set_tapdevice_ipaddr(TAPDEVICE_NAME, FIXED_TAP_ADDR) < 0) {
		goto failed;
	}

	if (set_tapdevice_netmask(TAPDEVICE_NAME, FIXED_TAP_NETMASK) < 0) {
		goto failed;
	}

	if (set_tapdevice_up(TAPDEVICE_NAME) < 0) {
		goto failed;
	}

	printf("tap device initialized successfully\n");

	return 0;

failed:
	return -1;
}