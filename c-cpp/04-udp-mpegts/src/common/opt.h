#ifndef __VA_COMMON_OPT__
#define __VA_COMMON_OPT__


#include <stdint.h> /* uint8_t */


typedef enum opt_option_kind_enum OptOptionKind;
typedef enum opt_state_enum OptState;
typedef int (*opt_parse_cb_fn) (void *opaque, OptState state, char *k, char *v);

/* config parser state */
enum opt_state_enum {
	OPT_STATE_POS = 0x01,  /* positional arguments */
	OPT_STATE_KEY = 0x02,  /* keyword/option arguments (--i ..., -i ..., i: ..., i=...) */
	OPT_STATE_END = 0x04,  /* got "--" */
};

#define OPT_STATE_SET_POS(X) \
	X &= ~OPT_STATE_KEY; \
	X |= OPT_STATE_POS;
#define OPT_STATE_SET_KEY(X) \
	X &= ~OPT_STATE_POS; \
	X |= OPT_STATE_KEY;
#define OPT_STATE_SET_END(X) X |= OPT_STATE_END;

/* option kinds */
enum opt_option_kind_enum {
	OPT_OPTION_KIND_UNKNOWN   = 0x00,
	OPT_OPTION_KIND_KEY       = 0x01,  /* --key */
	OPT_OPTION_KIND_KEY_VALUE = 0x02,  /* --key=value */
};


#endif // __VA_COMMON_OPT__
