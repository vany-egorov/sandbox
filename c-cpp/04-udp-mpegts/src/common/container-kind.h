#ifndef __COMMON_CONTAINER_KIND__
#define __COMMON_CONTAINER_KIND__


/* media container */


#include <url/url.h>


typedef enum container_kind_enum {
	CONTAINER_KIND_UNKNOWN,

	/* V+A */
	CONTAINER_KIND_MPEGTS,
	CONTAINER_KIND_MP4,
	CONTAINER_KIND_WEBM,

	/* A */
	CONTAINER_KIND_AAC,
	CONTAINER_KIND_MP2,

	CONTAINER_KIND_TS = CONTAINER_KIND_MPEGTS,
} ContainerKind;


ContainerKind container_kind_from_url(URL *u);


#endif // __COMMON_CONTAINER_KIND__
