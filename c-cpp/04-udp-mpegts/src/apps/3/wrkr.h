#ifndef __APPS_3_WRKR__
#define __APPS_3_WRKR__


#include <stdio.h>    /* printf */
#include <unistd.h>   /* sleep */
#include <pthread.h>  /* pthread_t, pthread_create */

#include "input.h"
#include "input-build.h"


typedef struct wrkr_s Wrkr;
typedef struct wrkr_cfg_s WrkrCfg;


struct wrkr_cfg_s {
	URL *url;

	InputCfg *i;
};

struct wrkr_s {
	pthread_t _thrd;

	Input input;
};


int wrkr_new(Wrkr **out);
int wrkr_init(Wrkr *it, WrkrCfg *cfg);
int wrkr_run(Wrkr *it);
int wrkr_fin(Wrkr *it);
int wrkr_del(Wrkr **out);


#endif /* __APPS_3_WRKR__ */
