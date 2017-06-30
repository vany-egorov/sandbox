#ifndef __APPS_3_INPUT__
#define __APPS_3_INPUT__


typedef struct input_s Input;
typedef struct input_vtable_s InputVt;


struct input_s {
	url *URL;
	void *w; /* wrapped, child, opaque */

	const InputVt *vt; /* virtual table */
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
