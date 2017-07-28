#include <stdio.h>
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include <io/udp.h>
#include <common/opt.h>
#include <common/codec-kind.h>

#include "cfg.h"
#include "signal.h"
#include "wrkr.h"
#include "map.h"

// tsplay ../tmp/HD-big-buck-bunny.ts 239.255.1.1:5500 -loop
// tsplay ../tmp/HD-tears-of-steel.ts 239.255.1.2:5500 -loop
// ../tmp/HD-tears-of-steel.ts
//
// make && ../bin/app-3 239.255.1.1:5500 239.255.1.2:5500 -i ../tmp/HD-big-buck-bunny.ts -- ../tmp/HD-tears-of-steel.ts

static int opt_parse_cb(void *opaque, OptState state, char *k, char *v) {
	Cfg *it = NULL;
	CfgI *cfg_i = NULL;
	CfgMap *cfg_map = NULL;
	CfgO *cfg_o = NULL;

	it = (Cfg*)opaque;

	cfg_i = slice_tail(&it->i);
	if cfg_i
		cfg_map = slice_tail(&cfg_i->maps);  /* TODO: set default to -map all if no mapped; */
	if cfg_map
		cfg_o = slice_tail(&cfg_map->o)

	if (
		(state & OPT_STATE_POS) || (
			(state & OPT_STATE_KEY) &&
			OPT_MTCH2(k, "i", "input")
		)
	) {
		CfgI cfg_i = { 0 };
		cfg_i_init(&cfg_i);
		url_parse(&cfg_i.url, v);
		slice_append(&it->i, &cfg_i);

	} else if (state & OPT_STATE_KEY) {
		if OPT_MTCH3(k, "c", "cfg", "config") it->c = v;
		else if OPT_MTCH2(k, "m", "map") {
			CfgMap c = { 0 };

			cfg_map_init(&c);
			map_parse(&c.map, v);

			slice_append(&cfg_i->maps, &c);
		} else if OPT_MTCH3(k, "o", "out", "output") {
			CfgO c = { 0 };

			url_parse(&cfg_o.url, v);

			slice_append(&cfg_map->o, &cfg_o);
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int ret = EX_OK;
	Cfg cfg = { 0 };
	UDP udp = { 0 };
	FILE file = { 0 };
	char ebuf[255] = { 0 };
	Slice wrkrs = { 0 };
	Wrkr *wrkr = NULL;

	slice_init(&wrkrs, sizeof(Wrkr));

	signal_init();

	char *opts[] = {
		"c", "cfg", "config",
		"i", "input",
		"o", "output",

		"map",
		"metrics",

		// TODO: handle bool flags
		"v", "vv", "vvv",
		"background", "foreground",
		"h", "help",

		NULL
	};

	InputCfg icfg = {
		.udp = {
			.fifo_cap = 100*MPEGTS_PACKET_COUNT*MPEGTS_PACKET_SIZE,
			.fifo_read_buf_sz = MPEGTS_PACKET_COUNT*MPEGTS_PACKET_SIZE,
		},

		.file = {},
	};

	cfg_init(&cfg);
	opt_parse(argc, argv, opts, (void*)&cfg, opt_parse_cb);

	if (!cfg_validate(&cfg)) {
		ret = EX_CONFIG; goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)cfg.i.len; i++) {  /* TODO: iterrator */
		wrkr = NULL;
		CfgI *cfg_i = slice_get(&cfg.i, (size_t)i);

		{int j = 0; for (j = 0; j < (int)cfg_i->maps.len; j++) {
			CfgMap *cfg_map = slice_get(&cfg_i->maps, (size_t)j);

			char buf[255] = { 0 };
			map_str(&cfg_map->map, buf, sizeof(buf));

			printf("~~~~~> %s, kind: %d, id/pid: %d\n", buf, cfg_map->map.kind, cfg_map->map.id);
		}}

		URL *u = &cfg_i->url;

		wrkr_new(&wrkr);  /* TODO: error handling */
		WrkrCfg wcfg = {
			.url = u,
			.i = &icfg,
		};
		wrkr_init(wrkr, &wcfg);  /* TODO: error handling */

		slice_append(&wrkrs, wrkr);
	}}

	{int i = 0; for (i = 0; i < (int)wrkrs.len; i++) {  /* TODO: iterrator */
		wrkr = slice_get(&wrkrs, (size_t)i);
		wrkr_run(wrkr);
	}}

	signal_wait();

cleanup:
	/* TODO: close workers */
	return ret;
}
