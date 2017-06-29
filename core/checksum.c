#include "lib.h"
#include "types.h"
#include "checksum.h"

uint16_t checksum(uint16_t* buff, int size) {
	uint32_t cksum = 0;

	while(size > 1) {
		cksum += *buff++;
		size -= 2;
	}
	if (size == 1) {
		cksum += htons(*(char *)buff << 8);
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (uint16_t)(~cksum);
}
