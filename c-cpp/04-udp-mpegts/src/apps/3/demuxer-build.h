#ifndef __APPS_3_DEMUXER_BUILD__
#define __APPS_3_DEMUXER_BUILD__


#include <url/url.h>
#include <common/container.h>  /* Container */

#include "demuxer.h"  /* Demuxer */
#include "demuxer-ts.h"  /* demuxer_ts_vt */


int demuxer_build(Demuxer *it, URL *u);


#endif /* __APPS_3_DEMUXER_BUILD__ */
