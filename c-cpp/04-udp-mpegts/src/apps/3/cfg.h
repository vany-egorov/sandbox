#ifndef __APPS_3_CFG__
#define __APPS_3_CFG__

#include <stdint.h>    /* uint8_t */
#include <inttypes.h>  /* PRIu64 */

#include <url/url.h>
#include <common/opt.h>  /* OptState */
#include <collections/slice.h>

#include "map.h"


#define CFG_DEFAULT_FIFO_CAP    100*7*188
#define CFG_DEFAULT_FIFO_BUF_SZ 7*188


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

	/* <udp multicast only> */
	size_t fifo_cap;
	size_t fifo_read_buf_sz;
	/* </udp multicast only> */

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
/* output human-readable config to stdout */
int cfg_print(Cfg *it);
/* display help */
void cfg_help(void);
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
