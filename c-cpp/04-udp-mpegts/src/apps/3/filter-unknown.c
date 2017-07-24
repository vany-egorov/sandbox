#include "filter-unknown.h"


int filter_unknown_new(FilterUnknown **out) {
	int ret = 0;
	FilterUnknown *it = NULL;

	it = calloc(1, sizeof(FilterUnknown));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_unknown_init(FilterUnknown *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "unknown";
	it->fltr.vt = &filter_unknown_vt;
}

static int consume_trk(void *ctx, Track *trk) {
	FilterUnknown *it = NULL;
	it = (FilterUnknown*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {

	return filter_produce_pkt(ctx, pkt);
}

int filter_unknown_fin(FilterUnknown *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_unknown_del(FilterUnknown **out) {
	int ret = 0;
	FilterUnknown *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_unknown_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_unknown_vt = {
	.consume_strm = NULL,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = NULL,
	.consume_frm = NULL,
};
