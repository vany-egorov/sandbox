#ifndef __APPS_3_MP2_PARSER__
#define __APPS_3_MP2_PARSER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_mp2_parser_s FilterMP2Parser;


extern FilterVT filter_mp2_parser_vt;  /* virtual table */


struct filter_mp2_parser_s {
	Filter fltr;
};


int filter_mp2_parser_new(FilterMP2Parser **out);
int filter_mp2_parser_init(FilterMP2Parser *it);
int filter_mp2_parser_fin(FilterMP2Parser *it);
int filter_mp2_parser_del(FilterMP2Parser **out);


#endif /* __APPS_3_MP2_PARSER__ */
