#include "log-level.h"


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
