#include "./cfg.h"


/* memory allocation */
int cfg_new(CFG **out) {
	int ret = 0;
	CFG *it = NULL;

	it = calloc(1, sizeof(CFG));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

/* set initial state */
int cfg_init(CFG *it) {
	int ret = 0;

	it->i = NULL;

	return ret;
}

static int get_v(int argc, char **argv, int i, char *k, const char *option, char **v) {
	int ret = 0;

	*v = argv[i+1];

cleanup:
	return ret;
}


#define BUFFER_PUSH(X, Y)                             \
	n = snprintf(rc, rbufsz, Y, r);                     \
	if ((n > 0) && (n < rbufsz)) {                      \
		X = rc;                                           \
		rc += n + 1;  /* n + \0 chracter */               \
		rbufsz -= (size_t)(n + 1);  /* n + \0 chracter */ \
	}

/* match left and right
 l => left
 r => right */
static int match_option(char *l, char *r) {
	int ret = 0;
	int n = 0;  /* bytes written to buffer */
	char rbuf[1024] = { 0 },
	     *rc = NULL,  /* r => right-cursor */
	     *r1 = NULL,
	     *r2 = NULL,
	     *r3 = NULL,
	     *r4 = NULL,
	     *r5 = NULL,
	     *r6 = NULL,
	     *r7 = NULL;
	size_t rbufsz = 0;

	rc = rbuf;
	rbufsz = sizeof(rbuf);

	BUFFER_PUSH(r1, "-%s")
	BUFFER_PUSH(r2, "--%s")
	BUFFER_PUSH(r3, "-%s:")
	BUFFER_PUSH(r4, "--%s:")
	BUFFER_PUSH(r5, "--%s:")
	BUFFER_PUSH(r6, "-%s=")
	BUFFER_PUSH(r7, "--%s=")

	printf("match-option: %s, %s, %s, %s, %s, %s, %s\n", r1, r2, r3, r4, r5, r6 ,r7);

	return ret;
}
#undef BUFFER_PUSH

/* command-line SAX parser */
int cfg_parse(CFG *it, int argc, char **argv) {
	char *k = NULL,
	     *v = NULL;
	unsigned char state = 0;

	state |= CFG_STATE_POS;

	{int i = 0; for (i = 1; i < argc; i++) {
		k = argv[i];

		if (!strcmp(k, CFG_END_OPTIONS)) {
			CFG_STATE_SET_POS(state);
			CFG_STATE_SET_END(state);
			continue;
		}

		if (!(state & CFG_STATE_END) && CFG_IS_OPTION(k))
			CFG_STATE_SET_KEY(state);

		if (state & CFG_STATE_POS) {
			v = k;

			CFGI cfg_i = { 0 };
			url_parse(&cfg_i.url, v);

			if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
			slice_append(it->i, &cfg_i);

		} else if (state & CFG_STATE_KEY) {
			match_option(k, "i");

			if CFG_MATCH_OPTION(k, "i") {
				get_v(argc, argv, i, "i", k, &v);

				printf("%s => %s\n", k, v);
			}
		}
	}}

	{int i = 0; for (i = 0; i < (int)it->i->len; i++) {
		CFGI *cfg_i = slice_get(it->i, (size_t)i);

		char urls[255] = { 0 };
		char urljson[255] = { 0 };
		url_sprint(&cfg_i->url, urls, sizeof(urls));
		url_sprint_json(&cfg_i->url, urljson, sizeof(urljson));

		printf("%s %s\n", urls, urljson);
	}}
}
