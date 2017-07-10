#include <time.h>
#include <stdio.h>

#include "url.h"


#define COLOR_BRIGHT_BLACK    "\x1B[1;30m"
#define COLOR_BRIGHT_RED      "\x1B[1;31m"
#define COLOR_BRIGHT_GREEN    "\x1B[1;32m"
#define COLOR_BRIGHT_YELLOW   "\x1B[1;33m"
#define COLOR_BRIGHT_BLUE     "\x1B[1;34m"
#define COLOR_BRIGHT_MAGENTA  "\x1B[1;35m"
#define COLOR_BRIGHT_CYAN     "\x1B[1;36m"
#define COLOR_BRIGHT_WHITE    "\x1B[1;37m"

#define COLOR_DIM_BLACK   "\x1B[2;30m"
#define COLOR_DIM_RED     "\x1B[2;31m"
#define COLOR_DIM_GREEN   "\x1B[2;32m"
#define COLOR_DIM_YELLOW  "\x1B[2;33m"
#define COLOR_DIM_BLUE    "\x1B[2;34m"
#define COLOR_DIM_MAGENTA "\x1B[2;35m"
#define COLOR_DIM_CYAN    "\x1B[2;36m"
#define COLOR_DIM_WHITE   "\x1B[2;37m"

#define COLOR_RESET "\033[0m"


struct fixture_s {
	char *in;

	char      *s;
	struct {
		URLScheme scheme;
		char      buf[255];
		char      userinfo[100];
		char      host[100];
		uint16_t  port;
		char      path[255];
		char      ext[255];
		char      query[255];
		char      fragment[100];
		int       flags;
	} u;
};
typedef struct fixture_s Fixture;


static Fixture fixtures[] = {
	{
		"../tmp/1.ts",
		"file://../tmp/1.ts",
		{
			.scheme=URL_SCHEME_FILE,
			.path="../tmp/1.ts",
			.ext="ts",
		},
	},
	{
		"./tmp/1.ts",
		"file://./tmp/1.ts",
		{
			.scheme=URL_SCHEME_FILE,
			.path="./tmp/1.ts",
			.ext="ts",
		},
	},
	{
		"udp://239.255.1.1:8800",
		"udp://239.255.1.1:8800",
		{
			.scheme=URL_SCHEME_UDP,
			.host="239.255.1.1",
			.port=8800,
		},
	},
	{
		"UdP://128.201.1.1",
		"udp://128.201.1.1",
		{
			.scheme=URL_SCHEME_UDP,
			.host="128.201.1.1",
			.port=0,
		},
	},
	{
		"UdP://128.201.1.1/foo/bar/buz",
		"udp://128.201.1.1/foo/bar/buz",
		{
			.scheme=URL_SCHEME_UDP,
			.host="128.201.1.1",
			.path="/foo/bar/buz",
			.ext="",
		},
	},
	{
		"239.255.1.1:8800",
		"udp://239.255.1.1:8800",
		{
			.scheme=URL_SCHEME_UDP,
			.host="239.255.1.1",
			.port=8800,
		},
	},
	{
		"file:///mnt/data/ts.ts",
		"file:///mnt/data/ts.ts",
		{
			.scheme=URL_SCHEME_FILE,
			.host="",
			.port=0,
			.path="/mnt/data/ts.ts",
			.ext="ts",
		},
	},
	{
		"file://mnt/data/ts.ts",
		"file:///mnt/data/ts.ts",
		{
			.scheme=URL_SCHEME_FILE,
			.host="",
			.port=0,
			.path="/mnt/data/ts.ts",
			.ext="ts",
		},
	},
	{
		"/tmp/ts.mp4",
		"file:///tmp/ts.mp4",
		{
			.scheme=URL_SCHEME_FILE,
			.host="",
			.port=0,
			.path="/tmp/ts.mp4",
			.ext="mp4",
		},
	},
	{
		"ssh://git@github.com/vany-egorov/absolut.git",
		"ssh://git@github.com:22/vany-egorov/absolut.git",
		{
			.scheme=URL_SCHEME_SSH,
			.userinfo="git",
			.host="github.com",
			.port=22,
			.path="/vany-egorov/absolut.git",
			.ext="git",
		},
	},
	{
		"http://127.0.0.1/google/com?foo=bar&bar=buz",
		"http://127.0.0.1:80/google/com?foo=bar&bar=buz",
		{
			.scheme=URL_SCHEME_HTTP,
			.host="127.0.0.1",
			.port=80,
			.path="/google/com",
			.query="foo=bar&bar=buz",
		},
	},
	{
		"http://127.0.0.1/google/com?foo=bar&bar=buz#section1",
		"http://127.0.0.1:80/google/com?foo=bar&bar=buz#section1",
		{
			.scheme=URL_SCHEME_HTTP,
			.host="127.0.0.1",
			.port=80,
			.path="/google/com",
			.query="foo=bar&bar=buz",
			.fragment="section1",
		},
	},
	{
		"http://127.0.0.1/google/com#section1",
		"http://127.0.0.1:80/google/com#section1",
		{
			.scheme=URL_SCHEME_HTTP,
			.host="127.0.0.1",
			.port=80,
			.path="/google/com",
			.fragment="section1",
		},
	},
	{
		"http://127.0.0.1/google/com#section1",
		"http://127.0.0.1:80/google/com#section1",
		{
			.scheme=URL_SCHEME_HTTP,
			.host="127.0.0.1",
			.port=80,
			.path="/google/com",
			.fragment="section1",
		},
	},
	{
		"/221.1.1.1:5500",
		"file:///221.1.1.1:5500",
		{
			.scheme=URL_SCHEME_FILE,
			.path="/221.1.1.1:5500",
			.ext="1:5500"
		},
	},

	{
		NULL,
		NULL,
		{ 0 },
	},
};


static int ret = 0,
           pass = 0,
           fail = 0;
static void psep() { printf("[ %s----%s ] ", COLOR_BRIGHT_WHITE, COLOR_RESET); }
static void ppass() { printf("[ %sPASS%s ] ", COLOR_BRIGHT_GREEN, COLOR_RESET); pass++; }
static void pfail() { printf("[ %sFAIL%s ] ", COLOR_BRIGHT_RED, COLOR_RESET); fail++; ret = 1; }


int main(int argc, char **argv) {
	URL u = { 0 };
	char s[255] = { 0 };
	Fixture *fixture;
	clock_t start = clock();

	for (fixture=fixtures; fixture->in; fixture++) {
		psep();
		printf("\"%s\"\n", fixture->in);

		memset(s, 0, sizeof(s));

		url_parse(&u, fixture->in);
		url_sprint(&u, s, sizeof(s));

		(!strcmp(fixture->s, s)) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->in, s);

		(fixture->u.scheme == u.scheme) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n",
			url_scheme_string(fixture->u.scheme), url_scheme_string(u.scheme));

		(!strcmp(fixture->u.userinfo, url_user_info(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.userinfo, url_user_info(&u));

		(!strcmp(fixture->u.userinfo, url_user_info(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.userinfo, url_user_info(&u));

		(!strcmp(fixture->u.host, url_host(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.host, url_host(&u));

		(fixture->u.port == u.port) ? ppass() : pfail();
		printf("%d | %d\n", fixture->u.port, u.port);

		(!strcmp(fixture->u.path, url_path(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.path, url_path(&u));

		(!strcmp(fixture->u.ext, url_ext(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.ext, url_ext(&u));

		(!strcmp(fixture->u.query, url_query(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.query, url_query(&u));

		(!strcmp(fixture->u.fragment, url_fragment(&u))) ? ppass() : pfail();
		printf("\"%s\" | \"%s\"\n", fixture->u.fragment, url_fragment(&u));

		printf("\n");
	}

	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;

	printf("%s-------------------------------%s\n", COLOR_BRIGHT_WHITE, COLOR_RESET);
	printf("Ran %d tests in %f seconds\n", pass+fail, seconds);
	if (ret) {
		printf("%sFAIL!%s -- %s%d Passed%s | %s%d Failed%s\n",
			COLOR_BRIGHT_RED, COLOR_RESET,
			COLOR_BRIGHT_GREEN, pass, COLOR_RESET,
			COLOR_BRIGHT_RED, fail, COLOR_RESET
		);
	} else {
		printf("%sPASS!%s -- %s%d Passed%s | %s%d Failed%s\n",
			COLOR_BRIGHT_GREEN, COLOR_RESET,
			COLOR_BRIGHT_GREEN, pass, COLOR_RESET,
			COLOR_BRIGHT_WHITE, fail, COLOR_RESET
		);
	}

	return ret;
}
