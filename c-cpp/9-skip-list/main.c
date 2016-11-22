#include <time.h>  // time
#include <stdio.h>
#include <stdint.h>   // uint8_t, uint16_t
#include <unistd.h>   // getpid, getppid
#include <stdlib.h>   // srand, rand
#include <sysexits.h> // EX_OK, EX_SOFTWARE
#include <inttypes.h> // PRId64


struct skip_list_list_s {
	size_t    l;   // length
	size_t    c;   // capacity
	uint64_t *es;  // elements
};

typedef struct skip_list_list_s SkipListList;

void skip_list_list_init(SkipListList *it) {
	it->l = 0;
	it->c = 2048;
	it->es = calloc(it->c, sizeof(uint64_t));

	int i = 0;
	for (i = 0; i < it->c; i++) {
		printf("%d ", it->es[i]);
	}
	printf("\n");
}

struct skip_list_s {
	size_t        levels;
	SkipListList *es;      // elements
};

typedef struct skip_list_s SkipList;

void new_skip_list(SkipList **out, size_t levels) {
	int i = 0;
	SkipList *it = NULL;

	it = calloc(1, sizeof(SkipList));
	it->levels = levels;
	it->es = calloc(levels, sizeof(SkipListList));
	for (i = 0; i < levels; i++)
		skip_list_list_init(&it->es[i]);

	*out = it;
}

int qsort_comp(const void *a, const void *b) {
	uint64_t i, j;
	i = *(uint64_t*)a;
	j = *(uint64_t*)b;

	return i - j;
}


int main(int argc, char const *argv[]) {
	int ret = EX_OK;
	int i = 0;

	uint64_t timestamps[1000] = { 0 };
	SkipList *sl = NULL;

	srand(time(NULL) + getpid() + getppid());
	new_skip_list(&sl, 5);
	printf("skip-list %p\n", sl);

	for (i = 0; i < 1000; i++) timestamps[i] = rand() % 10000;

	qsort(timestamps, 1000, sizeof(uint64_t), qsort_comp);

	for (i = 0; i < 1000; i++)
		printf("%4"PRId64" ", timestamps[i]);

	return ret;
}
