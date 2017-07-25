#ifndef __APPS_3_FILTER_UNKNOWN__
#define __APPS_3_FILTER_UNKNOWN__


#include <ac3/ac3.h>  /* AC3_SYNC_WORD */

#include "track.h"   /* Filter, FilterVT */
#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_unknown_s FilterUnknown;
typedef struct pipeline_s Pipeline;
typedef int (*filter_on_trk_detect) (void *ctx, Track *trk);


extern FilterVT filter_unknown_vt;  /* virtual table */


struct filter_unknown_s {
	Filter fltr;

	filter_on_trk_detect on_detect;
	void                *on_detect_ctx;

	/* private */
	FILE *f_dump;  /* debug; TODO: remove; */
	Track *trk;    /* debug; TODO: remove; */
};


int filter_unknown_new(FilterUnknown **out);
int filter_unknown_init(FilterUnknown *it, filter_on_trk_detect on_detect, void* on_detect_ctx);
int filter_unknown_fin(FilterUnknown *it);
int filter_unknown_del(FilterUnknown **out);


#endif /* __APPS_3_FILTER_UNKNOWN__ */
