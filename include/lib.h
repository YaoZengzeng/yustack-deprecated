#ifndef _YUSTACK_LIB_H
#define _YUSTACK_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>

#define DEBUG 1

#define htons(x) ((x >> 8 & 0x00ff) | (x << 8))
#define ntohs(x) (htons(s))

#endif /* _YUSTACK_LIB_H */
