#ifndef __COMMON_CONTAINER_KIND__
#define __COMMON_CONTAINER_KIND__


/* media container */


#include <url/url.h>


#define CONTAINER_KIND_UNKNOWN_STR "unknown"

#define CONTAINER_KIND_MPEGTS_STR "MPEG-TS"
#define CONTAINER_KIND_MP4_STR    "MP4"
#define CONTAINER_KIND_WEBM_STR   "WebM"
#define CONTAINER_KIND_MKV_STR    "MKV"


typedef enum container_kind_enum {
	CONTAINER_KIND_UNKNOWN,

	CONTAINER_KIND_MPEGTS,
	CONTAINER_KIND_MP4,
	CONTAINER_KIND_WEBM,
	CONTAINER_KIND_MKV,

	CONTAINER_KIND_TS = CONTAINER_KIND_MPEGTS,
} ContainerKind;


ContainerKind container_kind_from_url(URL *u);
char *container_kind_str(ContainerKind it);


#endif /* __COMMON_CONTAINER_KIND__ */
