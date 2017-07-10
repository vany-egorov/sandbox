#ifndef __APPS_3_DEMUXER__
#define __APPS_3_DEMUXER__


#include <stdint.h>  /* uint8_t */
#include <stddef.h>  /* size_t */


typedef struct demuxer_s Demuxer;
typedef struct demux_vt_s DemuxerVT;


struct demuxer_s {
	void *w; /* wrapped, child, opaque */

	DemuxerVT *vt; /* virtual table */
};

struct demux_vt_s {
	int (*consume_pkt_raw) (void *ctx, uint8_t *buf, size_t bufsz);
};


int demuxer_consume_pkt_raw(Demuxer *it, uint8_t *buf, size_t bufsz);


#endif /* __APPS_3_DEMUXER__ */
