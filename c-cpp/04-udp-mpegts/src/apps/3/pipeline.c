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
	filter_init(&it->fltr);
	it->fltr.w = (void*)it;
	it->fltr.name = "pipeline";
	it->fltr.vt = &pipeline_filter_vt;

	slice_init(&it->trks, sizeof(Track));
	slice_init(&it->fltrs, sizeof(char*));
}

int pipeline_on_trk_detect(void *ctx, Track *trk) {
	int ret = 0;
	FilterTrack *src = NULL;
	FilterUnknown *funk = NULL;  /* old filter */
	Pipeline *it = NULL;

	it = (Pipeline*)ctx;

	/* <remove and deregister unknown filter> */
	/* TODO: remove from fltrs */
	{int i = 0; for (i = 0; i < (int)it->trks.len; i++) {
		FilterTrack *fltrtrk = slice_get(&it->trks, (size_t)i);

		if (trk->id == fltrtrk->trk->id) {
			FilterUnknown *funk = (FilterUnknown*)fltrtrk->fltr->w;

			slice_del_el(&it->trks, (size_t)i);

			filter_unknown_fin(funk);
			filter_unknown_del(&funk);

			break;
		}
	}}
	/* </remove and deregister unknown filter> */

	if (trk->codec_kind == CODEC_KIND_AC3) {
		FilterAC3Parser *parser = NULL;
		FilterAC3Decoder *decoder = NULL;

		filter_ac3_parser_new(&parser);
		filter_ac3_parser_init(parser);
		filter_ac3_decoder_new(&decoder);
		filter_ac3_decoder_init(decoder);

		slice_append(&it->fltrs, &parser);
		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		FilterTrack src_s = {
			.fltr = &parser->fltr,
			.trk = trk,
		};
		src = &src_s;

		slice_append(&it->trks, src);
		src->fltr->vt->consume_trk(src->fltr->w, trk);
	}

	return 0;
}

static int consume_strm(void *ctx, Stream *strm) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	printf("[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	return filter_produce_strm(&it->fltr, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	int ret = 0;
	Pipeline *it = NULL;
	FilterTrack *src = NULL;
	it = (Pipeline*)ctx;

	printf("[%s @ %p] [<] track %p index: %d, PID/ID: %d\n",
		it->fltr.name, (void*)it,
		trk, trk->i, trk->id);

	if (trk->codec_kind == CODEC_KIND_H264) {
		FilterH264Parser *parser = NULL;
		FilterH264Decoder *decoder = NULL;

		filter_h264_parser_new(&parser);
		filter_h264_parser_init(parser);
		filter_h264_decoder_new(&decoder);
		filter_h264_decoder_init(decoder);

		slice_append(&it->fltrs, &parser);
		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		FilterTrack src_s = {
			.fltr = &parser->fltr,
			.trk = trk,
		};
		src = &src_s;

	} else if (trk->codec_kind == CODEC_KIND_MP2) {
		FilterMP2Parser *parser = NULL;
		FilterMP2Decoder *decoder = NULL;

		filter_mp2_parser_new(&parser);
		filter_mp2_parser_init(parser);
		filter_mp2_decoder_new(&decoder);
		filter_mp2_decoder_init(decoder);

		slice_append(&it->fltrs, &parser);
		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		FilterTrack src_s = {
			.fltr = &parser->fltr,
			.trk = trk,
		};
		src = &src_s;

	} else if (trk->codec_kind == CODEC_KIND_UNKNOWN) {
		FilterUnknown *fltr = NULL;

		filter_unknown_new(&fltr);
		filter_unknown_init(fltr, pipeline_on_trk_detect, (void*)it);

		FilterTrack src_s = {
			.fltr = &fltr->fltr,
			.trk = trk,
		};
		src = &src_s;
	} else {
		/* TODO: handle */
	}

	if (src) {
		slice_append(&it->trks, src);
		src->fltr->vt->consume_trk(src->fltr->w, trk);
	}

	return ret;
}

static int consume_pkt(void *ctx, Packet *pkt) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	if (!pkt->trk) { return 1; }  /* error: pkt without filter set */

	{int i = 0; for (i = 0; i < (int)it->trks.len; i++) {
		FilterTrack *fltrtrk = slice_get(&it->trks, (size_t)i);

		if (pkt->trk->id == fltrtrk->trk->id)
			return fltrtrk->fltr->vt->consume_pkt(fltrtrk->fltr->w, pkt);
	}}

	return 2;  /* error: 404 suitable filter not found */
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	return filter_produce_pkt_raw(&it->fltr, buf, bufsz);
}

int pipeline_fin(Pipeline *it) {
	filter_fin(&it->fltr);
	slice_fin(&it->trks);
	slice_fin(&it->fltrs);
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
