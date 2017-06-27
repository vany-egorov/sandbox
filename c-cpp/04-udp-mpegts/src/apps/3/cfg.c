#include "./cfg.h"


/* memory allocation */
int cfg_new(CFG **out) {
	int ret = 0;
	CFG *it = NULL;

	it = calloc(1, sizeof(CFG));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

/* initialize => set initial state */
int cfg_initialize(CFG *it) {
	int ret = 0;

	it->i = NULL;

	return ret;
}

/* finalize */
int cfg_finalize(CFG *it) {
	int ret = 0;

	if (!it) return ret;

	if (it->i) {
		slice_del(it->i);
		it->i = NULL;
	}

	return ret;
}

/* destructor */
int cfg_del(CFG **out) {
	int ret = 0;
	CFG *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	cfg_finalize(it);

	free(it);
	*out = NULL;

	return ret;
}
