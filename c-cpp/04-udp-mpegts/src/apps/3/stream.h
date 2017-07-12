#ifndef __APPS_3_STREAM__
#define __APPS_3_STREAM__


#include <stdlib.h>  /* calloc */

#include <collections/slice.h>  /* Slice */

#include "track.h"  /* Track */


typedef struct stream_s Stream;


struct stream_s {
	Slice *trks;  /* tracks */
};


int stream_new(Stream **out);
int stream_init(Stream *it);
int stream_from_mpegts_psi_pmt(Stream *it, MPEGTSPSIPMT *psi_pmt);
int stream_fin(Stream *it);
int stream_del(Stream **out);


#endif  /* __APPS_3_STREAM__ */
