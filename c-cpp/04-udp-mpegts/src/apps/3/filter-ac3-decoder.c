#include "filter-ac3-decoder.h"


int filter_ac3_decoder_new(FilterAC3Decoder **out) {
	int ret = 0;
	FilterAC3Decoder *it = NULL;

	it = calloc(1, sizeof(FilterAC3Decoder));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_ac3_decoder_init(FilterAC3Decoder *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "ac3-decoder";
	it->fltr.vt = &filter_ac3_decoder_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterAC3Decoder *it = NULL;
	it = (FilterAC3Decoder*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterAC3Decoder *it = NULL;
	it = (FilterAC3Decoder*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_ac3_decoder_fin(FilterAC3Decoder *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_ac3_decoder_del(FilterAC3Decoder **out) {
	int ret = 0;
	FilterAC3Decoder *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_ac3_decoder_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_ac3_decoder_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
