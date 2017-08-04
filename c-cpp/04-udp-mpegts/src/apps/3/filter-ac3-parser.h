#ifndef __APPS_3_AC3_PARSER__
#define __APPS_3_AC3_PARSER__


#include "filter.h"  /* Filter, FilterVT */
#include "filter-logger.h"  /* filter_logger */


typedef struct filter_ac3_parser_s FilterAC3Parser;


extern FilterVT filter_ac3_parser_vt;  /* virtual table */


struct filter_ac3_parser_s {
	Filter fltr;
};


int filter_ac3_parser_new(FilterAC3Parser **out);
int filter_ac3_parser_init(FilterAC3Parser *it);
int filter_ac3_parser_fin(FilterAC3Parser *it);
int filter_ac3_parser_del(FilterAC3Parser **out);


#endif /* __APPS_3_AC3_PARSER__ */
