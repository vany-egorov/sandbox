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

/* --key value
 match left and right
 l => left
 rraw => right raw (not formatted) */
static int match_option_key(char *l, char *rraw) {
	int ret = 0;
	int n = 0;  /* bytes written to buffer */
	char r[255] = { 0 };  /* formatted buffer / right */
	size_t rsz = sizeof(r);

	const char *patterns[] = {
		"-%s", "--%s",
		"-%s:", "--%s:",
		"-%s=", "--%s=",
		NULL
	};

	{const char **p = NULL; for (p = patterns; *p != NULL; p++) {
		n = snprintf(r, rsz, *p, rraw);
		if ((n > 0) && (n < rsz)) {
			if (!strcmp(l, r)) { ret = 1; goto cleanup; }
		}
	}}

cleanup:
	return ret;
}


/* --key=value
 match left and right
 l => left
 rraw => right raw (not formatted) */
static int match_option_key_value(char *l, char *rraw) {
	int ret = 0;
	int n = 0;  /* bytes written to buffer */
	char r[255] = { 0 };  /* formatted buffer / right */
	size_t rsz = sizeof(r);

	const char *patterns[] = {
		"-%s=", "--%s=",
		NULL
	};

	{const char **p = NULL; for (p = patterns; *p != NULL; p++) {
		n = snprintf(r, rsz, *p, rraw);
		if ((n > 0) && (n < rsz)) {
			if (
				(strlen(l) > strlen(r)) &&
				(!strncmp(l, r, strlen(r)))
			) {
				ret = 1;
				goto cleanup;
			}
		}
	}}

cleanup:
	return ret;
}

/* match both:
 --key value
 --key=value
*/
static int match_option(char *l, char *r) {
	if (match_option_key(l, r)) return 1;
	if (match_option_key_value(l, r)) return 1;

	return 0;
}


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
			if (match_option(k, "i")) {
				get_v(argc, argv, i, "i", k, &v);
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
