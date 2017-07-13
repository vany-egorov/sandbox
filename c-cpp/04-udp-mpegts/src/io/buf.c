#include "buf.h"


int buf_new(Buf **out) {
	int ret = 0;
	Buf *it = NULL;

	it = (Buf*)calloc(1, sizeof(Buf));
	if (!it) return 1;

	it->len = 0;
	it->cap = 0;

	*out = it;
	return ret;
}

int buf_init(Buf *it, size_t cap) {
	it->cap = cap;
	it->len = 0;
	it->v = calloc(it->cap, 1);
}

int buf_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
}

int buf_write(void *ctx, uint8_t *src, size_t srcsz, size_t *n) {
	int ret = 0;
	size_t cap_new = 0;
	Buf *it = NULL;

	it = (Buf*)ctx;

	if ((it->cap - it->len) <= srcsz) {
		cap_new = it->cap + srcsz + (size_t)(it->cap * BUF_REALLOC_FACTOR);
		it->v = realloc(it->v, cap_new);
		it->cap = cap_new;
	}

	memcpy(it->v + it->len, src, srcsz);
	it->len += srcsz;

cleanup:
	return ret;
}

int buf_reset(Buf *it) {
	memset(it->v, 0, it->cap);
	it->len = 0;
}

int buf_fin(Buf *it) {
	if (it->v) {
		free(it->v);
		it->v = NULL;
	}

	it->len = 0;
	it->cap = 0;
}

int buf_del(Buf **out) {
	int ret = 0;
	Buf *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	buf_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
