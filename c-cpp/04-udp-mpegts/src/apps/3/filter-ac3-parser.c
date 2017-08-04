#include "filter-ac3-parser.h"


int filter_ac3_parser_new(FilterAC3Parser **out) {
	int ret = 0;
	FilterAC3Parser *it = NULL;

	it = calloc(1, sizeof(FilterAC3Parser));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_ac3_parser_init(FilterAC3Parser *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "ac3-parser";
	it->fltr.vt = &filter_ac3_parser_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterAC3Parser *it = NULL;
	it = (FilterAC3Parser*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterAC3Parser *it = NULL;
	it = (FilterAC3Parser*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

int filter_ac3_parser_fin(FilterAC3Parser *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_ac3_parser_del(FilterAC3Parser **out) {
	int ret = 0;
	FilterAC3Parser *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_ac3_parser_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_ac3_parser_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = NULL,
	.consume_frm = NULL,
};
