#include "demuxer-ts.h"


int demuxer_ts_new(DemuxerTS **out) {
	int ret = 0;
	DemuxerTS *it = NULL;

	it = calloc(1, sizeof(DemuxerTS));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int demuxer_ts_init(DemuxerTS *it) {
}


static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	int ret = 0;

	if (buf[0] != MPEGTS_SYNC_BYTE) return 1;

	printf("[demuxer-ts @ %p] [<]\n", ctx);

	return ret;
}


DemuxerVT demuxer_ts_vt = {
	.consume_pkt_raw = consume_pkt_raw,
};
