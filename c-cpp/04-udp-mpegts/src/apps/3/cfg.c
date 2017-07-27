#include "cfg.h"


/* memory allocation */
int cfg_new(Cfg **out) {
	int ret = 0;
	Cfg *it = NULL;

	it = calloc(1, sizeof(Cfg));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

/* initialize => set initial state */
int cfg_init(Cfg *it) {
	int ret = 0;

	slice_init(&it->i, sizeof(CfgI));

	return ret;
}

int cfg_validate(Cfg *it) {
	int ok = 1;

	if (!it->i.len) {
		fprintf(stderr, "no inputs provided; -i --input: must be specified;\n");
		ok = 0;
	}

	return ok;
}

/* finalize */
int cfg_fin(Cfg *it) {
	int ret = 0;

	if (!it) return ret;

	if (!it->i.len)
		slice_fin(&it->i);

	return ret;
}

/* destructor */
int cfg_del(Cfg **out) {
	int ret = 0;
	Cfg *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	cfg_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


int cfg_i_init(CfgI *it) {
	slice_init(&it->maps, sizeof(CfgMap));
	return 0;
}

int cfg_i_fin(CfgI *it) {
	slice_fin(&it->maps);
	return 0;
}


int cfg_map_init(CfgMap *it) {
	slice_init(&it->o, sizeof(CfgO));
	return 0;
}

int cfg_map_fin(CfgMap *it) {
	slice_fin(&it->o);
	return 0;
}
