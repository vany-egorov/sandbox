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

	it->m = NULL;

	slice_init(&it->trks, sizeof(Track));
	slice_init(&it->fltrs, sizeof(char*));
}

static int append_trk(Pipeline *it, Track *trk) {
	int ret = 0;
	FilterTrack *head = NULL;
	Filter *tail = NULL;

	FilterTrack head_s = {
		.fltr = NULL,
		.trk = NULL,
	};

	if (trk->codec_kind == CODEC_KIND_H264) {
		FilterH264Parser *parser = NULL;
		FilterH264Decoder *decoder = NULL;

		filter_h264_parser_new(&parser);
		filter_h264_parser_init(parser);
		filter_h264_decoder_new(&decoder);
		filter_h264_decoder_init(decoder);

		/* don't append parser to filters;
		 * already appended to trks
		 */
		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		head_s.fltr = &parser->fltr;
		head_s.trk = trk;

		head = &head_s;
		tail = &decoder->fltr;

	} else if (trk->codec_kind == CODEC_KIND_MP2) {
		FilterMP2Parser *parser = NULL;
		FilterMP2Decoder *decoder = NULL;

		filter_mp2_parser_new(&parser);
		filter_mp2_parser_init(parser);
		filter_mp2_decoder_new(&decoder);
		filter_mp2_decoder_init(decoder);

		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		head_s.fltr = &parser->fltr;
		head_s.trk = trk;

		head = &head_s;
		tail = &decoder->fltr;

	} else if (trk->codec_kind == CODEC_KIND_AC3) {
		FilterAC3Parser *parser = NULL;
		FilterAC3Decoder *decoder = NULL;

		filter_ac3_parser_new(&parser);
		filter_ac3_parser_init(parser);
		filter_ac3_decoder_new(&decoder);
		filter_ac3_decoder_init(decoder);

		slice_append(&it->fltrs, &decoder);

		filter_append_consumer(&parser->fltr, &decoder->fltr);

		head_s.fltr = &parser->fltr;
		head_s.trk = trk;

		head = &head_s;
		tail = &decoder->fltr;

	} else if (trk->codec_kind == CODEC_KIND_UNKNOWN) {
		FilterUnknown *fltr = NULL;

		filter_unknown_new(&fltr);
		filter_unknown_init(fltr, pipeline_on_trk_detect, (void*)it);

		head_s.fltr = &fltr->fltr;
		head_s.trk = trk;

		head = &head_s;
		tail = &fltr->fltr;

	} else {
		/* TODO: handle */
	}

	if (head) {
		slice_append(&it->trks, head);
		head->fltr->vt->consume_trk(head->fltr->w, trk);
	}

	if ((tail) && (it->m->len)) {
		{int i = 0; for (i = 0; i < (int)it->m->len; i++) {
			PipelineMapCfg *mcfg = slice_get(it->m, (size_t)i);

			if (map_match(&mcfg->m, trk->codec_kind, trk->id)) {
				char buf[255];
				map_str(&mcfg->m, buf, sizeof(buf));
				{int j = 0; for (j = 0; j < (int)mcfg->o.len; j++) {
					PipelineOutputCfg *ocfg = slice_get(&mcfg->o, (size_t)j);

					if (url_is_null(&ocfg->u)) continue;

					FilterOutPkt *fltr = NULL;

					filter_out_pkt_new(&fltr);
					filter_out_pkt_init(fltr);
					fltr->u = &ocfg->u;

					filter_append_consumer(tail, &fltr->fltr);

					slice_append(&it->fltrs, &fltr->fltr);

					fltr->fltr.vt->consume_trk(fltr->fltr.w, trk);
				}}
			}
		}}
	}

	return ret;
}

int pipeline_on_trk_detect(void *ctx, Track *trk) {
	int ret = 0;
	FilterUnknown *funk = NULL;  /* old filter */
	Pipeline *it = NULL;

	it = (Pipeline*)ctx;

	/* <remove and deregister unknown filter> */
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

	ret = append_trk(it, trk);

	return ret;
}

static int consume_strm(void *ctx, Stream *strm) {
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] stream\n", it->fltr.name, (void*)it);

	if (it->m->len) {
		{int i = 0; for (i = 0; i < (int)it->m->len; i++) {
			PipelineMapCfg *mcfg = slice_get(it->m, (size_t)i);

			if (mcfg->m.kind == MAP_ALL) {
				{int j = 0; for (j = 0; j < (int)mcfg->o.len; j++) {
					PipelineOutputCfg *ocfg = slice_get(&mcfg->o, (size_t)j);

					if (url_is_null(&ocfg->u)) continue;

					FilterOutPktRaw *fltr = NULL;

					filter_out_pkt_raw_new(&fltr);
					filter_out_pkt_raw_init(fltr);
					fltr->u = &ocfg->u;

					filter_append_consumer(&it->fltr, &fltr->fltr);

					slice_append(&it->fltrs, &fltr->fltr);
				}}
			}
		}}
	}

	return filter_produce_strm(&it->fltr, strm);
}

static int consume_trk(void *ctx, Track *trk) {
	int ret = 0;
	Pipeline *it = NULL;
	it = (Pipeline*)ctx;

	log_trace(filter_logger, "[%s @ %p] [<] track %p index: %d, PID/ID: %d\n",
		it->fltr.name, (void*)it,
		trk, trk->i, trk->id);

	ret = append_trk(it, trk);
	ret = filter_produce_trk(ctx, trk);

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
