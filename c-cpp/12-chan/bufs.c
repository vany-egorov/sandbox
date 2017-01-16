#include "bufs.h"


int bufs_new(Bufs **out) {
	Bufs *it = NULL;

	it = (Bufs*)calloc(1, sizeof(Bufs));
	if (!it) return 1;

	*out = it;

	it->len = 0;
	it->els = NULL;

	return 0;
}

int bufs_push(Bufs *it, Buf *el) {
	int ret = 0;

	it->els = (Buf**)realloc(it->els, (it->len + 1)*sizeof(Buf*));

	if (!it->els) { ret = 1; goto cleanup; }

	it->els[it->len] = el;
	it->len++;

cleanup:
	return ret;
}


int bufs_del(Bufs **out) {
	int ret = 0;
	Bufs *it = NULL;

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
