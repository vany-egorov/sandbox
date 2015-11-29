#include <stdio.h>
#include <string.h> // memcpy
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint8_t, uint16_t


#define FIFO_CAP 8
#define DST_LEN 4


typedef struct _FIFO {
	uint8_t *data;
	size_t len, cap,
	       start, finish;
} FIFO;

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
	int i = 0;

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


	// // finish cross start
	// if (
	// 	// 1 | finish-prv ..... start
	// 	// 2 | finish-prv ..... start ..... finish-nxt
	// 	// 3 |                              finish-nxt
	// 	//                                  start
	// 	(
	// 		(finish_prv < finish_nxt) &&
	// 		(
	// 			(it->start > finish_prv) &&
	// 			(it->start <= finish_nxt)
	// 		)
	// 	// 1 | ..... start ..... finish-prv
	// 	// 2 | ..... start ..... finish-nxt ..... finish-prv
	// 	// 3 |                   finish-nxt
	// 	//                       start
	// 	) || (
	// 		(finish_prv > finish_nxt) &&
	// 		(it->finish >= it->start)
	// 	)
	// )
	// 	it->start = it->finish;

	// // finish cross start
	// // 1
	// // 1.1 | finish-prv ..... start
	// // 1.2 | finish-prv ..... start ..... finish-nxt
	// // 1.3 |                              finish-nxt
	// //                                    start
	// // 2
	// // 2.1 | ..... start ..... finish-prv
	// // 2.2 | ..... start ..... finish-nxt ..... finish-prv
	// // 2.3 |                   finish-nxt
	// //                         start
	// uint8_t sgn_prv = finish_prv > it->start;
	// uint8_t sgn_nxt = finish_nxt > it->start;
	// if (sgn_prv != sgn_nxt)
	// 	it->start = it->finish;

	return src_len;
}

size_t fifo_read(FIFO *it, uint8_t *dst, size_t dst_len) {
	void *result = memcpy(dst, it->data, dst_len);
	memset(it->data, 0, dst_len);

	it->start = (it->start + (size_t)dst_len) % it->cap;
	it->len -= dst_len;

	return dst_len;
}

int main(int argc, char const *argv[]) {
	int i = 0;
	FIFO *fifo = NULL;

	fifo = fifo_new(FIFO_CAP);
	fifo_print(fifo);

	uint8_t src1[] = {0x11, 0x12, 0x13, 0x14};
	fifo_write(fifo, src1, 4);
	fifo_print(fifo);

	uint8_t dst1[DST_LEN] = { 0 };
	fifo_read(fifo, dst1, DST_LEN);
	fifo_print(fifo);

	for (i=0; i<DST_LEN; i++) {
		printf("0x%02x ", dst1[i]);
	}
	printf("\n");

	uint8_t src2[] = {0x21, 0x22, 0x23, 0x24};
	fifo_write(fifo, src2, 4);
	fifo_print(fifo);

	uint8_t src3[] = {0x31, 0x32, 0x33, 0x34};
	fifo_write(fifo, src3, 4);
	fifo_print(fifo);

	uint8_t src4[] = {0x41, 0x42, 0x43, 0x44};
	fifo_write(fifo, src4, 4);
	fifo_print(fifo);

	uint8_t src5[] = {0x51, 0x52, 0x53, 0x54};
	fifo_write(fifo, src5, 4);
	fifo_print(fifo);

	// uint8_t dst2[DST_LEN] = { 0 };
	// fifo_read(fifo, dst2, 4);

	// for (i=0; i<DST_LEN; i++) {
	// 	printf("0x%02x ", dst2[i]);
	// }
	// printf("\n");
	// printf("len->%d start->%d finish->%d\n", fifo_len(fifo), fifo->start, fifo->finish);


	fifo_delete(fifo);
	exit(EXIT_SUCCESS);
}
