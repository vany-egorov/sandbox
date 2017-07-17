#ifndef __APPS_3_DEMUXER_TS__
#define __APPS_3_DEMUXER_TS__


#include <stdio.h>
#include <stdlib.h>  /* calloc */

#include <log/std.h>        /* logger_std */
#include <url/url.h>        /* URL */
#include <log/logger.h>     /* Logger */
#include <mpegts/mpegts.h>  /* MPEGTS_SYNC_BYTE, mpegts_* */

#include "filter.h"  /* Filter, FilterVT */
#include "stream.h"  /* Stream */


typedef struct demuxer_ts_s DemuxerTS;


extern FilterVT demuxer_ts_filter_vt;  /* MPEGTS virtual table */


struct demuxer_ts_s {
	Filter fltr;  /* TODO: move to base? demuxer-base */
	Stream strm;  /* TODO: move to base? demuxer-base */

	URL u;         /* debug */
	char us[255];  /* debug */

	MPEGTS ts;

	uint8_t
		is_psi_logged:1,
		is_stream_builded:1,
		reserved_bit_fields:6;
};


int demuxer_ts_new(DemuxerTS **out);
int demuxer_ts_init(DemuxerTS *it, URL *u);


#endif /* __APPS_3_DEMUXER_TS__ */
