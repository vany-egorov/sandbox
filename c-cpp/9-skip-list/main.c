#include <time.h>  // time
#include <stdio.h>
#include <stdint.h>   // uint8_t, uint16_t
#include <unistd.h>   // getpid, getppid
#include <stdlib.h>   // srand, rand
#include <sysexits.h> // EX_OK, EX_SOFTWARE
#include <inttypes.h> // PRId64
#include <math.h>     // pow


struct skip_list_node_s {
	uint64_t key;
	size_t   lower_index;
};

typedef struct skip_list_node_s SkipListNode;

struct skip_list_row_s {
	size_t        level;
	double        p;
	size_t        len;
	size_t        cap;
	SkipListNode *nodes;
};

typedef struct skip_list_row_s SkipListRow;

void skip_list_row_push(SkipListRow *it, uint64_t key, size_t lower_index) {
	size_t cap_new = 0;
	if (it->cap == it->len) {
		if (!it->cap) {
			cap_new = 2;
		} else {
			cap_new = it->cap + (size_t)((double)it->cap * 0.5);
			if (cap_new == it->cap) cap_new+=2;
		}

		printf("realloc #%d {%d => %d}\n", it->level, it->cap, cap_new);
		it->nodes = realloc(it->nodes, (cap_new)*sizeof(SkipListNode));
		it->cap = cap_new;
	}

	it->nodes[it->len].key = key;
	it->nodes[it->len].lower_index = lower_index;
	it->len++;
}

void skip_list_list_init(SkipListRow *it, size_t cap, double p, size_t level) {
	int i = 0;
	it->level = level;
	it->p = p;
	it->len = 0;
	it->cap = cap;
	it->nodes = calloc(it->cap, sizeof(SkipListNode));
}

struct skip_list_s {
	double       p;
	size_t       levels;
	SkipListRow *rows;
};

typedef struct skip_list_s SkipList;

void skip_list_push(SkipList *it, uint64_t key) {
	size_t i = 0;
	size_t level = 1;
	while ((((double)rand() / (double)RAND_MAX) < it->p) && (level < it->levels))
		level++;

	size_t lower_index = 0;
	for (i = 0; i < level; i++) {
		skip_list_row_push(&it->rows[i], key, lower_index);
		lower_index = it->rows[i].len-1;
	}
}

void skip_list_print(SkipList *it) {
	size_t i = 0;
	size_t level = 0;
	SkipListRow *row = NULL;
	for (level = 0; level < it->levels; level++) {
		row = &it->rows[level];

		printf("%p %7d %7d %f\n", row, row->len, row->cap, row->p);

		// for (i = 0; i < row->len; i++)
		// 	printf("%" PRId64 ":%d ", row->nodes[i].key, row->nodes[i].lower_index);
		// printf("\n");
		// printf("\n");
		// printf("\n");
	}
}

int skip_list_search(SkipList *it, uint64_t key) {
	size_t level = 0;
	size_t i = 0;
	size_t lower_index = 0;
	size_t res = 0;
	SkipListRow *row = NULL;

	for (level = it->levels; level-- > 0;) {
		row = &it->rows[level];

		for (i = lower_index; i < row->len; i++) {
			printf("%2d | %d\n", level, i);
			if (row->nodes[i].key > key) {
				if (i != 0)
					i--;

				lower_index = row->nodes[i].lower_index;
				if (level == 0) res = i;
				break;
			}
		}
	}

	SkipListNode *node_prv = &it->rows[0].nodes[res-1];
	SkipListNode *node = &it->rows[0].nodes[res];
	SkipListNode *node_next = &it->rows[0].nodes[res+1];
	printf("     index: %d, key: %d\n", res-1, node_prv->key);
	printf("---> index: %d, key: %d\n", res, node->key);
	printf("     index: %d, key: %d\n", res+1, node_next->key);

	if (node->key == key) {
		printf("YEP!\n");
	} else {
		printf("NOPE!\n");
	}
}

void new_skip_list(SkipList **out, size_t levels, size_t initial_list_cap, double p) {
	size_t level = 0;
	size_t row_cap = 0;
	double row_p = 0;

	SkipList *it = NULL;
	it = calloc(1, sizeof(SkipList));
	it->levels = levels;
	it->p = p;
	it->rows = calloc(levels, sizeof(SkipListRow));
	for (level = 0; level < levels; level++) {
		row_p = pow(it->p, (double)level);
		row_cap = (size_t)((double)initial_list_cap * row_p);
		printf("------> %d | %f | %d\n", initial_list_cap, row_p, row_cap);
		skip_list_list_init(&it->rows[level], row_cap, row_p, level);
	}

	*out = it;
}

int qsort_comp(const void *a, const void *b) {
	uint64_t i, j;
	i = *(uint64_t*)a;
	j = *(uint64_t*)b;

	return i - j;
}


#define TIMESTAMPS_COUNT 10000000


int main(int argc, char const *argv[]) {
	int ret = EX_OK;
	int i = 0;

	uint64_t *timestamps = calloc(TIMESTAMPS_COUNT, sizeof(uint64_t));
	SkipList *sl = NULL;

	srand(time(NULL) + getpid() + getppid());
	new_skip_list(&sl, 20, 10240, 0.5);
	printf("skip-list %p\n", sl);

	for (i = 0; i < TIMESTAMPS_COUNT; i++) timestamps[i] = rand() % TIMESTAMPS_COUNT;

	qsort(timestamps, TIMESTAMPS_COUNT, sizeof(uint64_t), qsort_comp);

	for (i = 0; i < TIMESTAMPS_COUNT; i++)
		skip_list_push(sl, timestamps[i]);
		// printf("%4"PRId64" ", timestamps[i]);

	skip_list_print(sl);
	skip_list_search(sl, 1000000);

	skip_list_search(sl, 1200000);

	return ret;
}
