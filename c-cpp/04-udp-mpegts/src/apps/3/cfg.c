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
int cfg_init(CFG *it) {
	int ret = 0;

	it->i = NULL;

	return ret;
}

int cfg_validate(CFG *it) {
	int ok = 1;

	if (!it->i) {
		fprintf(stderr, "no inputs provided; -i --input: must be specified;\n");
		ok = 0;
	}

	return ok;
}

/* finalize */
int cfg_fin(CFG *it) {
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

	cfg_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
