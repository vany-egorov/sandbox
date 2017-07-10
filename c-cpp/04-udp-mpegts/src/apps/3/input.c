#include "input.h"


int input_new(Input **out) {
	int ret = 0;
	Input *it = NULL;

	it = calloc(1, sizeof(Input));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int input_open(Input *it, URL *u) {
	if ((!it->vt) || (!it->w)) return 0;

	it->u = *u;
	it->vt->open(it->w, &it->u);

	return 0;
}

int input_read(Input *it, void *ctx, input_read_cb_fn cb) {
	if ((!it->vt) || (!it->w)) return 0;

	it->vt->read(it->w, ctx, cb);

	return 0;
}

int input_close(Input *it) {

}
