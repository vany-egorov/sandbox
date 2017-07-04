#ifndef __APPS_3_INPUT__
#define __APPS_3_INPUT__


#include <url/url.h>  /* URL */


typedef struct input_s Input;
typedef struct input_vt_s InputVt;


extern InputVt input_file_vt;  /* file virtual table */
extern InputVt input_udp_vt;   /* UDP virtual table */


struct input_s {
	URL url;
	void *w; /* wrapped, child, opaque */

	InputVt *vt; /* virtual table */
};

struct input_vt_s {
	int (*open) (void *it, URL *url);
	int (*read) (void *it, uint8_t *buf, size_t bufsz, size_t *n);
	int (*close) (void *it);
};


int input_new(Input **out);
int input_open(Input *it, URL *url);
int input_read(Input *it);
int input_close(Input *out);


#endif /* __APPS_3_INPUT__ */
