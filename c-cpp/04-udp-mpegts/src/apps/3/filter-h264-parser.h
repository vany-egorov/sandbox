#ifndef __APPS_3_H264_PARSER__
#define __APPS_3_H264_PARSER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_h264_parser_s FilterH264Parser;


extern FilterVT filter_h264_parser_vt;  /* virtual table */


struct filter_h264_parser_s {
	Filter fltr;
};


int filter_h264_parser_new(FilterH264Parser **out);
int filter_h264_parser_init(FilterH264Parser *it);
int filter_h264_parser_fin(FilterH264Parser *it);
int filter_h264_parser_del(FilterH264Parser **out);


#endif /* __APPS_3_H264_PARSER__ */
