#ifndef __APPS_3_CFG__
#define __APPS_3_CFG__

#include <stdint.h>    /* uint8_t */
#include <inttypes.h>  /* PRIu64 */

#include <url/url.h>
#include <common/opt.h>  /* OptState */
#include <collections/slice.h>

#include "map.h"


typedef struct cfg_s     Cfg;
typedef struct cfg_i_s   CfgI;
typedef struct cfg_map_s CfgMap;
typedef struct cfg_o_s   CfgO;

struct cfg_s {
	int v;          /* show version and exit */
	int h;          /* show help */
	char *c;        /* path to config file */
	int print_cfg;  /* print/log configuration */

	Slice i;  /* inputs */
};

struct cfg_i_s {
	uint64_t id;
	char *name;

	URL u;

	size_t fifo_cap;
	size_t fifo_read_buf_sz;

	Slice maps;  /* mapped pids */
};

struct cfg_map_s {
	Map map;
	Slice o;  /* outputs */
};

struct cfg_o_s {
	URL u;
};


/* memory allocation */
int cfg_new(Cfg **out);
/* initialize => set initial state */
int cfg_init(Cfg *it);
/* validate */
int cfg_validate(Cfg *it);
/* print to stdout */
int cfg_print(Cfg *it);
int cfg_opt_parse_cb(void *opaque, OptState state, char *k, char *v);
/* finalize */
int cfg_fin(Cfg *it);
/* destructor */
int cfg_del(Cfg **out);

int cfg_i_init(CfgI *it);
int cfg_i_fin(CfgI *it);

int cfg_map_init(CfgMap *it);
int cfg_map_fin(CfgMap *it);


#endif /* __APPS_3_CFG__ */
