#include "filter-out-pkt.h"


int filter_out_pkt_new(FilterOutPkt **out) {
	int ret = 0;
	FilterOutPkt *it = NULL;

	it = calloc(1, sizeof(FilterOutPkt));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_out_pkt_init(FilterOutPkt *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "out-pkt";
	it->fltr.vt = &filter_out_pkt_vt;

	it->u = NULL;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterOutPkt *it = NULL;
	it = (FilterOutPkt*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterOutPkt *it = NULL;
	it = (FilterOutPkt*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	if ((!it->__f) && (it->u)) {
		char ubuf[255] = { 0 };
		url_sprint(it->u, ubuf, sizeof(ubuf));

		it->__f = fopen(url_path(it->u), "wb");
		log_info(filter_logger, "[%s @ %p] [>>>] %s\n", it->fltr.name, (void*)it, ubuf);
	}

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	FilterOutPkt *it = NULL;
	it = (FilterOutPkt*)ctx;

	/* TODO: use setvbuf */
	if (it->__f)
		fwrite(pkt->buf.v, pkt->buf.len, 1, it->__f);

	return filter_produce_pkt(ctx, pkt);
}

int filter_out_pkt_fin(FilterOutPkt *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_out_pkt_del(FilterOutPkt **out) {
	int ret = 0;
	FilterOutPkt *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_out_pkt_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_out_pkt_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = NULL,
	.consume_frm = NULL,
};
