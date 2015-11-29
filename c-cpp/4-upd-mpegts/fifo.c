#include <stdio.h>
#include <string.h> // memcpy, size_t
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint8_t, uint16_t
#include <semaphore.h> // uint8_t, uint16_t

#include "fifo.h"


FIFO *fifo_new(size_t cap) {
	FIFO *it = (FIFO*)calloc(1, sizeof(FIFO));
	it->data = NULL;
	it->cap = cap;
	it->len = 0;
	it->start = 0;
	it->finish = 0;

	return it;
}

void fifo_print(FIFO *it) {
	printf(
		"{\"start\": %d"
		", \"finish\": %d"
		", \"length (len)\": %d"
		", \"capacity (cap)\": %d"
		"}\n",
		it->start,
		it->finish,
		it->len,
		it->cap
	);
}

void fifo_print_data(FIFO *it) {
	int i = 0;

	if (it->data) {
		for (i = 0; i < it->cap; i++) {
			printf("0x%02x ", it->data[i]);
		}
		printf("\n");

		for (i = 0; i < it->cap; i++) {
			if (i == it->start) {
				printf("s    ");
			} else {
				printf("     ");
			}
		}
		printf("\n");

		for (i = 0; i < it->cap; i++) {
			if (i == it->finish) {
				printf("f    ");
			} else {
				printf("     ");
			}
		}
		printf("\n");
	}
}

void fifo_delete(FIFO *it) {
	if (it) {
		if (it->data) {
			free(it->data);
			it->data = NULL;
		}
		free(it);
	}
}

size_t fifo_write(FIFO *it, uint8_t *src, size_t src_len) {
	if (!it->data) it->data = (uint8_t*)calloc(1, src_len);

	size_t finish_prv = it->finish;
	size_t finish_nxt = (it->finish + (size_t)src_len) % it->cap;

	void *result = memcpy(it->data + it->finish, src, src_len);

	it->finish = finish_nxt;
	it->len += src_len;

	if (it->len > it->cap) {
		it->len = it->cap;
		it->start = it->finish;
	}

	return src_len;
}

size_t fifo_read(FIFO *it, uint8_t *dst, size_t dst_len) {
	void *result = memcpy(dst, it->data, dst_len);
	memset(it->data, 0, dst_len);

	it->start = (it->start + (size_t)dst_len) % it->cap;
	it->len -= dst_len;

	return dst_len;
}
