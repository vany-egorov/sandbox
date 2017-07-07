#ifndef __APPS_3_INPUT__
#define __APPS_3_INPUT__


#include <url/url.h>  /* URL */

#include "input-vt.h"    /* InputVT */
#include "input-udp.h"   /* InputUDPCfg */
#include "input-file.h"  /* InputFileCfg */


typedef struct input_s Input;
typedef struct input_cfg_s InputCfg;


struct input_cfg_s {
	InputUDPCfg udp;
	InputFileCfg file;
};

struct input_s {
	URL u;
	void *w; /* wrapped, child, opaque */

	InputVT *vt; /* virtual table */
};


int input_new(Input **out);
int input_open(Input *it, URL *url);
int input_read(Input *it, void *ctx, input_read_cb_fn cb);
int input_close(Input *out);


#endif  /* __APPS_3_INPUT__ */
