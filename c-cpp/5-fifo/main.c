#include <stdio.h>
#include <string.h> // memcpy
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint8_t, uint16_t


#define FIFO_CAP 5
#define DST_LEN 10


typedef struct _FIFO {
	uint8_t *data;
	size_t cap;
	size_t start;
	size_t finish;
} FIFO;

FIFO *fifo_new(size_t cap) {
	FIFO *it = (FIFO*)calloc(1, sizeof(FIFO));
	it->data = NULL;
	it->cap = cap;
	it->start = 0;
	it->finish = 0;

	return it;
}

size_t fifo_len(FIFO *it) {
	if (it->finish > it->start) {
		return it->finish - it->start;
	}

	return it->cap - (it->finish - it->start);
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
	if (!it->data) {
		it->data = (uint8_t*)calloc(1, src_len);
	}
	void *result = memcpy(it->data + it->finish, src, src_len);
	it->finish = (it->finish + (size_t)src_len) % it->cap;
	return src_len;
}

size_t fifo_read(FIFO *it, uint8_t *dst, size_t dst_len) {
	void *result = memcpy(dst, it->data, dst_len);
	it->start = (it->start + (size_t)dst_len) % it->cap;
	return dst_len;
}

int main(int argc, char const *argv[]) {
	int i = 0;
	FIFO *fifo = NULL;

	fifo = fifo_new(FIFO_CAP);

	uint8_t src1[] = {0x11, 0x12, 0x13, 0x14};
	fifo_write(fifo, src1, 4);

	uint8_t dst1[DST_LEN];
	fifo_read(fifo, dst1, DST_LEN);

	for (i=0; i<DST_LEN; i++) {
		printf("0x%02x ", dst1[i]);
	}
	printf("\n");
	printf("%d\n", fifo_len(fifo));

	uint8_t src2[] = {0x21, 0x22, 0x23, 0x24};
	fifo_write(fifo, src2, 4);

	uint8_t dst2[DST_LEN];
	fifo_read(fifo, dst2, DST_LEN);

	for (i=0; i<DST_LEN; i++) {
		printf("0x%02x ", dst2[i]);
	}
	printf("\n");
	printf("%d\n", fifo_len(fifo));


	fifo_delete(fifo);
	exit(EXIT_SUCCESS);
}
