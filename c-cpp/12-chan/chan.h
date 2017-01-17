#ifndef __CHAN_H__
#define __CHAN_H__


#include <stddef.h>    /* size_t */
#include <semaphore.h> /* sem_t */
#include <stdlib.h>    /* malloc, calloc, free */
#include <string.h>    /* memcpy */
#include <pthread.h>   /* pthread_mutex_lock, pthread_mutex_unlock */


typedef struct chan_s Chan;


struct chan_s {
	size_t len, cap,
	       start, finish;

	size_t msgsz;
	void  *msgs;

	sem_t sem;

	pthread_mutex_t rw_mutex;
};


int        chan_new(Chan **out, size_t cap, size_t msgsz);
void       chan_wait(Chan *it);
int        chan_send(Chan *it, void *msg);
int        chan_send_silent(Chan *it, void *msg);
int        chan_recv_silent(Chan *it, void *msg);
inline int chan_got_msg(Chan *it);
void       chan_notify(Chan *it);
int        chan_recv(Chan *it, void *msg);
int        chan_del(Chan **out);


#endif /* __CHAN_H__ */
