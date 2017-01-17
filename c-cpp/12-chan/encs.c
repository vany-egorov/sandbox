#include "encs.h"


int encs_new(ENCs **out) {
	ENCs *it = NULL;

	it = (ENCs*)calloc(1, sizeof(ENCs));
	if (!it) return 1;

	*out = it;

	it->len = 0;
	it->els = NULL;

	return 0;
}

int encs_push(ENCs *it, ENC *el) {
	int ret = 0;

	it->els = (ENC**)realloc(it->els, (it->len + 1)*sizeof(ENC*));

	if (!it->els) { ret = 1; goto cleanup; }

	it->els[it->len] = el;
	it->len++;

cleanup:
	return ret;
}

int enc_push(ENC *it, ENCs *collection) {
	it->index = (uint8_t)collection->len;
	return encs_push(collection, it);
}

int encs_wait(ENCs *it) {
	int ret = 0,
	    i = 0;
	ENC *enc = NULL;

	for (i = 0; i < it->len; i++) {
		enc = it->els[i];
		chan_wait(enc->chan_o);
	}

	return ret;
}

int encs_stop(ENCs *it) {
	int ret = 0,
	    i = 0;
	ENC *enc = NULL;

	for (i = 0; i < it->len; i++) {
		enc = it->els[i];
		enc_stop(enc);
	}

	return 0;
}

int encs_del(ENCs **out) {
	int ret = 0;
	ENCs *it = NULL;

	if (!out) goto cleanup;

	it = *out;

	if (!it) goto cleanup;

	if (it->els) {
		free(it->els);
		it->els = NULL;
	}

	free(it);

	*out = NULL;

cleanup:
	x265_cleanup();

	return ret;
}
