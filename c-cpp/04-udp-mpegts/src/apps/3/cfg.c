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

static int cfg_get_v(int argc, char **argv, int i, char *k, const char *option, char **v) {
	int ret = 0;

	if CFG_MATCH_OPTION_KEY_VALUE(k, option) {
		printf("!!!!!!!!!\n");
	}

	*v = argv[i+1];

cleanup:
	return ret;
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
			if CFG_MATCH_OPTION(k, "i") {
				cfg_get_v(argc, argv, i, "i", k, &v);

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
