#include "container-kind.h"


ContainerKind container_kind_from_url(URL *u) {
	URLProtocol p = URL_SCHEME_UNKNOWN;
	const char *e = NULL;

	if (!u) return CONTAINER_KIND_UNKNOWN;

	p = u->scheme;
	e = url_ext(u);

	if (p == URL_SCHEME_UDP) return CONTAINER_KIND_MPEGTS;

	if (!strcasecmp(e, "ts") || !strcasecmp(e, "mpegts")) return CONTAINER_KIND_MPEGTS;
	if (!strcasecmp(e, "mp4")) return CONTAINER_KIND_MP4;
	if (!strcasecmp(e, "m4a")) return CONTAINER_KIND_MP4;
	if (!strcasecmp(e, "m4v")) return CONTAINER_KIND_MP4;
	if (!strcasecmp(e, "mkv")) return CONTAINER_KIND_MKV;

	return CONTAINER_KIND_UNKNOWN;
}

char *container_kind_str(ContainerKind it) {
	switch (it) {
	case CONTAINER_KIND_MPEGTS: return CONTAINER_KIND_MPEGTS_STR;
	case CONTAINER_KIND_MP4: return CONTAINER_KIND_MPEGTS_STR;
	case CONTAINER_KIND_WEBM: return CONTAINER_KIND_WEBM_STR;
	case CONTAINER_KIND_MKV: return CONTAINER_KIND_MKV_STR;
	default: return CONTAINER_KIND_UNKNOWN_STR;
	}

	return CONTAINER_KIND_UNKNOWN_STR;
}
