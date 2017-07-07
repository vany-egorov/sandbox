#ifndef __APPS_3_INPUT_VT__
#define __APPS_3_INPUT_VT__


#include <url/url.h>  /* URL */


typedef struct input_vt_s InputVT;
typedef int (*input_read_cb_fn) (void *ctx, uint8_t *buf, size_t bufsz);


struct input_vt_s {
	int (*open) (void *it, URL *url);
	int (*read) (void *it, void *opaque, input_read_cb_fn cb);
	int (*close) (void *it);
};


#endif /* __APPS_3_INPUT_VT__ */
