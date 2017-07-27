#include "slice.h"


int slice_new(Slice **out, size_t elsz) {
	int ret = 0;
	Slice *it = NULL;

	it = calloc(1, sizeof(Slice));
	if (!it) {
		*out = NULL;
		ret = 1; goto cleanup;
	}

	*out = it;

	slice_init(it, elsz);

cleanup:
	return 0;
}

int slice_init(Slice *it, size_t elsz) {
	it->len = 0;
	it->cap = 0;
	it->elsz = elsz;
	it->els = NULL;
}

int slice_prealloc(Slice *it) {
	it->els = calloc(SLICE_INITIAL_CAP, it->elsz);
	if (!it->els) {
		return 1;
	}
	it->cap = SLICE_INITIAL_CAP;

	return 0;
}

extern inline void *slice_head_free(Slice *it) {
	if (!it) return NULL;
	return &(((char*)it->els)[it->elsz*it->len]);
}

extern inline void *slice_tail(Slice *it) {
	if (!it) return NULL;
	if (!it->len) return it->els;
	return &(((char*)it->els)[it->elsz*(it->len-1)]);
}

int slice_tail_copy_data(Slice *it, void *el_out) {
	void *el = NULL;

	el = slice_head_free(it);
	memcpy(el_out, el, it->elsz);

	return 0;
}

/* TODO: handle not enought memory */
int slice_append(Slice *it, const void *el) {
	size_t cap_new = 0;
	void *els_new = NULL;

	if (!it->cap)
		slice_prealloc(it);

	if (it->len == it->cap) { // realloc
		cap_new = it->cap + (size_t)(it->cap * SLICE_REALLOC_FACTOR);
		els_new = realloc(it->els, cap_new*it->elsz);

		/* not enought memory */
		if (!els_new) return 1;

		it->els = els_new;
		it->cap = cap_new;
	}

	memcpy(slice_head_free(it), el, it->elsz);
	it->len++;

	return 0;
}

int slice_get_copy_data(Slice *it, size_t index, void *el_out) {
	void *el = NULL;

	if (it->len <= index) return 1;

	el = &(((char*)it->els)[it->elsz*index]);
	memcpy(el_out, el, it->elsz);

	return 0;
}

extern inline void *slice_get(Slice *it, size_t index) {
	if (it->len <= index) return NULL;
	return &(((char*)it->els)[it->elsz*index]);;
}

void slice_print(Slice *it) {
	size_t i = 0,
	       j = 0;
	char* el = NULL;

	for (i = 0; i < it->len; i++) {
		el = &(((char*)it->els)[it->elsz*i]);

		if (i != 0) printf("\n");

		for (j = 0; j < it->elsz; j++) {
			printf("%02X ", el[j]);
		}
	}

	printf("\n");
}

int slice_del_el(Slice *it, size_t idx) {
	int ret = 0;
	char *el = NULL,      /* el to del */
	     *el_nxt = NULL,  /* next el to shift back */
	     *el_lst = NULL;  /* last el */

	if (idx >= it->len) {
		ret = 1;  /* no such element to delete found */
		goto cleanup;
	}

	el = &(((char*)it->els)[it->elsz*idx]);

	el_lst = &(((char*)it->els)[it->elsz*(it->len-1)]);

	/* if not last element
	 * if last element to delete - nothing to shift, only memset
	 */
	if (it->len != idx+1) {
		el_nxt = &(((char*)it->els)[it->elsz*(idx+1)]);

		/* TODO: error handling; */
		/* shift data back */
		void *dst = memmove(
			  (void*)el      /* destination */
			, (void*)el_nxt  /* source */
			, (it->len - (idx+1)) * it->elsz
		);
	}

	/* TODO: error handling; */
	/* clear freed/shifted memory */
	memset(el_lst, 0, it->elsz);
	it->len--;

cleanup:
	return ret;
}

int slice_del_els(Slice *it, size_t f, size_t t) {

}

int slice_fin(Slice *it) {
	if (it->els) {
		free(it->els);
		it->els = NULL;
		it->len = 0;
		it->cap = 0;
	}

	return 0;
}

int slice_del(Slice **out) {
	int ret = 0;
	Slice *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	slice_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
