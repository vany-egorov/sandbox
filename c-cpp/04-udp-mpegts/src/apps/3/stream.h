#ifndef __APPS_3_STREAM__
#define __APPS_3_STREAM__


#include "track.h"


typedef struct stream_s Stream;


struct stream_s {
	uint8_t trks_cnt;
	Track  *trks[20];
};


#endif  /* __APPS_3_STREAM__ */
