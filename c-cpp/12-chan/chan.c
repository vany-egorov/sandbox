#include "chan.h"


int chan_new(Chan **out, size_t cap, size_t msgsz) {
	Chan *it = NULL;

	it = calloc(1, sizeof(Chan));
	if (!it) {
		return 1;
	}
	*out = it;

	it->len = 0;
	it->cap = cap;
	it->start = 0;
	it->finish = 0;

	it->msgsz = msgsz;
	it->msgs = calloc(it->cap, it->msgsz);

	sem_init(&it->sem, 0, 0);

	return 0;
}

void chan_wait(Chan *it) {
	sem_wait(&it->sem);
}

int chan_send(Chan *it, void *msg) {
	int ret = 0;

	pthread_mutex_lock(&it->rw_mutex);
	memcpy(it->msgs + (it->finish*it->msgsz), msg, it->msgsz);

	ret = chan_send_silent(it, msg);

	pthread_mutex_unlock(&it->rw_mutex);
	chan_notify(it);

	return ret;
}

/*
 * no thread mutex lock
 * no cond-variable/semaphore wake up
 */
int chan_send_silent(Chan *it, void *msg) {
	int ret = 0;

	memcpy(it->msgs + (it->finish*it->msgsz), msg, it->msgsz);

	it->finish = (it->finish + 1) % it->cap;
	it->len++;

	if (it->len > it->cap) { // chan overflow
		it->len = it->cap;
		it->start = it->finish;
	}

	return ret;
}

void chan_notify(Chan *it) { sem_post(&it->sem); }

int chan_recv(Chan *it, void *msg) {
	int ret = 0;

	pthread_mutex_lock(&it->rw_mutex);

	ret = chan_recv_silent(it, msg);

	pthread_mutex_unlock(&it->rw_mutex);

	return ret;
}

/*
 * no thread mutex lock
 * no cond-variable/semaphore wake up
 */
int chan_recv_silent(Chan *it, void *msg) {
	int ret = 0;

	if (!it->len) return ret;

	memcpy(msg, it->msgs + (it->start*it->msgsz), it->msgsz);
	memset(it->msgs + (it->start*it->msgsz), 0, it->msgsz);

	it->start = (it->start + 1) % it->cap;
	it->len--;

	return ret;
}

inline int chan_got_msg(Chan *it) { return it->len ? 1 : 0; }

int chan_del(Chan **out) {
	if (!out) return;

	Chan *it = *out;

	if (!it) return;

	sem_destroy(&it->sem);

	if (it->msgs) free(it->msgs);
	free(it);

	*out = NULL;
}
