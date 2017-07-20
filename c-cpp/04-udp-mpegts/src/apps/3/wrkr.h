#ifndef __APPS_3_WRKR__
#define __APPS_3_WRKR__


#include <stdio.h>    /* printf */
#include <unistd.h>   /* sleep */
#include <pthread.h>  /* pthread_t, pthread_create */

#include <log/logger.h> /* Logger */
#include <log/std.h>    /* logger_std */

#include "input.h"                /* Input */
#include "input-build.h"          /* input_build */
#include "pipeline.h"             /* Pipeline */
#include "filter.h"               /* Filter */
#include "filter-splitter.h"      /* Filter */
#include "filter-h264-parser.h"   /* FilterH264Parser */
#include "filter-h264-decoder.h"  /* FilterH264Decoder */
#include "demuxer-build.h"        /* demuxer_build */


typedef struct wrkr_s Wrkr;
typedef struct wrkr_cfg_s WrkrCfg;


struct wrkr_cfg_s {
	URL *url;

	InputCfg *i;
};

struct wrkr_s {
	pthread_t _thrd;

	Input input;

	/* TODO: <move to pipeline> */
	Filter *demuxer;

	FilterSplitter    splitter;
	FilterH264Parser  h264_parser;
	FilterH264Decoder h264_decoder;
	/* TODO: </move to pipeline> */
};


int wrkr_new(Wrkr **out);
int wrkr_init(Wrkr *it, WrkrCfg *cfg);
int wrkr_run(Wrkr *it);
int wrkr_fin(Wrkr *it);
int wrkr_del(Wrkr **out);


#endif /* __APPS_3_WRKR__ */
