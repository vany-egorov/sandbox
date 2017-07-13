#ifndef __IO_BUF__
#define __IO_BUF__


#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint8_t */
#include <stdlib.h>  /* calloc, free */
#include <string.h>  /* memcpy */


#define BUF_REALLOC_FACTOR 0.5


/* TODO: into-reader, into-writer func + virtual-table */
typedef struct buf_s Buf;

struct buf_s {
	uint8_t *v;  /* value */
	size_t cap;  /* capacity */
	size_t len;  /* length */
};


int buf_new(Buf **out);
int buf_init(Buf *it, size_t cap);
/* impl Reader for File */
int buf_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n);
/* impl Writer for Buf */
int buf_write(void *ctx, uint8_t *src, size_t srcsz, size_t *n);
int buf_reset(Buf *it);
int buf_fin(Buf *it);
int buf_del(Buf **out);


#endif /* __IO_BUF__ */
