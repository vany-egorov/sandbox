#ifndef __COLLECTIONS_FIFO__
#define __COLLECTIONS_FIFO__


#include <stdio.h>     /* printf */
#include <string.h>    /* memcpy, size_t */
#include <stdlib.h>    /* malloc, calloc, free */
#include <stdint.h>    /* uint8_t, uint16_t */
#include <pthread.h>   /* pthread_mutex_lock, pthread_mutex_unlock */
#include <semaphore.h> /* sem_t, sem_init, sem_wait, sem_post, sem_destroy */


typedef struct fifo_s FIFO;

struct fifo_s {
	uint8_t *data;
	size_t len, cap,
	       start, finish;
	sem_t sem;
	pthread_mutex_t rw_mutex;
};

int fifo_new(FIFO **out);
int fifo_init(FIFO *it, size_t cap);
void fifo_print(FIFO *it);
void fifo_print_safe(FIFO *it);
void fifo_sprint_safe(FIFO *it, char *buf, size_t bufsz);
void fifo_print_data(FIFO *it);
void fifo_wait_data(FIFO *it);
/* impl Writer for FIFO */
int fifo_write(void *ctx, uint8_t *src, size_t srcsz, size_t *writesz);
size_t fifo_len(FIFO *it);
/* impl Reader for FIFO */
int fifo_read(void *ctx, uint8_t *dst, size_t dstsz, size_t *readsz);
void fifo_reset(FIFO *it);
void fifo_del(FIFO *it);


#endif /* __COLLECTIONS_FIFO__ */
