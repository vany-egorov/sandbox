#ifndef __IO_BUF__
#define __IO_BUF__

/* TODO: into-reader, into-writer func + virtual-table */
typedef struct buf_s Buf;

struct buf_s {
	uint8_t *d;  /* data */
	size_t sz;   /* size */
};

#endif // __IO_BUF__
