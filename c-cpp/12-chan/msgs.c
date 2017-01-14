#include "msgs.h"


int msgs_new(Msgs **out, size_t cap, size_t elsz) {
	Msgs *it = NULL;

	it = calloc(1, sizeof(Msgs));
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

int msgs_get_available(Msgs *it, void **out) {
	int ret = 0;

	*out = &(((char*)it->els)[it->finish*it->elsz]);

	it->finish = (it->finish + 1) % it->cap;
	it->len += 1;

	if (it->len > it->cap) { // overflow
		it->len = it->cap;
		it->start = it->finish;
	}

	return ret;
}

int msgs_got_available(Msgs *it) { return (it->len == it->cap) ? 0 : 1; }
int msgs_is_empty(Msgs *it) { return (it->len == 0) ? 1 : 0; }

void msgs_del(Msgs *it) {
	if (!it) return;
	if (it->els != NULL) {
		free(it->els);
		it->els = NULL;
	}
	free(it);
}
