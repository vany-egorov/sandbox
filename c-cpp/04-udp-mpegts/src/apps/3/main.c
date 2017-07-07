#include <stdio.h>
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include <io/udp.h>
#include <common/opt.h>

#include "cfg.h"
#include "signal.h"
#include "wrkr.h"

// tsplay ../tmp/HD-NatGeoWild.ts 239.255.1.1:5500 -loop
// tsplay ../tmp/HD-1.ts 239.255.1.2:5500 -loop
// ../tmp/HD-1.ts
//
// make && ../bin/app-3 239.255.1.1:5500 239.255.1.2:5500 -i ../tmp/HD-NatGeoWild.ts -- ../tmp/HD-1.ts

static int opt_parse_cb(void *opaque, OptState state, char *k, char *v) {
	Cfg *it = NULL;

	it = (Cfg*)opaque;

	if (
		(state & OPT_STATE_POS) || (
			(state & OPT_STATE_KEY) &&
			OPT_MTCH2(k, "i", "input")
		)
	) {
		CfgI cfg_i = { 0 };
		url_parse(&cfg_i.url, v);

		if (it->i == NULL) slice_new(&it->i, sizeof(CfgI));
		slice_append(it->i, &cfg_i);

	} else if (state & OPT_STATE_KEY) {
		if OPT_MTCH3(k, "c", "cfg", "config") it->c = v;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int ret = EX_OK;
	Cfg cfg = { 0 };
	UDP udp = { 0 };
	FILE file = { 0 };
	char ebuf[255] = { 0 };
	Slice *wrkrs = NULL;
	Wrkr *wrkr = NULL;

	slice_new(&wrkrs, sizeof(Wrkr));

	signal_init();

	char *opts[] = {
		"c", "cfg", "config",
		"i", "input",
		"o", "output",
		"v", "vv", "vvv",
		"min-level", "max-level",

		// TODO: handle bool flags
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

	{int i = 0; for (i = 0; i < (int)cfg.i->len; i++) {  /* TODO: iterrator */
		wrkr = NULL;
		CfgI *cfg_i = slice_get(cfg.i, (size_t)i);
		URL *u = &cfg_i->url;

		wrkr_new(&wrkr);  /* TODO: error handling */
		WrkrCfg wcfg = {
			.url = u,
			.i = &icfg,
		};
		wrkr_init(wrkr, &wcfg);  /* TODO: error handling */

		slice_append(wrkrs, wrkr);
	}}

	{int i = 0; for (i = 0; i < (int)wrkrs->len; i++) {  /* TODO: iterrator */
		wrkr = slice_get(wrkrs, (size_t)i);
		wrkr_run(wrkr);
	}}

	signal_wait();

cleanup:
	/* TODO: close workers */
	return ret;
}
