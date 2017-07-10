#include "demuxer.h"


int demuxer_consume_pkt_raw(Demuxer *it, uint8_t *buf, size_t bufsz) {
	if ((!it) || (!it->vt)) return 0;

	it->vt->consume_pkt_raw(it->w, buf, bufsz);
}
