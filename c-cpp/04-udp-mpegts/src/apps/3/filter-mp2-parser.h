#ifndef __APPS_3_MP2_PARSER__
#define __APPS_3_MP2_PARSER__


#include "filter.h"  /* Filter, FilterVT */


typedef struct filter_mp2_parser_s FilterMP2Parser;


extern FilterVT filter_mp2_parser_vt;  /* virtual table */


struct filter_mp2_parser_s {
	Filter fltr;
};


#endif /* __APPS_3_MP2_PARSER__ */
