#ifndef __APPS_3_FILTER_UNKNOWN__
#define __APPS_3_FILTER_UNKNOWN__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_unknown_s FilterUnknown;


extern FilterVT filter_unknown_vt;  /* virtual table */


struct filter_unknown_s {
	Filter fltr;
};


int filter_unknown_new(FilterUnknown **out);
int filter_unknown_init(FilterUnknown *it);
int filter_unknown_fin(FilterUnknown *it);
int filter_unknown_del(FilterUnknown **out);


#endif /* __APPS_3_FILTER_UNKNOWN__ */
