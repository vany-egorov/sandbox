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
		goto cleanup;

	} else if (it <= TimeMillisecond) {
		Duration micro = (Duration)(it/TimeMicrosecond);
		Duration nano = it - micro*TimeMicrosecond;
		if (!nano) {
			snprintf(buf, bufsz, "%" PRId64 "µs", micro);
			goto cleanup;
		}

		snprintf(buf, bufsz, "%" PRId64 "µs%" PRId64 "ns", micro, nano);
		goto cleanup;

	} else if (it <= TimeSecond) {
		Duration milli = (Duration)(it/TimeMillisecond);
		Duration micro = (Duration)((it - milli*TimeMillisecond)/TimeMicrosecond);
		if (!micro) {
			snprintf(buf, bufsz, "%" PRId64 "ms", milli);
			goto cleanup;
		}

		snprintf(buf, bufsz, "%" PRId64 "ms%" PRId64 "µs / %" PRId64 "", milli, micro, it);
		goto cleanup;
	}

cleanup:
	return ret;
}
