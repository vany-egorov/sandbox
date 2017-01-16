#include "buf.h"


int buf_new(Buf **out, size_t cap, size_t elsz) {
	Buf *it = NULL;

	it = calloc(1, sizeof(Buf));
	if (!it) {
		return 1;
	}
	*out = it;

	it->cap = cap;
	it->len = 0;
	it->start = 0;
	it->finish = 0;
	it->elsz = elsz;
	it->els = calloc(it->cap, it->elsz);

	return 0;
}

int buf_get_available(Buf *it, void **out) {
	int ret = 0;

	*out = &(((char*)it->els)[it->finish*it->elsz]);

	it->finish = (it->finish + 1) % it->cap;
	it->len++;

	if (it->len > it->cap) { // overflow
		it->len = it->cap;
		it->start = it->finish;
	}

	return ret;
}

int buf_got_available(Buf *it) { return (it->len == it->cap) ? 0 : 1; }
int buf_is_empty(Buf *it) { return (it->len == 0) ? 1 : 0; }

void buf_del(Buf *it) {
	if (!it) return;
	if (it->els != NULL) {
		free(it->els);
		it->els = NULL;
	}
	free(it);
}
