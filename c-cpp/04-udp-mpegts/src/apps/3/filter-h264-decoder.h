#ifndef __APPS_3_H264_DECODER__
#define __APPS_3_H264_DECODER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_h264_decoder_s FilterH264Decoder;


extern FilterVT filter_h264_decoder_vt;  /* virtual table */


struct filter_h264_decoder_s {
	Filter fltr;
};


#endif /* __APPS_3_H264_DECODER__ */
