#ifndef __APPS_3_FILTER_SPLITTER__
#define __APPS_3_FILTER_SPLITTER__


#include <errno.h>  /* errno */
#include <string.h> /* strerror */

#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_splitter_s FilterSplitter;


extern FilterVT filter_splitter_vt;  /* virtual table */


struct filter_splitter_s {
	Filter fltr;
};


#endif /* __APPS_3_FILTER_SPLITTER__ */
