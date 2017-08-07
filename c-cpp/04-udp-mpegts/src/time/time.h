#ifndef __TIME_TIME__
#define __TIME_TIME__


#define TimeSecond       1
#define TimeMicrosecond (1000 * TimeSecond)
#define TimeMillisecond (1000 * TimeMicrosecond)
#define TimeSecond      (1000 * TimeMillisecond)
#define TimeMinute      (60 * TimeSecond)
#define TimeHour        (60 * TimeMinute)


typedef int64_t Duration;


/* the duration t-u */
Duration time_sub(struct timespec *t, struct timespec *u);

/* String returns a string representing the duration in the form "72h3m0.5s".
 * Leading zero units are omitted. As a special case, durations less than one
 * second format use a smaller unit (milli-, micro-, or nanoseconds) to ensure
 * that the leading digit is non-zero. The zero duration formats as 0s.
 */
int time_duration_str(Duration it, char *buf, size_t bufsz);

/* ParseDuration parses a duration string.
 * A duration string is a possibly signed
 * sequence of decimal numbers, each with
 * optional fraction and a unit suffix,
 * such as "300ms", "-1.5h" or "2h45m". Valid
 * time units are "ns", "us" (or "µs"), "ms", "s", "m", "h".
 */
int time_duration_parse(char *buf, size_t bufsz, Duration *out);


#endif /* __TIME_TIME__ */
