#ifndef __LOG_LOG__
#define __LOG_LOG__


#define LOG_MAX_MESSAGE_SIZE 16384
#define LOG_FILE_MODE        0644


#include <time.h>
#include <errno.h> /* errno */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>  /* va_list, vsprintf */
#include <string.h>  /* strcmp */
#include <pthread.h> /* pthread_mutex_lock, pthread_mutex_unlock */
#include <sys/stat.h>

#include <common/color.h>

#include "level.h"   /* LogLevel, log_level_short, log_level_color1, log_level_color2 */
#include "logger.h"  /* LoggerVT */
#include "common.h"  /* log_datetime */


extern LoggerVT log_logger_vt;  /* log logger virtual table */


typedef struct log_s Log;


struct log_s {
	char name[10];

	mode_t mode;
	FILE  *file_access;
	FILE  *file_error;
	int    fd_access;
	int    fd_error;

	LogLevel log_level_min;

	int is_stderr_stdout_enabled;

	pthread_mutex_t lock;
};

int log_new(Log **out);
int log_init(Log *it, char *name, LogLevel log_level_min,
             char *path,
             char *filename_acess, char *filename_error,
             int is_stderr_stdout_enabled);
int log_rotate(Log *it, char *path,
               char *filename_acess, char *filename_error);
int log_into_logger(Log *it, Logger *out);
int log_fin(Log *it);
int log_del(Log **out);

void log_log_trace(Log *it, const char* format, ...);
void log_log_debug(Log *it, const char* format, ...);
void log_log_info(Log *it, const char* format, ...);
void log_log_warn(Log *it, const char* format, ...);
void log_log_error(Log *it, const char* format, ...);
void log_log_critical(Log *it, const char* format, ...);

void log_log_noendl_trace(Log *it, const char* format, ...);
void log_log_noendl_debug(Log *it, const char* format, ...);
void log_log_noendl_info(Log *it, const char* format, ...);
void log_log_noendl_warn(Log *it, const char* format, ...);
void log_log_noendl_error(Log *it, const char* format, ...);
void log_log_noendl_critical(Log *it, const char* format, ...);


#endif /* __LOG_LOG__ */
