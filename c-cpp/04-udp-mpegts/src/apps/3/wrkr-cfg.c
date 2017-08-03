#include "wrkr-cfg.h"


int wrkr_cfg_init(WrkrCfg *it) {
	slice_init(&it->m, sizeof(WrkrMapCfg));
	return 0;
}

int wrkr_cfg_fin(WrkrCfg *it) {
	slice_fin(&it->m);
	return 0;
}

int wrkr_map_cfg_init(WrkrMapCfg *it) {
	slice_fin(&it->o);
	return 0;
}

int wrkr_map_cfg_fin(WrkrMapCfg *it) {
	slice_fin(&it->o);
	return 0;
}
