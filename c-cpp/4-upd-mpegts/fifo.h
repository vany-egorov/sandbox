#ifndef __FIFO__
#define __FIFO__


#include <string.h> // memcpy, size_t
#include <stdint.h> // uint8_t, uint16_t
#include <semaphore.h> // sem_t


typedef struct _FIFO {
	uint8_t *data;
	size_t len, cap,
	       start, finish;
	sem_t sem;
	pthread_mutex_t rw_mutex;
} FIFO;

FIFO *fifo_new(size_t cap);
void fifo_print(FIFO *it);
void fifo_print_data(FIFO *it);
void fifo_delete(FIFO *it);
void fifo_wait_data(FIFO *it);
size_t fifo_write(FIFO *it, uint8_t *src, size_t src_len);
void fifo_read(FIFO *it, uint8_t *dst, size_t dst_len, size_t *readed_len);


#endif // __FIFO__
