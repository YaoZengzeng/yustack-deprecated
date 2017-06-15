#include "lib.h"
#include "tap.h"
#include "driver.h"

int main(int argc, char **argv) {
	if (tapdevice_init() != 0 ) {
		printf("tapdevice_init failed\n");
		return -1;
	}

	if (ether_init_module() != 0) {
		printf("init ethernet driver failed\n");
		return -1;
	}

	return 0;
}
