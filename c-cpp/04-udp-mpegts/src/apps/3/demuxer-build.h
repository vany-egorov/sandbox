#ifndef __APPS_3_DEMUXER_BUILD__
#define __APPS_3_DEMUXER_BUILD__


#include <url/url.h>
#include <common/container-kind.h>  /* ContainerKind */

#include "filter.h"      /* Filter */
#include "demuxer-ts.h"  /* demuxer_ts_vt */


int demuxer_build(Filter **out, URL *u);


#endif /* __APPS_3_DEMUXER_BUILD__ */
