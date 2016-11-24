#include <stdio.h>
#include <pthread.h>

#include "./libmill/libmill.h"


#define NUM_WORKERS 10


struct _worker_ctx_ {
	int id;
	chan ch1;
	chan ch2;
};
typedef struct _worker_ctx_  worker_ctx;
typedef struct _worker_ctx_ *worker_ctx_p;


void *worker(void *arg) {
	int i = 0;
	worker_ctx_p ctx = NULL;

	ctx = arg;
	printf("[#%d] %p, %p\n", ctx->id, ctx->ch1, ctx->ch1);

	for (;;) {
		choose {
		in(ctx->ch1, int, i):
			printf("[#%d] 1 <- %d\n", ctx->id, i);
		in(ctx->ch2, int, i):
			printf("[#%d] 2 <- %d\n", ctx->id, i);
		otherwise:
			printf("[#%d] all done!\n", ctx->id);
			return 0;
		end
		}

		sleep(1);
	}
}

int main(int argc, char const *argv[]) {
	int i = 0,
	    ret = 0;
	chan ch1 = NULL,
	     ch2 = NULL;
	pthread_t  workers[NUM_WORKERS];
	worker_ctx worker_ctxs[NUM_WORKERS];

	ch1 = chmake(int, 20);
	for (i=0; i < 20; i++) {
		chs(ch1, int, i);
		printf("1 -> %d\n", i);
	}
	ch2 = chmake(int, 20);
	for (i=0; i < 20; i++) {
		chs(ch2, int, i);
		printf("2 -> %d\n", i);
	}

	for (i=0; i<NUM_WORKERS; i++) {
		worker_ctx_p ctx = &worker_ctxs[i];
		ctx->id = i+1;
		ctx->ch1=ch1;
		ctx->ch2=ch2;
		if (ret = pthread_create(&workers[i], NULL, worker, (void *)ctx)){
      printf("ERROR; return code from pthread_create() is %d\n", ret);
      exit(-1);
    }
	}

	sleep(10);

	printf("%p\n", ch1);
	printf("%p\n", ch2);

	chclose(ch1);
	chclose(ch2);
	pthread_exit(NULL);

	return 0;
}
