#include <stdio.h>     /* printf */
#include <string.h>    /* memset */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */
#include <inttypes.h>  /* PRId64 */

#include "time.h"  /* Duration */


typedef struct fixture3_s Fixture3;
typedef struct fixture4_s Fixture4;


struct fixture3_s {
	Duration in;
	char    *out;
};

struct fixture4_s {
	char *in;
	Duration out;
};


static Fixture3 fixtures3[] = {
	{
		10*TimeHour + 30*TimeMinute + 15*TimeSecond + 100*TimeMillisecond,
		"10h30m15.10s",
	},
	{
		-(10*TimeHour + 30*TimeMinute + 15*TimeSecond + 100*TimeMillisecond),
		"-10h30m15.10s",
	},
	{
		23*TimeMillisecond + 17*TimeMicrosecond + 40*TimeNanosecond,
		"23ms17µs",
	},
	{
		18*TimeMicrosecond + 41*TimeNanosecond,
		"18µs41ns",
	},
	{
		87*TimeNanosecond,
		"87ns"
	},
	{
		52*TimeSecond + 127*TimeMillisecond + 18*TimeMicrosecond + 41*TimeNanosecond,
		"52.13s",
	},
	{ 0, NULL },
};

static Fixture4 fixtures4[] = {
	{
		"14h10m15s200ms30000us40ns",
		14*TimeHour + 10*TimeMinute + 15*TimeSecond + 200*TimeMillisecond + 30000*TimeMicrosecond + 40*TimeNanosecond,
	},
	{  /* negative */
		"-14h10m15s200ms30000us40ns",
		-(14*TimeHour + 10*TimeMinute+ 15*TimeSecond + 200*TimeMillisecond + 30000*TimeMicrosecond + 40*TimeNanosecond),
	},
	{  /* single digit, hour only */
		"1h",
		1*TimeHour,
	},
	{  /* digits only without modificator */
		"12",
		12*TimeNanosecond,
	},
	{  /* invalid: modificator only; */
		"h",
		0,
	},
	{  /* invalid: empty; */
		"",
		0,
	},
	{ NULL, 0 },
};


int t1(void) {
	int ret = 0;
	Duration d = 0;
	char buf[100] = { 0 };
	struct timespec ts1 = { 0 },
	                ts2 = { 0 };

	printf("test-1: time-sub\n");

	clock_gettime(CLOCK_MONOTONIC, &ts1);

	ts1.tv_nsec = 50;

	ts2.tv_sec = ts1.tv_sec + 5;
	ts2.tv_nsec = 10;

	d = time_sub(&ts1, &ts2);

	printf("ts-1(f): %lld.%.9ld; ts-2(t): %lld.%.9ld;\n",
		(long long)ts1.tv_sec, ts1.tv_nsec,
		(long long)ts2.tv_sec, ts2.tv_nsec);

	time_duration_str(d, buf, sizeof(buf));
	printf("duration: %s\n", buf);

cleanup:
	printf("\n");
	return ret;
}

int t2(void) {
	int ret = 0;
	Duration d = 0;
	char buf[100] = { 0 };
	struct timespec ts  = { 0 },
	                slp1 = { 0, 12500000 },
	                slp2 = { 0 };

	printf("test-2: time-since\n");

	clock_gettime(CLOCK_MONOTONIC, &ts);

	nanosleep(&slp1, &slp2);

	d = time_since(&ts);

	time_duration_str(d, buf, sizeof(buf));
	printf("duration: %s\n", buf);

cleanup:
	printf("\n");
	return ret;
}

int t3() {
	int ret = 0;
	char buf[100] = { 0 };
	Fixture3 *fixture = NULL;

	printf("test-3: duration-str\n");

	for (fixture=fixtures3; fixture->out; fixture++) {
		memset(buf, 0, sizeof(buf));

		time_duration_str(fixture->in, buf, sizeof(buf));
		printf("duration: %s", buf);

		if (strcmp(fixture->out, buf)) {
			printf(" FAIL\n");
			ret = 1;
		} else {
			printf(" PASS\n");
		}
	}

cleanup:
	printf("\n");
	return ret;
}

int t4() {
	int ret = 0;
	Duration d = 0;
	Fixture4 *fixture = NULL;

	printf("test-4: duration-parse\n");

	for (fixture=fixtures4; fixture->in; fixture++) {
		d = 0;

		time_duration_parse(fixture->in, &d);
		printf("duration: %s", fixture->in);

		if (fixture->out != d) {
			printf(" FAIL\n");
			ret = 1;
		} else {
			printf(" PASS\n");
		}
	}

cleanup:
	printf("\n");
	return ret;
}

int main(int argc, char *argv[]) {
	int ret = EX_OK;
	char buf[100] = { 0 };
	Duration d = 0;

	if (t1()) { ret = 1; }
	if (t2()) { ret = 1; }
	if (t3()) { ret = 1; }
	if (t4()) { ret = 1; }

cleanup:
	if (ret) {
		printf("FAIL!\n");
	} else
		printf("PASS!\n");

	return ret;
}
