#include "filter-out-pkt-raw.h"


int filter_out_pkt_raw_new(FilterOutPktRaw **out) {
	int ret = 0;
	FilterOutPktRaw *it = NULL;

	it = calloc(1, sizeof(FilterOutPktRaw));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_out_pkt_raw_init(FilterOutPktRaw *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "out-pkt-raw";
	it->fltr.vt = &filter_out_pkt_raw_vt;

	it->u = NULL;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterOutPktRaw *it = NULL;
	it = (FilterOutPktRaw*)ctx;

	printf("[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	if ((!it->__f) && (it->u)) {
		char ubuf[255] = { 0 };
		url_sprint(it->u, ubuf, sizeof(ubuf));

		it->__f = fopen(url_path(it->u), "wb");
		printf("[%s @ %p] [>>>] %s\n", it->fltr.name, (void*)it, ubuf);
	}

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterOutPktRaw *it = NULL;
	it = (FilterOutPktRaw*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	FilterOutPktRaw *it = NULL;
	it = (FilterOutPktRaw*)ctx;

	if (it->__f)
		fwrite(buf, bufsz, 1, it->__f);

	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_out_pkt_raw_fin(FilterOutPktRaw *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_out_pkt_raw_del(FilterOutPktRaw **out) {
	int ret = 0;
	FilterOutPktRaw *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_out_pkt_raw_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_out_pkt_raw_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = NULL,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
