#ifndef __APPS_3_FILTER_OUT_PKT_RAW__
#define __APPS_3_FILTER_OUT_PKT_RAW__


#include "filter.h"         /* Filter, FilterVT */
#include "filter-logger.h"  /* filter_logger */


typedef struct filter_out_pkt_raw_s FilterOutPktRaw;


extern FilterVT filter_out_pkt_raw_vt;  /* virtual table */


struct filter_out_pkt_raw_s {
	Filter fltr;

	URL *u;

	FILE *__f;
};


int filter_out_pkt_raw_new(FilterOutPktRaw **out);
int filter_out_pkt_raw_init(FilterOutPktRaw *it);
int filter_out_pkt_raw_fin(FilterOutPktRaw *it);
int filter_out_pkt_raw_del(FilterOutPktRaw **out);


#endif /* __APPS_3_FILTER_OUT_PKT_RAW__ */
