#ifndef _YUSTACK_DRIVER_H
#define _YUSTACK_DRIVER_H

#include "netdevice.h"

int ether_init_module(void);
void ether_rx(struct net_device *dev);

#endif /* _YUSTACK_DRIVER_H */
