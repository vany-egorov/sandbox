#ifndef __APPS_3_FILTER_OUT_PKT__
#define __APPS_3_FILTER_OUT_PKT__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_out_pkt_s FilterOutPkt;


extern FilterVT filter_out_pkt_vt;  /* virtual table */


struct filter_out_pkt_s {
	Filter fltr;

	URL *u;

	FILE *__f;
};


int filter_out_pkt_new(FilterOutPkt **out);
int filter_out_pkt_init(FilterOutPkt *it);
int filter_out_pkt_fin(FilterOutPkt *it);
int filter_out_pkt_del(FilterOutPkt **out);


#endif /* __APPS_3_FILTER_OUT_PKT__ */
