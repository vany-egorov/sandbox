#include "msgs.h"


int msgs_new(Msgs **out, size_t el_sz, size_t cap) {
	Msgs *it = NULL;

	it = calloc(1, sizeof(Msgs));
	if (!it) {
		return 1;
	}
	*out = it;

	it->cap = cap;
	it->len = 0;
	it->el_sz = el_sz;
	it->els = calloc(it->cap, it->el_sz);

	return 0;
}
