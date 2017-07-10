#ifndef __COMMON_CONTAINER__
#define __COMMON_CONTAINER__


/* media container */


#include <url/url.h>


typedef enum container_enum {
	CONTAINER_UNKNOWN,

	/* V+A */
	CONTAINER_MPEGTS,
	CONTAINER_MP4,
	CONTAINER_WEBM,

	/* A */
	CONTAINER_AAC,
	CONTAINER_MP2,

	CONTAINER_TS = CONTAINER_MPEGTS,
} Container;


Container container_from_url(URL *u);


#endif // __COMMON_CONTAINER__
