#include <stdio.h>
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include "cfg.h"     /* Cfg*, cfg_opt_parse_cb */
#include "signal.h"  /* signal_init, signal_wait */
#include "wrkr.h"
#include "map.h"


Opt opts[] = {
	{(char *[]){"c", "cfg", "config", NULL}, OPT_OPTION_KIND_ARG},

	{(char *[]){"i", "input", NULL},         OPT_OPTION_KIND_ARG},
		{(char *[]){"id", NULL},               OPT_OPTION_KIND_ARG},
		{(char *[]){"name", NULL},             OPT_OPTION_KIND_ARG},
		{(char *[]){"fifo-cap", NULL},         OPT_OPTION_KIND_ARG},
		{(char *[]){"fifo-read-buf-sz", NULL}, OPT_OPTION_KIND_ARG},
		{(char *[]){"name", NULL},             OPT_OPTION_KIND_ARG},
		{(char *[]){"m", "map", NULL},         OPT_OPTION_KIND_ARG},
			{(char *[]){"o", "output", NULL},    OPT_OPTION_KIND_ARG},

	{(char *[]){"v", "version", NULL},        OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"vv", "verbose", NULL},       OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"vvv", "very-verbose", NULL}, OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"h", "help", NULL},           OPT_OPTION_KIND_NO_ARG},

	{(char *[]){"print-config", "print-cfg", NULL}, OPT_OPTION_KIND_NO_ARG},

	{(char *[]){"background", NULL}, OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"foreground", NULL}, OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"color",      NULL}, OPT_OPTION_KIND_NO_ARG},
	{(char *[]){"no-color",   NULL}, OPT_OPTION_KIND_NO_ARG},

	{NULL, 0},
};


int main(int argc, char *argv[]) {
	int ret = EX_OK;
	Cfg cfg = { 0 };
	Slice wrkrs = { 0 };
	Wrkr *wrkr = NULL;

	slice_init(&wrkrs, sizeof(Wrkr));

	signal_init();


	cfg_init(&cfg);
	opt_parse(argc, argv, opts, (void*)&cfg, cfg_opt_parse_cb);

	if (!cfg_validate(&cfg)) {
		if (cfg.h) cfg_help();
		if (cfg.print_cfg) cfg_print(&cfg);

		ret = EX_CONFIG; goto cleanup;
	}

	if ((cfg.print_cfg) || (cfg.h)) {
		if (cfg.h) cfg_help();
		if (cfg.print_cfg) cfg_print(&cfg);

		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)cfg.i.len; i++) {  /* TODO: iterrator */
		wrkr = NULL;
		CfgI *cfg_i = slice_get(&cfg.i, (size_t)i);

		InputCfg icfg = {
			.udp = {
				.fifo_cap = cfg_i->fifo_cap,
				.fifo_read_buf_sz = cfg_i->fifo_read_buf_sz,
			},

			.file = {},
		};
		WrkrCfg wcfg = {
			.url = cfg_i->u,
			.i = icfg,
		};
		wrkr_cfg_init(&wcfg);
		{int j = 0; for (j = 0; j < (int)cfg_i->maps.len; j++) {
			CfgMap *cfg_map = slice_get(&cfg_i->maps, (size_t)j);

			PipelineMapCfg pcfg_map = {
				.m = cfg_map->map,
			};
			pipeline_map_cfg_init(&pcfg_map);

			{int k = 0; for (k = 0; k < (int)cfg_map->o.len; k++) {
				CfgO *cfg_o = slice_get(&cfg_map->o, (size_t)k);

				PipelineOutputCfg pcfg_o = {
					.u = cfg_o->u,
				};

				slice_append(&pcfg_map.o, &pcfg_o);
			}}

			slice_append(&wcfg.m, &pcfg_map);
		}}

		wrkr_new(&wrkr);  /* TODO: error handling */
		wrkr_init(wrkr, wcfg);  /* TODO: error handling */

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
