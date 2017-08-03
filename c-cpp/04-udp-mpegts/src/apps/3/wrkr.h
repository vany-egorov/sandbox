#ifndef __APPS_3_WRKR__
#define __APPS_3_WRKR__


#include <stdio.h>    /* printf */
#include <unistd.h>   /* sleep */
#include <pthread.h>  /* pthread_t, pthread_create */

#include <log/logger.h> /* Logger */
#include <log/std.h>    /* logger_std */

#include "wrkr-cfg.h"       /* WrkrCfg */
#include "input.h"          /* Input */
#include "input-build.h"    /* input_build */
#include "pipeline.h"       /* Pipeline */
#include "filter.h"         /* Filter */
#include "demuxer-build.h"  /* demuxer_build */


typedef struct wrkr_s Wrkr;


struct wrkr_s {
	WrkrCfg cfg;

	Input input;

	Filter *demuxer;  /* TODO: <move to pipeline> */
	Pipeline ppln;

	pthread_t _thrd;
};


int wrkr_new(Wrkr **out);
int wrkr_init(Wrkr *it, WrkrCfg cfg);
int wrkr_run(Wrkr *it);
int wrkr_fin(Wrkr *it);
int wrkr_del(Wrkr **out);


#endif /* __APPS_3_WRKR__ */
