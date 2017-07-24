#ifndef __APPS_3_MP2_DECODER__
#define __APPS_3_MP2_DECODER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_mp2_decoder_s FilterMP2Decoder;


extern FilterVT filter_mp2_decoder_vt;  /* virtual table */


struct filter_mp2_decoder_s {
	Filter fltr;
};


int filter_mp2_decoder_new(FilterMP2Decoder **out);
int filter_mp2_decoder_init(FilterMP2Decoder *it);
int filter_mp2_decoder_fin(FilterMP2Decoder *it);
int filter_mp2_decoder_del(FilterMP2Decoder **out);


#endif /* __APPS_3_MP2_DECODER__ */
