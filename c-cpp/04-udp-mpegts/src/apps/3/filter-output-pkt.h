#ifndef __APPS_3_FILTER_OUTPUT_PKT_RAW__
#define __APPS_3_FILTER_OUTPUT_PKT_RAW__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_output_pkt_s FilterOutputPkt;


extern FilterVT filter_output_pkt_vt;  /* virtual table */


struct filter_output_pkt_s {
	Filter fltr;
};


int filter_output_pkt_new(FilterOutputPkt **out);
int filter_output_pkt_init(FilterOutputPkt *it);
int filter_output_pkt_fin(FilterOutputPkt *it);
int filter_output_pkt_del(FilterOutputPkt **out);


#endif /* __APPS_3_FILTER_OUTPUT_PKT_RAW__ */
