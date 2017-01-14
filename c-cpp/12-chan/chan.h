#ifndef __CHAN_H__
#define __CHAN_H__


#include <stddef.h>    /* size_t */
#include <semaphore.h> /* sem_t */
#include <stdlib.h>    /* malloc, calloc, free */


typedef struct chan_s Chan;


struct chan_s {
	size_t len, cap;

	size_t el_sz;
	void *els;

	sem_t sem;
};


int  chan_new(Chan **out, size_t cap, size_t el_sz);
void chan_del(Chan **it);


#endif /* __CHAN_H__ */
