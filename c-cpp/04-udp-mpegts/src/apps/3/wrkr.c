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
int wrkr_init(Wrkr *it, WrkrCfg cfg) {
	it->cfg = cfg;

	input_build(&it->input, url_protocol(&it->cfg.url), &it->cfg.i);
	input_open(&it->input, &it->cfg.url);
	demuxer_build(&it->demuxer, &it->cfg.url);  /* TODO: move demuxer to pipeline? */

	pipeline_init(&it->ppln);
	it->ppln.m = &it->cfg.m;

	filter_append_consumer(it->demuxer, (Filter*)&it->ppln);
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
