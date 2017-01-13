#include <stddef.h>    /* size_t */
#include <stdlib.h>    /* malloc, calloc, free */


typedef struct msgs Msgs;


struct msgs {
	size_t len, cap,
	       start, finish;

	size_t el_sz;
	void *els;
};


int  msgs_new(Msgs **out, size_t cap, size_t el_sz);
void msgs_del(Msgs **it);
