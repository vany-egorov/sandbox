#include "demuxer-build.h"


int demuxer_build(Filter **out, URL *u) {
	ContainerKind ck = CONTAINER_KIND_UNKNOWN;

	ck = container_kind_from_url(u);

	switch (ck) {
		case CONTAINER_KIND_MPEGTS: {
			DemuxerTS *dmxr = NULL;
			demuxer_ts_new(&dmxr);
			demuxer_ts_init(dmxr, u);
			dmxr->fltr.w = (void*)dmxr;
			dmxr->fltr.vt = &demuxer_ts_filter_vt;

			*out = &dmxr->fltr;

			break;
		}
		default:
			*out = NULL;
	}
}
