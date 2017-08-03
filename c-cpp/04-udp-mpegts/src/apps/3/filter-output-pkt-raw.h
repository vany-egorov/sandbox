#ifndef __APPS_3_FILTER_OUTPUT_PKT_RAW__
#define __APPS_3_FILTER_OUTPUT_PKT_RAW__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_output_pkt_raw_s FilterOutputPktRaw;


extern FilterVT filter_output_pkt_raw_vt;  /* virtual table */


struct filter_output_pkt_raw_s {
	Filter fltr;
};


int filter_output_raw_new(FilterOutputPktRaw **out);
int filter_output_raw_init(FilterOutputPktRaw *it);
int filter_output_raw_fin(FilterOutputPktRaw *it);
int filter_output_raw_del(FilterOutputPktRaw **out);


#endif /* __APPS_3_FILTER_OUTPUT_PKT_RAW__ */
