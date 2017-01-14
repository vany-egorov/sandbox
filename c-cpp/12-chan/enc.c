#include "enc.h"


int enc_new(ENC **out) {
	ENC *it = NULL;

	it = calloc(1, sizeof(Msgs));
	if (!it) {
		return 1;
	}
	*out = it;

	return 0;
}


int enc_init(ENC *it, size_t cap_chan_i, size_t cap_chan_o) {
	int ret = 0;

	it->w = 0;
	it->h = 0;
	it->b = 0;

	it->thread = 0;

	it->chan_i = NULL;
	it->chan_o = NULL;

	if (!chan_new(&it->chan_i, cap_chan_i, sizeof(void *))) {
		ret = 1; goto cleanup;
	}
	if (!chan_new(&it->chan_o, cap_chan_o, sizeof(void *))) {
		ret = 1; goto cleanup;
	}

cleanup:
	return ret;
}

void enc_new(ENC *it) {
	if (!it) return;
	if (it->chan_i != NULL) {
		free(it->els);
		it->els = NULL;
	}
	free(it);
}
