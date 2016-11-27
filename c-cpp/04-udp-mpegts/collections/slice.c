#include "slice.h"


int slice_new(Slice **out, size_t el_size) {
	Slice *it = NULL;

	it = calloc(1, sizeof(Slice));
	*out = it;

	it->cap = SLICE_INITIAL_CAP;
	it->len = 0;
	it->el_size = el_size;
	it->els = calloc(SLICE_INITIAL_CAP, el_size);

	return 0;
}

inline void *slice_tail(Slice *it) {
	if (!it) return NULL;
	return &(((char*)it->els)[it->el_size*it->len]);
}

int slice_tail_copy_data(Slice *it, void *el_out) {
	void *el = NULL;

	el = slice_tail(it);
	memcpy(el_out, el, it->el_size);

	return 0;
}

// TODO: handle not enought memory
int slice_append(Slice *it, const void *el) {
	size_t cap_new = 0;

	if (it->len == it->cap) { // realloc
		cap_new = it->cap + (size_t)(it->cap * SLICE_REALLOC_FACTOR);
		it->els = realloc(it->els, cap_new*it->el_size);
		it->cap = cap_new;
	}

	memcpy(slice_tail(it), el, it->el_size);
	it->len++;

	return 0;
}

int slice_get_copy_data(Slice *it, size_t index, void *el_out) {
	void *el = NULL;

	if (it->len <= index) return 1;

	el = &(((char*)it->els)[it->el_size*index]);
	memcpy(el_out, el, it->el_size);

	return 0;
}

inline void *slice_get(Slice *it, size_t index) {
	if (it->len <= index) return NULL;
	return &(((char*)it->els)[it->el_size*index]);;
}

void slice_print(Slice *it) {
	size_t i = 0,
	       j = 0;
	char* el = NULL;

	for (i = 0; i < it->len; i++) {
		el = &(((char*)it->els)[it->el_size*i]);

		if (i != 0) printf("\n");

		for (j = 0; j < it->el_size; j++) {
			printf("%02X ", el[j]);
		}
	}

	printf("\n");
}

void slice_del(Slice *it) {
	if (!it) return;
	if (it->els != NULL) {
		free(it->els);
		it->els = NULL;
	}
	free(it);
}
