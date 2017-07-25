#ifndef __APPS_3_PIPELINE__
#define __APPS_3_PIPELINE__


#include <collections/slice.h>  /* Slice */

#include "track.h"                /* Track */
#include "filter.h"               /* Filter, FilterVT */
#include "filter-unknown.h"       /* FilterUnknown */
#include "filter-h264-parser.h"   /* FilterH264Parser */
#include "filter-h264-decoder.h"  /* FilterH264Decoder */
#include "filter-mp2-parser.h"    /* FilterMP2Parser */
#include "filter-mp2-decoder.h"   /* FilterMP2Decoder */
#include "filter-ac3-parser.h"    /* FilterAC3Parser */
#include "filter-ac3-decoder.h"   /* FilterAC3Decoder */


typedef struct pipeline_s     Pipeline;
typedef struct filter_track_s FilterTrack;


extern FilterVT pipeline_filter_vt;  /* virtual table */


/* filter + attached track
 * ID/PID inside track
 */
struct filter_track_s {
	Filter *fltr;
	Track  *trk;  /* <----- ID/PID storage */
};

struct pipeline_s {
	Filter fltr;  /* pipeline is filter; inherited from Filter; */

	Slice fltrs; /* Filter storage */
	Slice trks;  /* FilterTracks */
};


int pipeline_new(Pipeline **out);
int pipeline_init(Pipeline *it);
int pipeline_on_trk_detect(void *ctx, Track *trk);
int pipeline_fin(Pipeline *it);
int pipeline_del(Pipeline **out);


#endif /* __APPS_3_PIPELINE__ */
