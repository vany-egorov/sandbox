#include "logger.h"


void log_trace(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->trace(it->w, format, args); va_end(args);
}

void log_debug(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->debug(it->w, format, args); va_end(args);
}

void log_info(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->info(it->w, format, args); va_end(args);
}

void log_warn(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->warn(it->w, format, args); va_end(args);
}

void log_error(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->error(it->w, format, args); va_end(args);
}

void log_critical(Logger *it, const char* format, ...) {
	va_list args;
	va_start(args, format); it->vt->critical(it->w, format, args); va_end(args);
}
