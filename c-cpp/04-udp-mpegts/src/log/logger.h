#ifndef __VA_LOG_LOGGER__
#define __VA_LOG_LOGGER__


#include <stdarg.h>  /* va_list */


typedef struct logger_s Logger;
typedef struct logger_vt_s LoggerVT;


struct logger_s {
	void *w; /* wrapped, child, opaque */

	LoggerVT *vt; /* virtual table */
};

struct logger_vt_s {
	void (*trace) (void *it, const char* format, va_list args);
	void (*debug) (void *it, const char* format, va_list args);
	void (*info) (void *it, const char* format, va_list args);
	void (*warn) (void *it, const char* format, va_list args);
	void (*error) (void *it, const char* format, va_list args);
	void (*critical) (void *it, const char* format, va_list args);
};


void log_trace(Logger *it, const char* format, ...);
void log_debug(Logger *it, const char* format, ...);
void log_info(Logger *it, const char* format, ...);
void log_warn(Logger *it, const char* format, ...);
void log_error(Logger *it, const char* format, ...);
void log_critical(Logger *it, const char* format, ...);


#endif /* __VA_LOG_LOGGER__ */
