#include "filter-h264-parser.h"


int filter_h264_parser_new(FilterH264Parser **out) {
	int ret = 0;
	FilterH264Parser *it = NULL;

	it = calloc(1, sizeof(FilterH264Parser));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_h264_parser_init(FilterH264Parser *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "h264-parser";
	it->fltr.vt = &filter_h264_parser_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterH264Parser *it = NULL;
	it = (FilterH264Parser*)ctx;

	printf("[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterH264Parser *it = NULL;
	it = (FilterH264Parser*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

/* static FILE *f_dump = NULL; */
static int consume_pkt(void *ctx, Packet *pkt) {
	/*if (!f_dump)
		f_dump = fopen("/vagrant/sandbox/c-cpp/04-udp-mpegts/tmp/app-3-out.h264", "wb");

	fwrite(pkt->buf.v, pkt->buf.len, 1, f_dump);
	fflush(f_dump);*/

	return filter_produce_pkt(ctx, pkt);
}

int filter_h264_parser_fin(FilterH264Parser *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_h264_parser_del(FilterH264Parser **out) {
	int ret = 0;
	FilterH264Parser *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_h264_parser_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_h264_parser_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = NULL,
	.consume_frm = NULL,
};
