#include "chan.h"


int chan_new(Chan **out, size_t cap, size_t el_sz) {
	Chan *it = NULL;

	it = calloc(1, sizeof(Chan));
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
