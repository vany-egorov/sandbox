#include "filter-mp2-parser.h"


int filter_mp2_parser_new(FilterMP2Parser **out) {
	int ret = 0;
	FilterMP2Parser *it = NULL;

	it = calloc(1, sizeof(FilterMP2Parser));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_mp2_parser_init(FilterMP2Parser *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "mp2-parser";
	it->fltr.vt = &filter_mp2_parser_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterMP2Parser *it = NULL;
	it = (FilterMP2Parser*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterMP2Parser *it = NULL;
	it = (FilterMP2Parser*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_mp2_parser_fin(FilterMP2Parser *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_mp2_parser_del(FilterMP2Parser **out) {
	int ret = 0;
	FilterMP2Parser *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_mp2_parser_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_mp2_parser_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
