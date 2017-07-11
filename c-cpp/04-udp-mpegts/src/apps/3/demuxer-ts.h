#ifndef __APPS_3_DEMUXER_TS__
#define __APPS_3_DEMUXER_TS__


#include <stdio.h>
#include <stdlib.h>  /* calloc */

#include <log/std.h>        /* logger_std */
#include <url/url.h>        /* URL */
#include <log/logger.h>     /* Logger */
#include <mpegts/mpegts.h>  /* MPEGTS_SYNC_BYTE, mpegts_* */

#include "demuxer.h"  /* DemuxerVT */


typedef struct demuxer_ts_s DemuxerTS;


extern DemuxerVT demuxer_ts_vt;  /* MPEGTS virtual table */


struct demuxer_ts_s {
	URL u;
	char us[255];

	int is_psi_printed;
	MPEGTS ts;
};


int demuxer_ts_new(DemuxerTS **out);
int demuxer_ts_init(DemuxerTS *it, URL *u);


#endif /* __APPS_3_DEMUXER_TS__ */
