#include <stddef.h>    // size_t
#include <semaphore.h> // sem_t


typedef struct chan_s Chan;


struct chan_s {
	size_t len, cap;

	size_t el_sz;
	void *els;

	sem_t sem;
};
