#include "filter-unknown.h"


int filter_unknown_new(FilterUnknown **out) {
	int ret = 0;
	FilterUnknown *it = NULL;

	it = calloc(1, sizeof(FilterUnknown));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_unknown_init(FilterUnknown *it, filter_on_trk_detect on_detect, void* on_detect_ctx) {
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "unknown";
	it->fltr.vt = &filter_unknown_vt;

	it->on_detect = on_detect;
	it->on_detect_ctx = on_detect_ctx;

	it->trk = NULL;
	it->f_dump = NULL;
}

static int consume_trk(void *ctx, Track *trk) {
	FilterUnknown *it = NULL;
	it = (FilterUnknown*)ctx;

	printf("[%s @ %p] [<] track\n", it->fltr.name, (void*)it);

	it->trk = trk;

	return filter_produce_trk(ctx, trk);
}

static int consume_pkt(void *ctx, Packet *pkt) {
	FilterUnknown *it = NULL;
	Track *trk = NULL;

	it = (FilterUnknown*)ctx;
	trk = it->trk;

	/*if (!it->f_dump) {
		char f_name[255] = { 0 };
		snprintf(f_name, sizeof(f_name), "/vagrant/sandbox/c-cpp/04-udp-mpegts/tmp/app-3-out-%d.out", trk->id);
		it->f_dump = fopen(f_name, "wb");
	}*/

	/*fwrite(pkt->buf.v, pkt->buf.len, 1, it->f_dump);
	fflush(it->f_dump);*/

	/*printf("[%s @ %p] [<] track %p index: %d, PID/ID: %d; 0x%02X 0x%02X 0x%02X 0x%02X\n",
		it->fltr.name, (void*)it,
		trk, trk->i, trk->id,
		pkt->buf.v[0], pkt->buf.v[1], pkt->buf.v[2], pkt->buf.v[3]
	);*/


	uint16_t sync_word_ac3 = (
		(uint16_t)pkt->buf.v[0] << 8 |
		(uint16_t)pkt->buf.v[1]
	);

	if (sync_word_ac3 == AC3_SYNC_WORD) {
		trk->codec_kind = CODEC_KIND_AC3;

		printf("[%s @ %p] [<] track %p index: %d, PID/ID: %d, packet-len: %ld, codec: \"%s\"; 0x%02X 0x%02X 0x%02X 0x%02X\n",
			it->fltr.name, (void*)it,
			trk, trk->i, trk->id,
			pkt->buf.len,
			codec_kind_str(trk->codec_kind),
			pkt->buf.v[0], pkt->buf.v[1], pkt->buf.v[2], pkt->buf.v[3]
		);

		it->on_detect(it->on_detect_ctx, trk);
	} else {
		/* printf("[%s @ %p] [<] track %p index: %d, PID/ID: %d, packet-len: %ld; 0x%02X 0x%02X 0x%02X 0x%02X\n",
			it->fltr.name, (void*)it,
			trk, trk->i, trk->id,
			pkt->buf.len,
			pkt->buf.v[0], pkt->buf.v[1], pkt->buf.v[2], pkt->buf.v[3]
		); */
	}

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
