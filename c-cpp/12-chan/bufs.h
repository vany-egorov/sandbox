#ifndef __BUFS_H__
#define __BUFS_H__


#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* malloc, calloc, free */

#include "./buf.h"


typedef struct bufs_c Bufs;


struct bufs_c {
	size_t len;
	Buf **els;
};


int bufs_new(Bufs **out);
int bufs_push(Bufs *it, Buf *el);
int bufs_del(Bufs **out);


#endif /* __BUFS_H__ */
