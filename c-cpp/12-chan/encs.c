#include "encs.h"


int encs_new(ENCs **out) {
	ENCs *it = NULL;

	it = calloc(1, sizeof(ENCs));
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


int encs_del(ENCs *it) {
	if (!it) return;
	free(it);
}
