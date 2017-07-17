#include "filter-splitter.h"


static int consume_strm(void *ctx, Stream *strm) {
	return filter_produce_strm(ctx, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	return filter_produce_trk(ctx, trk);
}

FILE *f_dump_h264 = NULL;
FILE *f_dump_mp2 = NULL;
static int consume_pkt(void *ctx, Packet *pkt) {
	if (!f_dump_h264) {
		f_dump_h264 = fopen("/vagrant/sandbox/c-cpp/04-udp-mpegts/tmp/app-3-out.h264", "wb");
	}

	if (!f_dump_mp2) {
		f_dump_mp2 = fopen("/vagrant/sandbox/c-cpp/04-udp-mpegts/tmp/app-3-out.mp2", "wb");
	}

	if (!pkt->trk) { return 0; }

	if (pkt->trk->codec_kind == CODEC_KIND_H264) {
		printf("%s / %d %zu\n", codec_kind_str(pkt->trk->codec_kind), pkt->trk->id, pkt->buf.len);
		size_t n = fwrite(pkt->buf.v, pkt->buf.len, 1, f_dump_h264);
		if (n != pkt->buf.len) {
			fprintf(stderr, "[file-write @ %p] fwrite error: \"%s\"\n",
				ctx, strerror(errno));
		}
		// fflush(f_dump_h264);
	}

	// if (pkt->trk->codec_kind == CODEC_KIND_MP2) {
	// 	fwrite(pkt->buf.v, pkt->buf.len, 1, f_dump_mp2);
	// 	fflush(f_dump_mp2);
	// }

	return filter_produce_pkt(ctx, pkt);
}

FILE *f_dump_ts = NULL;
static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	if (!f_dump_ts) {
		f_dump_ts = fopen("/vagrant/sandbox/c-cpp/04-udp-mpegts/tmp/app-3-out.ts", "wb");
	}

	fwrite(buf, bufsz, 1, f_dump_ts);

	return filter_produce_pkt_raw(ctx, buf, bufsz);
}


FilterVT filter_splitter_vt = {
	.consume_strm = consume_strm,
	.consume_trk = consume_trk,
	.consume_pkt = consume_pkt,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
