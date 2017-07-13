#include "packet.h"


int packet_init(Packet *it) {
	buf_init(&it->buf, 1024); /* TODO: move initial capacity to config */
}

int packet_fin(Packet *it) {
	buf_fin(&it->buf);
}
