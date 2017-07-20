#ifndef __APPS_3_PIPELINE__
#define __APPS_3_PIPELINE__


typedef struct pipeline_s     Pipeline;
typedef struct filter_track_s FIlterTrack;

/* filter + attached track
 * ID/PID inside track
 */
struct filter_track_s {
	Filter *fltr;
	Track  *trk;  /* <----- ID/PID storage */
}

struct pipeline_s {
	Slice trks;  /* FilterTracks */
}


#endif /* __APPS_3_PIPELINE__ */
