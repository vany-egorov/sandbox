#include "fifo.h"


int fifo_new(FIFO **out) {
	int ret = 0;
	FIFO *it = NULL;

	it = (FIFO*)calloc(1, sizeof(FIFO));
	if (!it) return 1;
	*out = it;

	return ret;
}

int fifo_init(FIFO *it, size_t cap) {
	it->cap = cap;
	it->len = 0;
	it->start = 0;
	it->finish = 0;
	it->data = (uint8_t*)calloc(1, cap);
	if (!it->data) return 1;
	sem_init(&it->sem, 0, 0);

	return 0;
}

void fifo_print(FIFO *it) {
	printf(
		"{\"start\": %zd"
		", \"finish\": %zd"
		", \"length (len)\": %zd"
		", \"capacity (cap)\": %zd"
		"}\n",
		it->start,
		it->finish,
		it->len,
		it->cap
	);
}

void fifo_print_safe(FIFO *it) {
	pthread_mutex_lock(&it->rw_mutex);
	fifo_print(it);
	pthread_mutex_unlock(&it->rw_mutex);
}

void fifo_sprint_safe(FIFO *it, char *buf, size_t bufsz) {
	if (!it) return;

	pthread_mutex_lock(&it->rw_mutex);
	snprintf(buf, bufsz, "{"
		"\"start\": %zd"
		", \"finish\": %zd"
		", \"length (len)\": %zd"
		", \"capacity (cap)\": %zd"
		"}",
		it->start,
		it->finish,
		it->len,
		it->cap
	);
	pthread_mutex_unlock(&it->rw_mutex);
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

void fifo_wait_data(FIFO *it) {
	sem_wait(&it->sem);
}

size_t fifo_len(FIFO *it) {
	size_t len = 0;
	pthread_mutex_lock(&it->rw_mutex);
	len = it->len;
	pthread_mutex_unlock(&it->rw_mutex);
	return len;
}

// TODO: support multiple overflow
// impl Writer for FIFO
int fifo_write(void *ctx, uint8_t *src, size_t srcsz, size_t *writesz) {
	FIFO *it = NULL;
	int ret = 0;
	size_t finish_nxt = 0,
	       offset = 0;

	it = (FIFO*)ctx;

	pthread_mutex_lock(&it->rw_mutex);
	finish_nxt = (it->finish + srcsz) % it->cap;

	if (it->finish + srcsz <= it->cap) {
		memcpy(it->data + it->finish, src, srcsz);
	} else { // split data into two segments
		offset = it->cap - it->finish;
		                                                       // [0 .............................. cap]
		memcpy(it->data + it->finish, src,        offset);     // [..................... finish ... cap]
		memcpy(it->data,              src+offset, finish_nxt); // [0 ... finish-nxt ............... cap]
	}

	it->finish = finish_nxt;
	it->len += srcsz;

	if (it->len > it->cap) { // fifo overflow
		it->len = it->cap;
		it->start = it->finish;
		fprintf(stderr, "[fifo @ %p] fifo overflow\n", it);
	}

	pthread_mutex_unlock(&it->rw_mutex);
	sem_post(&it->sem);

	*writesz = srcsz;
	return ret;
}

// TODO: support multiple overflow
// TODO: check overflow (MLR for mpegts detected when not chunked read)
// impl Reader for FIFO
int fifo_read(void *ctx, uint8_t *dst, size_t dstsz, size_t *readsz) {
	int ret = 0;
	FIFO *it = NULL;
	size_t offset = 0;

	it = (FIFO*)ctx;

	pthread_mutex_lock(&it->rw_mutex);

	if (!it->len) {
		*readsz = 0;
		pthread_mutex_unlock(&it->rw_mutex);
		return ret;
	}

	if (dstsz > it->len) dstsz = it->len;

	if (it->start + dstsz < it->cap) {
		memcpy(dst, it->data + it->start, dstsz);
		memset(it->data + it->start, 0, dstsz);
	} else { // split data into two segments
		offset = it->cap - it->start;

		memcpy(dst,        it->data + it->start, offset);
		memcpy(dst+offset, it->data            , dstsz-offset);

		memset(it->data + it->start, 0, offset);
		memset(it->data, 0, dstsz-offset);
	}

	it->start = (it->start + (size_t)dstsz) % it->cap;
	it->len -= dstsz;
	pthread_mutex_unlock(&it->rw_mutex);

	*readsz = dstsz;
	return ret;
}

void fifo_reset(FIFO *it) {
	pthread_mutex_lock(&it->rw_mutex);
	memset(it->data, 0, it->cap);
	it->len = 0;
	it->start = 0;
	it->finish = 0;
	pthread_mutex_unlock(&it->rw_mutex);
}

void fifo_del(FIFO *it) {
	if (it) {
		if (it->data) {
			free(it->data);
			it->data = NULL;
		}
		sem_destroy(&it->sem);
		free(it);
	}
}
