#ifndef __LOG_LOG__
#define __LOG_LOG__


#define LOG_MAX_MESSAGE_SIZE 16384
#define LOG_FILE_MODE        0644


#include "level.h"  /* LogLevel, log_level_short, log_level_color1, log_level_color2 */
#include "common.h"  /* log_datetime */


typedef struct {
	char        name[10];

	mode_t mode;
	FILE  *file_access;
	FILE  *file_error;
	int    fd_access;
	int    fd_error;

	log_level_t log_level_min;

	int is_stderr_stdout_enabled;

	pthread_mutex_t lock;
} log_t;

log_level_t log_level_new(char* v);
log_t      *log_new(char *name, log_level_t log_level_min,
                    char *path,
                    char *filename_acess, char *filename_error,
                    int is_stderr_stdout_enabled);
int         log_rotate(log_t *it, char *path,
                       char *filename_acess, char *filename_error);
void        log_del(log_t *it);

void log_trace(log_t *it, const char* format, ...);
void log_debug(log_t *it, const char* format, ...);
void log_info(log_t *it, const char* format, ...);
void log_warn(log_t *it, const char* format, ...);
void log_error(log_t *it, const char* format, ...);
void log_critical(log_t *it, const char* format, ...);

void log_noendl_trace(log_t *it, const char* format, ...);
void log_noendl_debug(log_t *it, const char* format, ...);
void log_noendl_info(log_t *it, const char* format, ...);
void log_noendl_warn(log_t *it, const char* format, ...);
void log_noendl_error(log_t *it, const char* format, ...);
void log_noendl_critical(log_t *it, const char* format, ...);


#endif // __LOG_LOG__
