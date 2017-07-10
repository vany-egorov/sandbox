#include "std.h"


static void p(void *ctx, LogLevel level, const char* format, va_list args) {
	char msg[16384] = { 0 };
	char datetime[30] = { 0 };
	char *c1 = NULL,
	     *c2 = NULL;

	log_datetime(datetime, sizeof(datetime));
	c1 = log_level_color1(level);
	c2 = log_level_color2(level);
	vsprintf(msg, format, args);

	printf("%s[%s]%s %s[%c]%s %s%s%s",
		COLOR_BRIGHT_WHITE, datetime, COLOR_RESET,
		c1, log_level_short(level), COLOR_RESET,
		c2, msg, COLOR_RESET
	);
}


static void trace(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_TRACE, format, args);
}

static void debug(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_DEBUG, format, args);
}

static void info(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_INFO, format, args);
}

static void warn(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_WARN, format, args);
}

static void error(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_ERROR, format, args);
}

static void critical(void *ctx, const char* format, va_list args) {
	p(ctx, LOG_LEVEL_CRITICAL, format, args);
}


LoggerVT log_std_vt = {
	.trace = trace,
	.debug = debug,
	.info = info,
	.warn = warn,
	.error = error,
	.critical = critical,
};

Logger logger_std = {
	.w = NULL,
	.vt = &log_std_vt,
};

// Logger = {
// 	.w = 0,
// 	.vt = &log_std_vt,
// };
