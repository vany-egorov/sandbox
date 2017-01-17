#ifndef __BUF_H__
#define __BUF_H__


#include <stdio.h>   /* printf */
#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* malloc, calloc, free */


typedef struct buf Buf;


struct buf {
	size_t len, cap,
	       start, finish;

	size_t elsz;
	void *els;
};


int  buf_new(Buf **out, size_t cap, size_t elsz);
int  buf_get_available(Buf *it, void **out);
int  buf_got_available(Buf *it);
int  buf_is_empty(Buf *it);
int  buf_first(Buf *it, void **out);
void buf_del(Buf **out);


#endif /* __BUF_H__ */
