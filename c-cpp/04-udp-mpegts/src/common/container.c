#include "container.h"


Container container_from_url(URL *u) {
	URLProtocol p = URL_SCHEME_UNKNOWN;
	const char *e = NULL;

	if (!u) return CONTAINER_UNKNOWN;

	p = u->scheme;
	e = url_ext(u);

	if (p == URL_SCHEME_UDP) return CONTAINER_MPEGTS;

	if (!strcasecmp(e, "ts") || !strcasecmp(e, "mpegts")) return CONTAINER_MPEGTS;
	if (!strcasecmp(e, "mp4")) return CONTAINER_MP4;
	if (!strcasecmp(e, "m4a")) return CONTAINER_MP4;
	if (!strcasecmp(e, "m4v")) return CONTAINER_MP4;

	return CONTAINER_UNKNOWN;
}
