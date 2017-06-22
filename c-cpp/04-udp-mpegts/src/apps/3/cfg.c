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

/* --key value
 match left and right
 l => left
 rraw => right raw (not formatted) */
static int match_option_key(char *l, char *rraw) {
	int ret = 0;
	int n = 0;  /* bytes written to buffer */
	char r[128] = { 0 };  /* formatted buffer / right */
	size_t rsz = sizeof(r);

	const char const *patterns[] = {
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
	char r[128] = { 0 };  /* formatted buffer / right */
	size_t rsz = sizeof(r);

	const char const *patterns[] = {
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
static CFGOptionKind match_option(char *l, char *r) {
	if (match_option_key(l, r)) return CFG_OPTION_KIND_KEY;
	if (match_option_key_value(l, r)) return CFG_OPTION_KIND_KEY_VALUE;

	return CFG_OPTION_KIND_UNKNOWN;
}

static inline int is_option(char *v) {
	return ((!strncmp(v, "-", 1)) || (!strncmp(v, "--", 2)));
}

static int extract_v_from_key_option(int argc, char **argv, int i, char **v) {
	char *vtmp = NULL;

	if (i+1 >= argc) return 0;

	vtmp = argv[i+1];

	if (*v)

	return 1;
}

static int extract_v_from_key_value_option(char *k, char **v) {
	int ret = 0;

	*v = strchr(k, CFG_KEY_VALUE_SEPARATOR) + 1;

cleanup:
	return ret;
}

static int extract_v(int argc, char **argv, int i, char *l, char *r, char **v) {
	int ret = 0;
	CFGOptionKind oknd = CFG_OPTION_KIND_UNKNOWN;

	oknd = match_option(l, r);

	if (oknd == CFG_OPTION_KIND_UNKNOWN) {
		*v = NULL;
		ret = 0;
		goto cleanup;
	} else if (oknd == CFG_OPTION_KIND_KEY) {
		if (extract_v_from_key_option(argc, argv, i, v)) {
			ret = 1;
			goto cleanup;
		} else {
			ret = 0;
			goto cleanup;
		}
	} else if (oknd == CFG_OPTION_KIND_KEY_VALUE) {
		extract_v_from_key_value_option(l, v);
		ret = 1;
		goto cleanup;
	}

cleanup:
	return ret;
}

/* command-line SAX parser */
int cfg_parse(CFG *it, int argc, char **argv) {
	char *k = NULL,
	     *v = NULL;
	uint8_t state = 0;

	state |= CFG_STATE_POS;

	{int i = 0; for (i = 1; i < argc; i++) {
		k = argv[i];

		if (!strcmp(k, CFG_END_OPTIONS)) {
			CFG_STATE_SET_POS(state);
			CFG_STATE_SET_END(state);
			continue;
		}

		if (!(state & CFG_STATE_END) && is_option(k))
			CFG_STATE_SET_KEY(state);

		if (state & CFG_STATE_POS) {
			v = k;

			CFGI cfg_i = { 0 };
			url_parse(&cfg_i.url, v);

			if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
			slice_append(it->i, &cfg_i);

		} else if (state & CFG_STATE_KEY) {
			if (extract_v(argc, argv, i, k, "i", &v)) {
				CFGI cfg_i = { 0 };
				url_parse(&cfg_i.url, v);

				if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
				slice_append(it->i, &cfg_i);
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
