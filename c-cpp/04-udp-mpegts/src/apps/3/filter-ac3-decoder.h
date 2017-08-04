#ifndef __APPS_3_AC3_DECODER__
#define __APPS_3_AC3_DECODER__


#include "filter.h"  /* Filter, FilterVT */
#include "filter-logger.h"  /* filter_logger */


typedef struct filter_ac3_decoder_s FilterAC3Decoder;


extern FilterVT filter_ac3_decoder_vt;  /* virtual table */


struct filter_ac3_decoder_s {
	Filter fltr;
};


int filter_ac3_decoder_new(FilterAC3Decoder **out);
int filter_ac3_decoder_init(FilterAC3Decoder *it);
int filter_ac3_decoder_fin(FilterAC3Decoder *it);
int filter_ac3_decoder_del(FilterAC3Decoder **out);


#endif /* __APPS_3_AC3_DECODER__ */
