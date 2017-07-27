#ifndef __COLLECTIONS_SLICE__
#define __COLLECTIONS_SLICE__


#include <stdio.h>  /* printf */
#include <string.h> /* memcpy */
#include <stddef.h> /* size_t */
#include <stdlib.h> /* calloc, realloc */
#include <stdint.h> /* uint8_t, uint32_t */


#define SLICE_INITIAL_CAP    10
#define SLICE_REALLOC_FACTOR 0.5


typedef struct slice_s Slice;

struct slice_s {
	size_t cap;
	size_t len;
	size_t elsz;

	void *els;
};


/* alloc slice struct + init */
int slice_new(Slice **out, size_t elsz);
/* init already allocated struct */
int slice_init(Slice *it, size_t elsz);
/* alloc internal buffer */
int slice_prealloc(Slice *it);
int slice_append(Slice *it, const void *el);
int slice_get_copy_data(Slice *it, size_t index, void *el);
void *slice_get(Slice *it, size_t index);
void *slice_tail(Slice *it);
void *slice_head_free(Slice *it);
int slice_tail_copy_data(Slice *it, void *el);
int slice_del_el(Slice *it, size_t idx);
int slice_del_els(Slice *it, size_t f, size_t t);  /* f == from, t == to */
void slice_print(Slice *it);
int slice_fin(Slice *it);
int slice_del(Slice **out);


#endif  /* __COLLECTIONS_SLICE__ */
