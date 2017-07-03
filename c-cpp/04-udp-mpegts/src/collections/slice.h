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
	size_t el_size;

	void *els;
};


int slice_new(Slice **out, size_t el_size);
int slice_append(Slice *it, const void *el);
int slice_get_copy_data(Slice *it, size_t index, void *el);
void *slice_get(Slice *it, size_t index);
void *slice_tail(Slice *it);
int slice_tail_copy_data(Slice *it, void *el);
void slice_print(Slice *it);
void slice_del(Slice *it);


#endif
