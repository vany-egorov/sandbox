#include "pipeline.h"


int pipeline_new(Pipeline **out) {
	int ret = 0;
	Pipeline *it = NULL;

	it = (Pipeline*)calloc(1, sizeof(Pipeline));
	if (!it) return 1;

	*out = it;
	return ret;
}

int pipeline_init(Pipeline *it) {
	slice_fin(&it->trks);
	filter_init(&it->fltr);
}

static int consume_strm(void *ctx, Stream *strm) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	printf("[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(&it->fltr, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	return filter_produce_trk(&it->fltr, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	if (!pkt->trk) { return 0; }

	return filter_produce_pkt(&it->fltr, pkt);
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	return filter_produce_pkt_raw(&it->fltr, buf, bufsz);
}

int pipeline_fin(Pipeline *it) {
	filter_fin(&it->fltr);
	slice_fin(&it->trks);
}

int pipeline_del(Pipeline **out) {
	int ret = 0;
	Pipeline *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	pipeline_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


FilterVT pipeline_filter_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
