#ifndef __APPS_3_CFG__
#define __APPS_3_CFG__

#include <stdint.h> /* uint8_t */

#include <url/url.h>
#include <collections/slice.h>


#define CFG_END_OPTIONS         "--"
#define CFG_KEY_VALUE_SEPARATOR '='


typedef struct cfg_s   Cfg;
typedef struct cfg_i_s CfgI;
typedef struct cfg_o_s CfgO;

struct cfg_s {
	int v;  /* show version and exit */
	int h;  /* show help */
	char *c;  /* path to config file */
	int print_cfg;  /* print/log configuration */

	Slice i;  /* inputs */
};

struct cfg_i_s {
	uint64_t id;
	char *name;

	URL url;

	Slice o;  /* outputs */
};

struct cfg_o_s {
	URL *url;
};


/* memory allocation */
int cfg_new(Cfg **out);
/* initialize => set initial state */
int cfg_init(Cfg *it);
/* finalize */
int cfg_validate(Cfg *it);
/* finalize */
int cfg_fin(Cfg *it);
/* set initial state */
int cfg_del(Cfg **out);


#endif /* __APPS_3_CFG__ */
