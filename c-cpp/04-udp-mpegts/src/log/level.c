#include "level.h"


LogLevel log_level_parse(char* s) {
	if ((!strcasecmp(s, "trace")) || (!strcasecmp(s, "t"))) {
		return LOG_LEVEL_TRACE;
	} else if ((!strcasecmp(s, "debug")) || (!strcasecmp(s, "d"))) {
		return LOG_LEVEL_DEBUG;
	} else if ((!strcasecmp(s, "info")) || (!strcasecmp(s, "i"))) {
		return LOG_LEVEL_INFO;
	} else if ((!strcasecmp(s, "warn")) || (!strcasecmp(s, "w"))) {
		return LOG_LEVEL_WARN;
	} else if ((!strcasecmp(s, "error")) || (!strcasecmp(s, "e"))) {
		return LOG_LEVEL_ERROR;
	} else if ((!strcasecmp(s, "critical")) || (!strcasecmp(s, "c"))) {
		return LOG_LEVEL_CRITICAL;
	} else if (!strcasecmp(s, "off")) {
		return LOG_LEVEL_OFF;
	}

	return LOG_LEVEL_INFO;
}

inline char log_level_short(LogLevel it) {
	switch (it) {
	case LOG_LEVEL_TRACE:     return 't';
	case LOG_LEVEL_DEBUG:     return 'd';
	case LOG_LEVEL_INFO:      return 'i';
	case LOG_LEVEL_WARN:      return 'w';
	case LOG_LEVEL_ERROR:     return 'e';
	case LOG_LEVEL_CRITICAL:  return 'c';
	case LOG_LEVEL_OFF:       break;
	default:                  return '~';
	}

	return '~';
}

inline char *log_level_color1(LogLevel it) {
	switch (it) {
	case LOG_LEVEL_TRACE: return &COLOR_BRIGHT_CYAN[0];
	case LOG_LEVEL_DEBUG: return &COLOR_BRIGHT_BLUE[0];
	case LOG_LEVEL_INFO: return &COLOR_BRIGHT_GREEN[0];
	case LOG_LEVEL_WARN: return &COLOR_BRIGHT_YELLOW[0];
	case LOG_LEVEL_ERROR: return &COLOR_BRIGHT_RED[0];
	case LOG_LEVEL_CRITICAL: return &COLOR_BRIGHT_MAGENTA[0];
	case LOG_LEVEL_OFF: return "";
	default: return "";
	}

	return &COLOR_BRIGHT_WHITE[0];
}

inline char *log_level_color2(LogLevel it) {
	switch (it) {
	case LOG_LEVEL_TRACE: return &COLOR_DIM_CYAN[0];
	case LOG_LEVEL_DEBUG: return &COLOR_BRIGHT_BLUE[0];
	case LOG_LEVEL_INFO: return &COLOR_DIM_GREEN[0];
	case LOG_LEVEL_WARN: return &COLOR_DIM_YELLOW[0];
	case LOG_LEVEL_ERROR: return &COLOR_DIM_RED[0];
	case LOG_LEVEL_CRITICAL: return &COLOR_DIM_MAGENTA[0];
	case LOG_LEVEL_OFF: return "";
	default: return "";
	}

	return &COLOR_BRIGHT_WHITE[0];
}
