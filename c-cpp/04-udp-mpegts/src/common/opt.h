#ifndef __VA_COMMON_OPT__
#define __VA_COMMON_OPT__



#include <ctype.h>  /* tolower */
#include <stdio.h>  /* snprintf */
#include <string.h>  /* strlen */
#include <stdint.h>  /* uint8_t */


#define OPT_END_OPTIONS         "--"
#define OPT_KEY_VALUE_SEPARATOR '='
#define OPT_MTCH1(X, Y1) (!strcmp(X, Y1))
#define OPT_MTCH2(X, Y1, Y2) ( \
		(!strcmp(X, Y1)) || \
		(!strcmp(X, Y2))    \
	)
#define OPT_MTCH3(X, Y1, Y2, Y3) ( \
		(!strcmp(X, Y1)) || \
		(!strcmp(X, Y2)) || \
		(!strcmp(X, Y3))    \
	)
#define OPT_MTCH4(X, Y1, Y2, Y3, Y4) ( \
		(!strcmp(X, Y1)) || \
		(!strcmp(X, Y2)) || \
		(!strcmp(X, Y3)) || \
		(!strcmp(X, Y4))    \
	)
#define OPT_MTCH5(X, Y1, Y2, Y3, Y4, Y5) ( \
		(!strcmp(X, Y1)) || \
		(!strcmp(X, Y2)) || \
		(!strcmp(X, Y3)) || \
		(!strcmp(X, Y4)) || \
		(!strcmp(X, Y5))    \
	)

typedef struct opt Opt;

/* option kinds */
typedef enum opt_option_kind_enum {
	OPT_OPTION_KIND_UNKNOWN   = 0x00,
	OPT_OPTION_KIND_KEY       = 0x01,  /* --key */
	OPT_OPTION_KIND_KEY_VALUE = 0x02,  /* --key=value */

	OPT_OPTION_KIND_ARG    = 0x04,  /* required-argument */
	OPT_OPTION_KIND_NO_ARG = 0x08,  /* no argument required (bool flag) e.g. --help, --v, --vv, --vvv */
} OptOptionKind;

/* config parser state */
typedef enum {
	OPT_STATE_POS = 0x01,  /* positional arguments */
	OPT_STATE_KEY = 0x02,  /* keyword/option arguments (--i ..., -i ..., i: ..., i=...) */
	OPT_STATE_END = 0x04,  /* got "--" */
} OptState;

typedef int (*opt_parse_cb_fn) (void *opaque, OptState state, char *k, char *v);


#define OPT_STATE_SET_POS(X) \
	X &= ~OPT_STATE_KEY; \
	X |= OPT_STATE_POS;
#define OPT_STATE_SET_KEY(X) \
	X &= ~OPT_STATE_POS; \
	X |= OPT_STATE_KEY;
#define OPT_STATE_SET_END(X) X |= OPT_STATE_END;


struct opt {
	char **names;

	OptOptionKind kind;
};


int opt_parse(int argc, char **argv, Opt *opts, void *opaque, opt_parse_cb_fn cb);


#endif /* __VA_COMMON_OPT__ */
