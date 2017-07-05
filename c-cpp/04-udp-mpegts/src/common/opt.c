#include "./opt.h"


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

	const char **p = patterns;

	{const char **p = NULL; for (p = patterns; *p != NULL; p++) {
		n = snprintf(r, rsz, *p, rraw);
		if ((n > 0) && (n < rsz)) {
			if (!strcasecmp(l, r)) { ret = 1; goto cleanup; }
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
static OptOptionKind match_option(char *l, char *r) {
	if (match_option_key(l, r)) return OPT_OPTION_KIND_KEY;
	if (match_option_key_value(l, r)) return OPT_OPTION_KIND_KEY_VALUE;

	return OPT_OPTION_KIND_UNKNOWN;
}

static inline int is_option(char *v) {
	return ((!strncmp(v, "-", 1)) || (!strncmp(v, "--", 2)));
}

static inline int is_option_end(char *v) { return (!strcmp(v, OPT_END_OPTIONS)); }

static int extract_v_from_key_option(int argc, char **argv, int i, char **v) {
	int ok;
	char *vtmp = NULL;

	if (i+1 >= argc) {
		ok = 0;
		goto cleanup;
	}

	vtmp = argv[i+1];

	if (is_option(vtmp)) {
		ok = 1;
		*v = NULL;
		goto cleanup;
	}
	if (is_option_end(vtmp)) {
		ok = 0;
		goto cleanup;
	}

	*v = vtmp;
	ok = 1;

cleanup:
	return ok;
}

static int extract_v_from_key_value_option(char *k, char **v) {
	int ok = 0;
	char *vtmp = NULL;

	vtmp = strchr(k, OPT_KEY_VALUE_SEPARATOR);
	if (!vtmp) {
		ok = 0;
		goto cleanup;
	}

	*v = vtmp+1;
	ok = 1;

cleanup:
	return ok;
}

/* ret:
 0 => false => no match / no extract
 1 => true  => OK / match
*/
static int extract_v(int argc, char **argv, int i, char *l, char *r, char **v) {
	int ok = 0;
	OptOptionKind oknd = OPT_OPTION_KIND_UNKNOWN;

	oknd = match_option(l, r);

	if (oknd == OPT_OPTION_KIND_UNKNOWN) {
		ok = 0;
		goto cleanup;
	} else if (oknd == OPT_OPTION_KIND_KEY) {
		if (extract_v_from_key_option(argc, argv, i, v)) {
			ok = 1;
			goto cleanup;
		} else {
			ok = 0;
			goto cleanup;
		}
	} else if (oknd == OPT_OPTION_KIND_KEY_VALUE) {
		if (extract_v_from_key_value_option(l, v)) {
			ok = 1;
			goto cleanup;
		} else {
			ok = 0;
			goto cleanup;
		}
	}

cleanup:
	if (!ok) *v = NULL;
	return ok;
}

// --log_level => --log-level
static void canonicalize_replace(char *v) {
	{char *r = NULL; for (r = v; *r; r++) {
		if (*r == OPT_KEY_VALUE_SEPARATOR) return;
		if (*r == '_') *r = '-';
	}}
}

// --LoG-LeVeL => --log-level
static void canonicalize_tolower(char *s) {
	char *start = NULL,
	     *finish = NULL;
	int len = 0;

	if (!is_option(s)) return;

	if (!strncmp(s, "--", 2)) start = s + 2;
	else if (!strncmp(s, "-", 1)) start = s + 1;

	finish = strchr(s, OPT_KEY_VALUE_SEPARATOR);
	if (!finish) finish = &s[strlen(s)];

	len = (int)(finish - start);
	if (len <= 1) return;

	{char *r = NULL; for (r = start; r != finish; r++) {
		*r = tolower(*r);
	}}
}

static void canonicalize(int argc, char **argv) {
	char *k = NULL;

	{int i = 0; for (i = 1; i < argc; i++) {
		k = argv[i];

		if ((!is_option(k)) || is_option_end(k)) continue;

		canonicalize_replace(k);
		canonicalize_tolower(k);
	}}
}

/* command-line SAX parser */
int opt_parse(int argc, char **argv, char **opts, void *opaque, opt_parse_cb_fn cb) {
	char *k = NULL,
	     *v = NULL;
	int match = 0;  /* got match? */
	uint8_t state = 0;

	canonicalize(argc, argv);

	state |= OPT_STATE_POS;

	{int i = 0; for (i = 1; i < argc; i++) {
		k = argv[i];
		match = 0;

		if (is_option_end(k)) {
			OPT_STATE_SET_POS(state);
			OPT_STATE_SET_END(state);
			continue;
		}

		if (!(state & OPT_STATE_END) && is_option(k)) OPT_STATE_SET_KEY(state);

		if (state & OPT_STATE_POS) {
			v = k;

			cb(opaque, state, NULL, v);
			match = 1;

		} else if (state & OPT_STATE_KEY) {
			if (!is_option(k)) continue;

			{char **opt = NULL; for (opt = opts; *opt != NULL; opt++) {
				if (extract_v(argc, argv, i, k, *opt, &v)) {
					cb(opaque, state, *opt, v);
					match = 1;
					break;
				}
			}}
		}

		// TODO: handle no value for option value
		// TODO: move warnings to output structute
		if (!match)
			fprintf(stderr, "unknown option \"%s\"\n", k);
	}}
}

