#ifndef _YUSTACK_TAP_H
#define _YUSTACK_TAP_H

#define TUNDEVICE "/dev/net/tun"

#define TAPDEVICE_NAME 		"tap0"

#define FIXED_TAP_ADDR		0x0100000a		// 10.0.0.1

#define FIXED_TAP_NETMASK	0x00ffffff		// 255.255.255.0

int tapdevice_init(void);

#endif /* _YUSTACK_TAP_H */
