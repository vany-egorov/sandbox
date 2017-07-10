#ifndef __APPS_3_DEMUXER_TS__
#define __APPS_3_DEMUXER_TS__


#include <stdio.h>
#include <stdlib.h>  /* calloc */

#include <mpegts/mpegts.h>  /* calloc */

#include "demuxer.h"  /* DemuxerVT */


typedef struct demuxer_ts_s DemuxerTS;


extern DemuxerVT demuxer_ts_vt;  /* MPEGTS virtual table */


struct demuxer_ts_s {};


int demuxer_ts_new(DemuxerTS **out);
int demuxer_ts_init(DemuxerTS *it);




#endif /* __APPS_3_DEMUXER_TS__ */
