#ifndef __APPS_3_INPUT_VT__
#define __APPS_3_INPUT_VT__


#include <url/url.h>  /* URL */


typedef struct input_vt_s InputVT;


struct input_vt_s {
	int (*open) (void *it, URL *url);
	int (*read) (void *it, uint8_t *buf, size_t bufsz, size_t *n);
	int (*close) (void *it);
};


#endif /* __APPS_3_INPUT_VT__ */
