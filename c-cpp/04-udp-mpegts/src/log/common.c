#include "common.h"


inline void log_datetime(char* buf, size_t sz) {
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buf, sz, "%d/%b/%Y %H:%M:%S", timeinfo);

	return;
}
