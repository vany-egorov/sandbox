#ifndef __FIFO__
#define __FIFO__


#include <string.h> // memcpy, size_t
#include <stdint.h> // uint8_t, uint16_t


typedef struct _FIFO {
	uint8_t *data;
	size_t len, cap,
	       start, finish;
} FIFO;

FIFO *fifo_new(size_t cap);
void fifo_print(FIFO *it);
void fifo_print_data(FIFO *it);
void fifo_delete(FIFO *it);
size_t fifo_write(FIFO *it, uint8_t *src, size_t src_len);
size_t fifo_read(FIFO *it, uint8_t *dst, size_t dst_len);


#endif // __FIFO__
