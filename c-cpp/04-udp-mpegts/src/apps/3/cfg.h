#ifndef __APPS_3_CFG__
#define __APPS_3_CFG__

#include <stdint.h> /* uint8_t */

#include <url/url.h>
#include <collections/slice.h>


#define CFG_END_OPTIONS         "--"
#define CFG_KEY_VALUE_SEPARATOR '='

/* positional arguments */
#define CFG_STATE_POS 0x01
/* keyword/option arguments (--i ..., -i ..., i: ..., i=...) */
#define CFG_STATE_KEY 0x02
/* got "--" */
#define CFG_STATE_END 0x04
#define CFG_STATE_SET_POS(X) \
	X &= ~CFG_STATE_KEY; \
	X |= CFG_STATE_POS;
#define CFG_STATE_SET_KEY(X) \
	X &= ~CFG_STATE_POS; \
	X |= CFG_STATE_KEY;
#define CFG_STATE_SET_END(X) X |= CFG_STATE_END;


typedef struct cfg_s   CFG;
typedef struct cfg_i_s CFGI;
typedef struct cfg_o_s CFGO;
typedef enum cfg_option_kind_enum CFGOptionKind;


/* option kinds */
enum cfg_option_kind_enum {
	CFG_OPTION_KIND_UNKNOWN   = 0x00,
	CFG_OPTION_KIND_KEY       = 0x01,  /* --key */
	CFG_OPTION_KIND_KEY_VALUE = 0x02,  /* --key=value */
};

struct cfg_s {
	int v;  /* show version and exit */
	int h;  /* show help */
	char *c;  /* path to config file */
	int print_cfg;  /* print/log configuration */

	Slice *i;  /* inputs */
};

struct cfg_i_s {
	uint64_t id;
	char *name;

	URL url;

	Slice *o;  /* outputs */
};

struct cfg_o_s {
	URL *url;
};


/* memory allocation */
int cfg_new(CFG **out);
/* set initial state */
int cfg_init(CFG *it);
/* command-line SAX parser */
int cfg_parse(CFG *it, int argc, char **argv);


#endif /* __APPS_3_CFG__ */
