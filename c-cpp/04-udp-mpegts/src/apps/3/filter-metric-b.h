#ifndef __APPS_3_FILTER_METRIC_BITRATE__
#define __APPS_3_FILTER_METRIC_BITRATE__


#include "filter.h"         /* Filter, FilterVT */
#include "filter-logger.h"  /* filter_logger */


typedef struct filter_metric_b_s FilterMetricB;


extern FilterVT filter_metric_b_vt;  /* virtual table */


struct filter_metric_b_s {
	Filter fltr;
};


int filter_metric_b_new(FilterMetricB **out);
int filter_metric_b_init(FilterMetricB *it);
int filter_metric_b_fin(FilterMetricB *it);
int filter_metric_b_del(FilterMetricB **out);


#endif /* __APPS_3_FILTER_METRIC_BITRATE__ */
