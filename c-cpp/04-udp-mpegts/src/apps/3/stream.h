#ifndef __APPS_3_STREAM__
#define __APPS_3_STREAM__


#include <stdlib.h>  /* calloc */

#include <common/container-kind.h>  /* ContainerKind */
#include <collections/slice.h>  /* Slice */

#include "track.h"  /* Track */


typedef struct stream_s Stream;


struct stream_s {
	ContainerKind container_kind;

	Slice trks;  /* tracks */
};


int stream_new(Stream **out);
int stream_init(Stream *it);
int stream_from_mpegts_psi_pmt(Stream *it, MPEGTSPSIPMT *psi_pmt);
/* get track by ID/PID */
int stream_get_track(Stream *it, uint32_t id, Track **trk);
int stream_fin(Stream *it);
int stream_del(Stream **out);


#endif  /* __APPS_3_STREAM__ */
