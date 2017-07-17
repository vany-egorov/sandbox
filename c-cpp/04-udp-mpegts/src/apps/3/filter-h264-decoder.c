#include "filter-h264-decoder.h"


static int consume_strm(void *ctx, Stream *strm) {
	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}


FilterVT filter_h264_decoder_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
