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


int encs_del(ENCs **out) {
	int ret = 0;
	ENCs *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	if (it->els) {
		free(it->els);
		it->els = NULL;
	}

	free(it);

	*out = NULL;

	return ret;
}
