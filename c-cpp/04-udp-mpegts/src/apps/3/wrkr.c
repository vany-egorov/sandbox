#include "wrkr.h"


static Logger *lgr = &logger_std;


int wrkr_new(Wrkr **out) {
	int ret = 0;
	Wrkr *it = NULL;

	it = calloc(1, sizeof(Wrkr));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

/* TODO: error code */
/* TODO: cfg is invalid: i.e. url scheme is not supported */
int wrkr_init(Wrkr *it, WrkrCfg *cfg) {
	input_build(&it->input, url_protocol(cfg->url), cfg->i);
	input_open(&it->input, cfg->url);
	demuxer_build(&it->demuxer, cfg->url);

	/* build-pipeline */
	filter_init((Filter*)&it->splitter);
	it->splitter.fltr.w = &it->splitter;
	it->splitter.fltr.name = "splitter";
	it->splitter.fltr.vt = &filter_splitter_vt;
	filter_init((Filter*)&it->h264_parser);
	it->h264_parser.fltr.w = &it->h264_parser;
	it->h264_parser.fltr.name = "h264-parser";
	it->h264_parser.fltr.vt = &filter_h264_parser_vt;
	filter_init((Filter*)&it->h264_decoder);
	it->h264_decoder.fltr.w = &it->h264_decoder;
	it->h264_decoder.fltr.vt = &filter_h264_decoder_vt;
	it->h264_decoder.fltr.name = "h264-decoder";

	printf("fitler: [%s @ %p]\n", it->demuxer->name, it->demuxer);

	filter_append_consumer(it->demuxer, &it->splitter.fltr);
	filter_append_consumer(&it->splitter.fltr, &it->h264_parser.fltr);
	filter_append_consumer(&it->h264_parser.fltr, &it->h264_decoder.fltr);
}

static int on_read(void *ctx, uint8_t *buf, size_t bufsz) {
	Wrkr *it = (Wrkr*)ctx;

	filter_consume_pkt_raw(it->demuxer, buf, bufsz);
	return 0;
}

static void* go(void *args) {
	Wrkr *it = (Wrkr*)args;

	for (;;) {
		input_read(&it->input, (void*)it, on_read);
		// sleep(1);
	}
}

int wrkr_run(Wrkr *it) {  /* TODO: error code */
	if (pthread_create(&it->_thrd, NULL, go, (void*)it))
		return 1;

	return 0;
}

int wrkr_fin(Wrkr *it) { return 0; }

int wrkr_del(Wrkr **out) {
	int ret = 0;
	Wrkr *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	wrkr_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
