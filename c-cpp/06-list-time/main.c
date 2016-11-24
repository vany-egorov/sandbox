#include <stdio.h>
#include <string.h> // memcpy
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint8_t, uint16_t


// typedef struct _MLRCB {
//     uint32_t *data;
//     size_t len, cap,
//            start, finish;
//     pthread_mutex_t rw_mutex;
// };
// typedef struct _MLRCtx MLRCB;

// MLRCB *mlr_cb_new(size_t cap) {
//     MLRCB *it = (MLRCB*)calloc(1, sizeof(MLRCB));

//     it->data = NULL;
//     it->len = 0;
//     it->cap = 0;
//     it->start = 0;
//     it->finish = 0;

//     return it;
// }

// void mlr_cb_delete(MLRCB *it) {
//     if (it) {
//         if (it->data) {
//             free(it->data);
//             it->data = NULL;
//         }
//         free(it);
//     }
// }

// void mlr_cb_write(MLRCB *it, uint32_t *v) {
//     pthread_mutex_lock(&it->rw_mutex);

//     if (!it->data) it->data = (uint32_t*)calloc(1, sizeof(uint32_t));
//     memcpy(it->data + it->finish, v, sizeof(uint32_t));

//     it->finish += 1;
//     it->len += 1;
//     it->cap += 1;

//     pthread_mutex_unlock(&it->rw_mutex);

//     return src_len;
// }


typedef struct _MLRDrop MLRDrop;
typedef struct _MLRDrop {
	uint32_t v;
	MLRDrop *next;
} MLRDrop;

typedef struct _MLRDrops {
	MLRDrop *head,
	        *tail;
	size_t len;
	pthread_mutex_t rw_mutex;
} MLRDrops;

MLRDrops *mlr_drops_new(void) {
	MLRDrops *it = (MLRDrops*)calloc(1, sizeof(MLRDrops));

	it->head = NULL;
	it->tail = NULL;
	it->len = 0;

	return it;
}

void mlr_drops_delete(MLRDrops *it) {
	if (it) {
		if (it->head) {
			MLRDrop *head = it->head;
			while(head) {
				MLRDrop *el = head;
				head = head->next;
				free(el);
			}
		}
		free(it);
	}
}

void mlr_drops_push(MLRDrops *it, uint32_t v) {
	pthread_mutex_lock(&it->rw_mutex);

	MLRDrop *el = NULL;
	el = (MLRDrop*)calloc(1, sizeof(MLRDrop));
	el->v = v;
	el->next = NULL;

	if (!it->head) {
		it->head = el;
		it->tail = el;
	} else {
		it->tail->next = el;
		it->tail = el;
	}

	it->len++;

	pthread_mutex_unlock(&it->rw_mutex);
}

void mlr_drops_pull(MLRDrops *it, uint32_t *v) {
	pthread_mutex_lock(&it->rw_mutex);

	if (!it->head) {
		*v = 0;
		pthread_mutex_unlock(&it->rw_mutex);
		return;
	}

	MLRDrop *el = it->head;
	*v = el->v;

	it->head = el->next;
	it->len--;
	free(el);

	pthread_mutex_unlock(&it->rw_mutex);
}

void mlr_drops_print(MLRDrops *it) {
	printf(
		"{\"length (len)\": %d"
		", \"head\": %d"
		", \"tail\": %d"
		"}\n",
		it->len,
		it->head,
		it->tail
	);
}

void mlr_drops_print_data(MLRDrops *it) {
	int i = 0;

	if (it->head) {
		MLRDrop *head = it->head;
		while(head) {
			printf("%10d ", head->v);
			head = head->next;
		}
		printf("\n");
	}
}

static void mlr_drops_reject_outdated(MLRDrops *it, uint32_t outdated_since_stamp) {
    pthread_mutex_lock(&it->mutex);

    if (!it->head) {
        pthread_mutex_unlock(&it->mutex);
        return;
    }

    MLRDrop *head = it->head;
    while (head) {
        if (head->v > outdated_since_stamp) {
            pthread_mutex_unlock(&it->mutex);
            return;
        }

        if (!head->next) {
            it->head = NULL;
            it->tail = NULL;
            it->len = 0;
            free(head);
            pthread_mutex_unlock(&it->mutex);
            return;
        }

        it->head = head->next;
        it->len--;
        free(head);

        head = it->head;
    }

    pthread_mutex_unlock(&it->mutex);
}

static void mlr_drops_len(MLRDrops *it, uint32_t *v) {
    pthread_mutex_unlock(&it->mutex);
    *v = it->len;
    pthread_mutex_unlock(&it->mutex);
    return;
}

int main(int argc, char const *argv[]) {
	MLRDrops *it = mlr_drops_new();

	uint32_t ts1 = (uint32_t)time(NULL);
	uint32_t ts2 = ts1 + 1000;
	uint32_t ts3 = ts2 + 1000;
	uint32_t ts4 = ts3 + 1000;
	uint32_t ts5 = ts4 + 1000;
	uint32_t ts6 = ts5 + 1000;
	uint32_t ts7 = ts6 + 1000;

	mlr_drops_push(it, ts1);
	mlr_drops_push(it, ts2);
	mlr_drops_push(it, ts3);
	mlr_drops_push(it, ts4);

	mlr_drops_print(it);
	mlr_drops_print_data(it);

	uint32_t ts1_out = 0;
	mlr_drops_pull(it, &ts1_out);
	printf("%d\n", ts1_out);
	mlr_drops_print(it);
	mlr_drops_print_data(it);

	uint32_t ts2_out = 0;
	mlr_drops_pull(it, &ts2_out);
	printf("%d\n", ts2_out);
	mlr_drops_print(it);
	mlr_drops_print_data(it);

	// mlr_drops_pull(it, &ts2_out);
	// mlr_drops_pull(it, &ts2_out);
	// mlr_drops_pull(it, &ts2_out);
	// mlr_drops_pull(it, &ts2_out);
	// mlr_drops_pull(it, &ts2_out);

	mlr_drops_print(it);
	mlr_drops_print_data(it);

	mlr_drops_delete(it);

	uint32_t num = 1;
	uint32_t den = 3;
	double v = (double)num/(double)den;
	printf("%f\n", v);


	exit(EXIT_SUCCESS);
}
