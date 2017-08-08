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

	struct timespec slp1 = {0,125000000};
	struct timespec slp2 = { 0 };
	nanosleep(&slp1, &slp2);

	d = time_since(&ts1);

	char buf[100] = { 0 };
	time_duration_str(d, buf, sizeof(buf));
	printf("duration-since: %s\n", buf);


cleanup:
	return ret;
}
