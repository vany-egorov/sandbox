#ifndef __VA_LOG_STD__
#define __VA_LOG_STD__


#include <stdio.h>

#include "logger.h"  /* LoggerVT */
#include "level.h"   /* LogLevel */
#include "common.h"  /* log_datetime */


extern LoggerVT log_std_vt;  /* std log virtual table */
extern Logger logger_std;


#endif /* __VA_LOG_STD__ */
