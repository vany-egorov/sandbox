#include "./wrkr.h"


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
int wrkr_initialize(Wrkr *it, WrkrCfg cfg) {
	input_open(&it->input, cfg.url);
}

static void* wrkr_do(void *args) {
	Wrkr *it = (Wrkr*)args;

	for (;;) {
		input_read(&it->input);
		sleep(1);
	}
}

int wrkr_run(Wrkr *it) {  /* TODO: error code */
	if (pthread_create(&it->_thrd, NULL, wrkr_do, (void*)it)) {
		return 1;
	}
	return 0;
}

int wrkr_finalize(Wrkr *it) { return 0; }

int wrkr_del(Wrkr **out) {
	int ret = 0;
	Wrkr *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	wrkr_finalize(it);

	free(it);
	*out = NULL;

	return ret;
}
