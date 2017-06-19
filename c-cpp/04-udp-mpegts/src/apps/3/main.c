#include <stdio.h>

#include "./cfg.h"

// make && ../bin/app-3 udp://239.255.1.1:5500 -i udp://239.255.1.1:5500 -i udp://239.255.1.2:5500 -i=239.255.1.10:5500 -i /tmp/dump-i.ts -- 239.255.1.3:5500 /tmp/dump-3/ts

int main (int argc, char *argv[]) {
	CFG cfg = { 0 };
	cfg_init(&cfg);
	cfg_parse(&cfg, argc, argv);
}
