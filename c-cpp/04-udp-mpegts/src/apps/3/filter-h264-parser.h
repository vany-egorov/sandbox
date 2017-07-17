#ifndef __APPS_3_H264_PARSER__
#define __APPS_3_H264_PARSER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_h264_parser_s FilterH264Parser;


extern FilterVT filter_h264_parser_vt;  /* virtual table */


struct filter_h264_parser_s {
	Filter fltr;
};


#endif /* __APPS_3_H264_PARSER__ */
