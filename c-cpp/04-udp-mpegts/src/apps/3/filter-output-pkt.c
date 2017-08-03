#include "filter-output-pkt.h"


int filter_output_pkt_new(FilterOutputPkt **out) {
	int ret = 0;
	FilterOutputPkt *it = NULL;

	it = calloc(1, sizeof(FilterOutputPkt));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_output_pkt_init(FilterOutputPkt *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "output-pkt";
	it->fltr.vt = &filter_output_pkt_vt;
}

static int consume_pkt(void *ctx, Packet *pkt) {
	return filter_produce_pkt(ctx, pkt);
}

int filter_output_pkt_fin(FilterOutputPkt *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_output_pkt_del(FilterOutputPkt **out) {
	int ret = 0;
	FilterOutputPkt *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_output_pkt_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_output_pkt_vt = {
	.consume_strm = NULL,
	.consume_trk = NULL,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = NULL,
	.consume_frm = NULL,
};
