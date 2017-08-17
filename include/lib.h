#ifndef _YUSTACK_LIB_H
#define _YUSTACK_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "types.h"

#define DEBUG 1

#define htons(x) ((uint16_t)(((x) >> 8 & 0x00ff) | ((x) << 8)))
#define ntohs(x) ((uint16_t)(htons(x)))

#define htonl(x) (((x) << 24 & 0xff000000) | ((x) << 8 & 0x00ff0000) | \
					((x) >> 8 & 0x0000ff00) | ((x) >> 24 & 0x000000ff))
#define ntohl(x) (htonl(x))

#endif /* _YUSTACK_LIB_H */
