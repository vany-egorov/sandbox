#include "filter-metric-b.h"


int filter_metric_b_new(FilterMetricB **out) {
	int ret = 0;
	FilterMetricB *it = NULL;

	it = calloc(1, sizeof(FilterMetricB));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_metric_b_init(FilterMetricB *it) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "metric-bitrate";
	it->fltr.vt = &filter_metric_b_vt;
}

static int consume_strm(void *ctx, Stream *strm) {
	FilterMetricB *it = NULL;
	it = (FilterMetricB*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	FilterMetricB *it = NULL;
	it = (FilterMetricB*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(ctx, trk);
}

static int on_consume_pkt(FilterMetricB *it, size_t sz) {
	int ret = 0;

	return ret;
}

static int consume_pkt(void *ctx, Packet *pkt) {
	FilterMetricB *it = NULL;
	it = (FilterMetricB*)ctx;

	on_consume_pkt(it, pkt->buf.len);

	return filter_produce_pkt(ctx, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	FilterMetricB *it = NULL;
	it = (FilterMetricB*)ctx;

	on_consume_pkt(it, bufsz);

	return filter_produce_pkt_raw(ctx, buf, bufsz);
}

int filter_metric_b_fin(FilterMetricB *it) {
	int ret = 0;

	if (!it) return ret;

	filter_fin(&it->fltr);

	return ret;
}

int filter_metric_b_del(FilterMetricB **out) {
	int ret = 0;
	FilterMetricB *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_metric_b_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT filter_metric_b_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
