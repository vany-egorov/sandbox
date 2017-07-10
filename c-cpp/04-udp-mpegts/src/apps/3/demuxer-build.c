#include "demuxer-build.h"


int demuxer_build(Demuxer *it, URL *u) {
	Container c = CONTAINER_UNKNOWN;

	c = container_from_url(u);

	switch (c) {
		case CONTAINER_MPEGTS: {
			DemuxerTS *i = NULL;
			demuxer_ts_new(&i);
			demuxer_ts_init(i);
			it->w = (void*)i;
			it->vt = &demuxer_ts_vt;

			break;
		}
		default:
			it->w = NULL;
			it->vt = NULL;
	}
}
