#include <stdio.h>     /* printf */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */
#include <inttypes.h>  /* PRId64 */

#include "time.h"  /* filter */


int main(int argc, char *argv[]) {
	int ret = EX_OK;

	struct timespec ts1 = { 0 },
	                ts2 = { 0 };

	clock_gettime(CLOCK_MONOTONIC, &ts1);

	ts1.tv_nsec = 50;

	ts2.tv_sec = ts1.tv_sec + 5;
	ts2.tv_nsec = 10;

	Duration d = time_sub(&ts1, &ts2);

	printf("ts-1(f): %lld.%.9ld; ts-2(t): %lld.%.9ld;\n",
		(long long)ts1.tv_sec, ts1.tv_nsec,
		(long long)ts2.tv_sec, ts2.tv_nsec);
	printf("duration: %" PRId64 "\n", (int64_t)d);

	clock_gettime(CLOCK_MONOTONIC, &ts1);

	struct timespec slp1 = { 0, 12500000 };
	struct timespec slp2 = { 0 };
	nanosleep(&slp1, &slp2);

	d = time_since(&ts1);

	char buf[100] = { 0 };
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-since: %s\n", buf);

	d = 10*TimeHour + 30*TimeMinute + 15*TimeSecond + 100*TimeMillisecond;
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-1: %s\n", buf);

	d = 23*TimeMillisecond + 17*TimeMicrosecond + 40*TimeNanosecond;
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-2: %s\n", buf);

	d = 18*TimeMicrosecond + 41*TimeNanosecond;
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-3: %s\n", buf);

	d = 87*TimeNanosecond;
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-4: %s\n", buf);

	d = 52*TimeSecond + 127*TimeMillisecond + 18*TimeMicrosecond + 41*TimeNanosecond;
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-5: %s\n", buf);


cleanup:
	return ret;
}
