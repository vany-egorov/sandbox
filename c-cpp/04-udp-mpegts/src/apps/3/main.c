#include <stdio.h>

#include "./cfg.h"

// make && ../bin/app-3 udp://239.255.1.1:5500 -c /etc/va/config/yml -i udp://239.255.1.1:5500 -i udp://239.255.1.2:5500 -i=239.255.1.10:5500 -i /tmp/dump-i.ts -- 239.255.1.3:5500 /tmp/dump-3/ts

static int cfg_parse_cb(void *opaque, CFGState state, char *k, char *v) {
	printf("cfg-parse-cb => %d - %s - %s\n", state, k, v);

	return 0;
}

// CFGI cfg_i = { 0 };
// url_parse(&cfg_i.url, v);

// if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
// slice_append(it->i, &cfg_i);

// if (extract_v(argc, argv, i, k, "i", &v)) {
// 	CFGI cfg_i = { 0 };
// 	url_parse(&cfg_i.url, v);

// 	if (it->i == NULL) slice_new(&it->i, sizeof(CFGI));
// 	slice_append(it->i, &cfg_i);
// }

// {int i = 0; for (i = 0; i < (int)it->i->len; i++) {
// 	CFGI *cfg_i = slice_get(it->i, (size_t)i);

// 	char urls[255] = { 0 };
// 	char urljson[255] = { 0 };
// 	url_sprint(&cfg_i->url, urls, sizeof(urls));
// 	url_sprint_json(&cfg_i->url, urljson, sizeof(urljson));

// 	printf("%s %s\n", urls, urljson);
// }}

int main (int argc, char *argv[]) {
	CFG cfg = { 0 };

	char *opts[] = {
		"c", "config",
		"i", "input",
		"o", "output",
		"v", "vv", "vvv", "h",
		NULL
	};

	cfg_init(&cfg);
	cfg_parse(argc, argv, opts, (void*)&cfg, cfg_parse_cb);
}
