#include "filter-output-pkt-raw.h"


int filter_output_pkt_raw_new(FilterOutputPktRaw **out) {
	int ret = 0;
	FilterOutputPktRaw *it = NULL;

	it = calloc(1, sizeof(FilterOutputPktRaw));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_output_pkt_raw_init(FilterOutputPktRaw *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "output-pkt-raw";
	it->fltr.vt = &filter_output_pkt_raw_vt;
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_output_pkt_raw_fin(FilterOutputPktRaw *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_output_pkt_raw_del(FilterOutputPktRaw **out) {
	int ret = 0;
	FilterOutputPktRaw *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_output_pkt_raw_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_output_pkt_raw_vt = {
	.consume_strm = NULL,
	.consume_trk = NULL,
	.consume_pkt = NULL,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
