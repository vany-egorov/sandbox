#include <stdio.h>

#include <common/opt.h>

#include "./cfg.h"

// make && ../bin/app-3 udp://239.255.1.1:5500 -c /etc/va/config/yml -i udp://239.255.1.1:5500 -i udp://239.255.1.2:5500 -i=239.255.1.10:5500 -i /tmp/dump-i.ts -vv -vvv -max_level --min_level -mAX_leveL -- 239.255.1.3:5500 /tmp/dump-3/ts

static int opt_parse_cb(void *opaque, OptState state, char *k, char *v) {
	CFG *it = NULL;

	it = (CFG*)opaque;

	if (
		(state & OPT_STATE_POS) || (
			(state & OPT_STATE_KEY) &&
			OPT_MTCH2(k, "i", "input")
		)
	) {
		CFGI cfg_i = { 0 };
		url_parse(&cfg_i.url, v);

		if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
		slice_append(it->i, &cfg_i);

	} else if (state & OPT_STATE_KEY) {
		if OPT_MTCH3(k, "c", "cfg", "config") it->c = v;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	CFG cfg = { 0 };

	char *opts[] = {
		"c", "cfg", "config",
		"i", "input",
		"o", "output",
		"v", "vv", "vvv", "h",
		"min-level", "max-level",
		NULL
	};

	cfg_init(&cfg);
	opt_parse(argc, argv, opts, (void*)&cfg, opt_parse_cb);

	printf("config => %s\n", cfg.c);
	{int i = 0; for (i = 0; i < (int)cfg.i->len; i++) {
		CFGI *cfg_i = slice_get(cfg.i, (size_t)i);

		char urls[255] = { 0 };
		char urljson[255] = { 0 };
		url_sprint(&cfg_i->url, urls, sizeof(urls));
		url_sprint_json(&cfg_i->url, urljson, sizeof(urljson));

		printf("%s %s\n", urls, urljson);
	}}
}
