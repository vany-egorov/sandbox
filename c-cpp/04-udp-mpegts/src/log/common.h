#ifndef __LOG_COMMON__
#define __LOG_COMMON__


#include <time.h>  /* tm, time_t, localtime */
#include <stddef.h>  /* size_t */


void log_datetime(char* buf, size_t sz);


#endif /* __LOG_COMMON__ */
