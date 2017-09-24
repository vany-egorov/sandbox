#include "time.h"


Duration time_sub(struct timespec *f, struct timespec *t) {
	return ((Duration)((t->tv_sec - f->tv_sec) * TimeSecond) + (Duration)(t->tv_nsec - f->tv_nsec));
}

Duration time_since(struct timespec *start) {
	struct timespec now = { 0 };

	clock_gettime(CLOCK_MONOTONIC, &now);

	return time_sub(start, &now);
}

int time_duration_str(Duration that, char *buf, size_t bufsz) {
	int ret = 0;
	Duration it = 0;
	char sgn = 0;

	it = that;
	if (it < 0) {
		it *= -1;

		buf[0] = TimeRuneMinus;
		buf++;
		bufsz--;
	}

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
	int ret = 0,
	    match = 0,     /* got match? "ns", "us" (or "µs"), "ms", "s", "m", "h"? */
      negative = 0;  /* "-" sign */
	char *head = NULL,
	     *tail = NULL,
	     *cursor = NULL,
	     *head_global = NULL,
	      rune = 0,
	      rune_prv = 0,
	      rune_nxt = 0;
	double v = 0;
	TimeUnit tu = TIME_UNIT_UNKNOWN;

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

	cursor = raw;

	if (raw[0] == TimeRuneMinus) {
		negative = 1;
		cursor++;
	} else if (raw[0] == TimeRunePlus) {
		negative = 0;
		cursor++;
	}

	head = cursor;
	head_global = cursor;
	for (; cursor[0]; cursor++) {
		rune = cursor[0];
		rune_nxt = cursor[1];

		if (rune == TimeRuneHour) {
			tu = TIME_UNIT_HOUR;

		} else if ((rune == TimeRuneMinute) && (rune_nxt != TimeRuneSecond)) {
			tu = TIME_UNIT_MINUTE;

		} else if (rune == TimeRuneSecond) {
			if (cursor != head) { /* cursor moved so we got rune_prv */
				rune_prv = (cursor-1)[0];

				if (rune_prv == TimeRuneMilli) {
					tu = TIME_UNIT_MILLI;
				} else if (rune_prv == TimeRuneMicro2) {
					tu = TIME_UNIT_MICRO;
				} else if (rune_prv == TimeRuneNano) {
					tu = TIME_UNIT_NANO;
				} else {
					tu = TIME_UNIT_SECOND;
				}
			}
		}

		if (tu != TIME_UNIT_UNKNOWN) {
			match = 1;
			tail = cursor - 1;  /* digits end, addr of last digit */
			v = strtod(head, &tail);

			if (tu == TIME_UNIT_HOUR) {
				*out += (Duration)(v*TimeHour);
			} else if (tu == TIME_UNIT_MINUTE) {
				*out += (Duration)(v*TimeMinute);
			} else if (tu == TIME_UNIT_SECOND) {
				*out += (Duration)(v*TimeSecond);
			} else if (tu == TIME_UNIT_MILLI) {
				*out += (Duration)(v*TimeMillisecond);
			} else if (tu == TIME_UNIT_MICRO) {
				*out += (Duration)(v*TimeMicrosecond);
			} else if (tu == TIME_UNIT_NANO) {
				*out += (Duration)(v*TimeNanosecond);
			}

			/* reset variables */
			tu = TIME_UNIT_UNKNOWN;
			head = cursor + 1;
		}
	}

	/* no runes found. fallback to ns units */
	if (!match) {
		v = strtod(head_global, NULL);
		*out += (Duration)(v*TimeNanosecond);
	}

	if (negative)
		*out *= -1;

cleanup:
	return ret;
}
