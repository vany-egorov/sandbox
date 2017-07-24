#include "filter-mp2-decoder.h"


int filter_mp2_decoder_new(FilterMP2Decoder **out) {
	int ret = 0;
	FilterMP2Decoder *it = NULL;

	it = calloc(1, sizeof(FilterMP2Decoder));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_mp2_decoder_init(FilterMP2Decoder *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "mp2-decoder";
	it->fltr.vt = &filter_mp2_decoder_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterMP2Decoder *it = NULL;
	it = (FilterMP2Decoder*)ctx;

	printf("[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterMP2Decoder *it = NULL;
	it = (FilterMP2Decoder*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_mp2_decoder_fin(FilterMP2Decoder *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_mp2_decoder_del(FilterMP2Decoder **out) {
	int ret = 0;
	FilterMP2Decoder *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_mp2_decoder_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_mp2_decoder_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
