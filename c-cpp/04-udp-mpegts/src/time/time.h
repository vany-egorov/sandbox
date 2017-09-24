#ifndef __TIME_TIME__
#define __TIME_TIME__


#include <time.h>      /* struct timespec */
#include <stdio.h>     /* snprintf */
#include <stdint.h>    /* int64_t */
#include <stdlib.h>    /* strtof */
#include <inttypes.h>  /* PRId64 */


#define TimeNanosecond  1
#define TimeMicrosecond 1e3
#define TimeMillisecond 1e6
#define TimeSecond      1e9
#define TimeMinute      6e10
#define TimeHour        36e11

#define TimeRuneNano   'n'
#define TimeRuneMicro1 'µ'
#define TimeRuneMicro2 'u'
#define TimeRuneMilli  'm'
#define TimeRuneSecond 's'
#define TimeRuneMinute 'm'
#define TimeRuneHour   'h'
#define TimeRuneMinus  '-'
#define TimeRuneNone   ''
#define TimeRunePlus   '+'


typedef enum time_unit TimeUnit;

enum time_unit {
	TIME_UNIT_UNKNOWN = 0,

	TIME_UNIT_NANO   = 1,
	TIME_UNIT_MICRO  = 2,
	TIME_UNIT_MILLI  = 3,
	TIME_UNIT_SECOND = 4,
	TIME_UNIT_MINUTE = 5,
	TIME_UNIT_HOUR   = 6,
};


typedef int64_t Duration;


/* the duration t-f */
Duration time_sub(struct timespec *f, struct timespec *t);

/* Since returns the time elapsed since start.
 * It is shorthand for time_sub(start, clock_gettime(CLOCK_MONOTONIC, &now);
 */
Duration time_since(struct timespec *start);

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
int time_duration_parse(char *raw, Duration *out);


#endif /* __TIME_TIME__ */
