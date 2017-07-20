#ifndef __APPS_3_PIPELINE__
#define __APPS_3_PIPELINE__


#include <collections/slice.h>  /* Slice */

#include "filter.h"  /* Filter, FilterVT */
#include "track.h"   /* Track */


typedef struct pipeline_s     Pipeline;
typedef struct filter_track_s FIlterTrack;


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

	Slice trks;  /* FilterTracks */
};


int pipeline_new(Pipeline **out);
int pipeline_init(Pipeline *it);
int pipeline_fin(Pipeline *it);
int pipeline_del(Pipeline **out);


#endif /* __APPS_3_PIPELINE__ */
