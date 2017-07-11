#include "demuxer-build.h"


int demuxer_build(Demuxer *it, URL *u) {
	ContainerKind ck = CONTAINER_KIND_UNKNOWN;

	ck = container_kind_from_url(u);

	switch (ck) {
		case CONTAINER_KIND_MPEGTS: {
			DemuxerTS *i = NULL;
			demuxer_ts_new(&i);
			demuxer_ts_init(i, u);
			it->w = (void*)i;
			it->vt = &demuxer_ts_vt;

			break;
		}
		default:
			it->w = NULL;
			it->vt = NULL;
	}
}
