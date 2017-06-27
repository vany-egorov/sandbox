#include <stdio.h>
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include <io/udp.h>
#include <common/opt.h>

#include "./cfg.h"

// tsplay ../tmp/HD-NatGeoWild.ts 239.255.1.1:5500 -loop
// tsplay ../tmp/HD-1.ts 239.255.1.2:5500 -loop
// ../tmp/HD-1.ts
// make && ../bin/app-3 239.255.1.1:5500 239.255.1.2:5500 -i ../tmp/HD-NatGeoWild.ts -- ../tmp/HD-1.ts

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
	int ret = EX_OK;
	CFG cfg = { 0 };
	UDP udp = { 0 };
	FILE file = { 0 };
	char ebuf[255] = { 0 };

	char *opts[] = {
		"c", "cfg", "config",
		"i", "input",
		"o", "output",
		"v", "vv", "vvv", "h",
		"min-level", "max-level",
		NULL
	};

	cfg_initialize(&cfg);
	opt_parse(argc, argv, opts, (void*)&cfg, opt_parse_cb);

	{int i = 0; for (i = 0; i < (int)cfg.i->len; i++) {
		CFGI *cfg_i = slice_get(cfg.i, (size_t)i);
		URL *u = &cfg_i->url;

		switch (u->scheme) {
		case URL_SCHEME_UDP:
			if (udp_open_i(&udp, url_host(u), u->port,
			               NULL, ebuf, sizeof(ebuf))) {
				printf("FAIL udp open %s\n", ebuf);
			} else
				printf("OK udp open\n");

			printf("UDP %s %d\n", url_host(u), u->port);

			break;
		case URL_SCHEME_FILE:
			if (file_open(&file, url_path(u), "rb",
			              ebuf, sizeof(ebuf))) {
				printf("FAIL file open %s\n", ebuf);
			} else
				printf("OK file open\n");
			break;
		case URL_SCHEME_HTTP:
			printf("HTTP\n");
			break;
		default:
			break;
		}

		// char urls[255] = { 0 };
		// char urljson[255] = { 0 };
		// url_sprint(&cfg_i->url, urls, sizeof(urls));
		// url_sprint_json(&cfg_i->url, urljson, sizeof(urljson));

		// printf("%s\n", urls);
		// printf("%s\n", urljson);
	}}

cleanup:
	return ret;
}
