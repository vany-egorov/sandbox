#include "time.h"


Duration time_sub(struct timespec *f, struct timespec *t) {
	return ((Duration)((t->tv_sec - f->tv_sec) * TimeSecond) + (Duration)(t->tv_nsec - f->tv_nsec));
}

Duration time_since(struct timespec *start) {
	struct timespec now = { 0 };

	clock_gettime(CLOCK_MONOTONIC, &now);

	return time_sub(start, &now);
}

int time_duration_str(Duration it, char *buf, size_t bufsz) {
	int ret = 0;

	if (it <= TimeMicrosecond) {
		snprintf(buf, bufsz, "%" PRId64 "ns", it);

	} else if (it <= TimeMillisecond) {
		Duration micro = (Duration)(it/TimeMicrosecond);
		Duration nano = (Duration)(it % (Duration)TimeMicrosecond);
		if (!nano) {
			snprintf(buf, bufsz, "%" PRId64 "µs", micro);
		} else
			snprintf(buf, bufsz, "%" PRId64 "µs%" PRId64 "ns", micro, nano);

	} else if (it <= TimeSecond/10) {
		Duration milli = (Duration)(it/TimeMillisecond);
		Duration micro = (Duration)((it % (Duration)TimeMillisecond)/TimeMicrosecond);
		if (!micro) {
			snprintf(buf, bufsz, "%" PRId64 "ms", milli);
		} else
			snprintf(buf, bufsz, "%" PRId64 "ms%" PRId64 "µs", milli, micro);

	} else if (it < TimeMinute) {
		double s = ((double)it/(double)TimeSecond);

		snprintf(buf, bufsz, "%.2lfs", s);

	} else {
		Duration h = (Duration)(it/TimeHour);
		Duration m = (Duration)((it % (Duration)TimeHour)/TimeMinute);
		double s = ((double)(it % (Duration)TimeMinute)/(double)TimeSecond);

		if (!h) {
			snprintf(buf, bufsz, "%" PRId64 "m%.2lfs", m, s);
		} else if (!m) {
			snprintf(buf, bufsz, "%" PRId64 "h%.2lfs", h, s);
		} else if (!s) {
			snprintf(buf, bufsz, "%" PRId64 "h%" PRId64 "m", h, m);
		} else if (!h && !s) {
			snprintf(buf, bufsz, "%" PRId64 "m", m);
		} else if (!m && !s) {
			snprintf(buf, bufsz, "%" PRId64 "h", h);
		} else
			snprintf(buf, bufsz, "%" PRId64 "h%" PRId64 "m%.2lfs", h, m, s);
	}

cleanup:
	return ret;
}

int time_duration_parse(char *raw, Duration *out) {
	int ret = 0;
	char *cursor = NULL;

	/* provided raw string is null pointer */
	if (!raw) {
		*out = 0; ret = 1; goto cleanup;
	}

	switch (raw[0]) {
		case '\0':  /* provided raw string is empty */
			*out = 0; ret = 2; goto cleanup;
		case '0':  /* zero */
			*out = 0; goto cleanup;
		default:
			break;
	}

	for (cursor = raw; *cursor; cursor++)
		printf("~~~~~> %c\n", cursor[0]);

cleanup:
	return ret;
}
