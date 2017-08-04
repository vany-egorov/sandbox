#ifndef __APPS_3_H264_DECODER__
#define __APPS_3_H264_DECODER__


#include "filter.h"  /* Filter, FilterVT */
#include "filter-logger.h"  /* filter_logger */


typedef struct filter_h264_decoder_s FilterH264Decoder;


extern FilterVT filter_h264_decoder_vt;  /* virtual table */


struct filter_h264_decoder_s {
	Filter fltr;
};


int filter_h264_decoder_new(FilterH264Decoder **out);
int filter_h264_decoder_init(FilterH264Decoder *it);
int filter_h264_decoder_fin(FilterH264Decoder *it);
int filter_h264_decoder_del(FilterH264Decoder **out);


#endif /* __APPS_3_H264_DECODER__ */
