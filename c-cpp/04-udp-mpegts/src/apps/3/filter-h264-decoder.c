#include "filter-h264-decoder.h"


int filter_h264_decoder_new(FilterH264Decoder **out) {
	int ret = 0;
	FilterH264Decoder *it = NULL;

	it = calloc(1, sizeof(FilterH264Decoder));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_h264_decoder_init(FilterH264Decoder *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "h264-decoder";
	it->fltr.vt = &filter_h264_decoder_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterH264Decoder *it = NULL;
	it = (FilterH264Decoder*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterH264Decoder *it = NULL;
	it = (FilterH264Decoder*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_h264_decoder_fin(FilterH264Decoder *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_h264_decoder_del(FilterH264Decoder **out) {
	int ret = 0;
	FilterH264Decoder *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_h264_decoder_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_h264_decoder_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
